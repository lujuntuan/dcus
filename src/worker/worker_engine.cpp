/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2021/04/22
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include "dcus/worker/worker_engine.h"
#include "commonlib/dir.h"
#include "corelib/core.h"
#include "dcus/base/setting.h"
#include "dcus/base/thread.h"
#include "dcus/worker/worker_event.h"

#define m_hpr m_workerHelper

#define USAGE_NAME_LIST       \
    {                         \
        "-n", "--n", "--name" \
    }
#define USAGE_GUID_LIST       \
    {                         \
        "-g", "--g", "--guid" \
    }

DCUS_NAMESPACE_BEGIN

struct WorkerHelper {
    Thread workThread;
    Upgrade upgrade;
    Detail detail;
    Data attribute;
    Data meta;
    std::string name;
    std::string guid;
    std::string version;
    std::string downloadDir;
    Timer* replyControlTimer = nullptr;
    WorkerEngine::DeployFunction deployFunction = nullptr;
    WorkerEngine::DetailFunction detailFunction = nullptr;
    std::atomic<uint32_t> controlMessageId { 0 };
    std::atomic<uint32_t> detailMessageId { 0 };
    std::atomic_bool hasCancelAction { false };
    std::atomic_bool cancelEnable { true };
    std::atomic_bool needDiscovery { true };
    std::atomic<MasterState> masterState { MR_OFFLINE };
    std::atomic<Control> control = { CTL_UNKNOWN };
    Files patchNewFiles;
    static std::string _workerName;
    static std::string _workerGuid;
    static bool _hasRegister;
};

std::string WorkerHelper::_workerName;
std::string WorkerHelper::_workerGuid;
bool WorkerHelper::_hasRegister = false;

void WorkerEngine::registerDomain(const std::string& name, const std::string& guid)
{
    if (worker_engine) {
        LOG_WARNING("must be registered in worker engine creation");
        return;
    }
    WorkerHelper::_workerName = name;
    WorkerHelper::_workerGuid = guid;
    WorkerHelper::_hasRegister = true;
}

void WorkerEngine::destroy()
{
    if (getInstance()) {
        delete getInstance();
    }
}

WorkerEngine::WorkerEngine(int argc, char** argv)
    : Queue(DCUS_QUEUE_ID_WORKER)
    , Application(argc, argv, "dcus_worker")
{
    if (getInstance()) {
        std::terminate();
        return;
    }
    setInstance(this);
    loadUseage({ USAGE_NAME_LIST, USAGE_GUID_LIST });

    if (!m_hpr) {
        m_hpr = new WorkerHelper;
    }
    static std::mutex mutex;
    setMutex(mutex);

    if (worker_config.value("download_dir").valid()) {
        m_hpr->downloadDir = worker_config.value("download_dir").toString();
    } else {
        m_hpr->downloadDir = Common::getTempDir() + "/dcus_worker_tmp";
    }

    if (WorkerHelper::_hasRegister) {
        m_hpr->name = WorkerHelper::_workerName;
        m_hpr->guid = WorkerHelper::_workerGuid;
    } else {
        Value name = "unknown";
        getOptions(name, USAGE_NAME_LIST, "name");
        m_hpr->name = name.toString();
        Value guid = "";
        getOptions(guid, USAGE_GUID_LIST, "guid");
        m_hpr->guid = guid.toString();
    }

    m_hpr->detail.domain().state() = WR_IDLE;
    m_hpr->detail.domain().last() = WR_OFFLINE;
}

WorkerEngine::~WorkerEngine()
{
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
    setInstance(nullptr);
}

const std::string& WorkerEngine::name() const
{
    return m_hpr->name;
}

const std::string& WorkerEngine::guid() const
{
    return m_hpr->guid;
}

const Upgrade& WorkerEngine::upgrade() const
{
    return m_hpr->upgrade;
}

Control WorkerEngine::control() const
{
    return m_hpr->control;
}

bool WorkerEngine::hasCancelAction() const
{
    return m_hpr->hasCancelAction;
}

bool WorkerEngine::hasStopAction() const
{
    return m_hpr->workThread.isReadyFinished();
}

bool WorkerEngine::cancelEnable() const
{
    return m_hpr->cancelEnable;
}

const std::string& WorkerEngine::version() const
{
    return m_hpr->version;
}

const Data& WorkerEngine::attribute() const
{
    return m_hpr->attribute;
}

const Data& WorkerEngine::meta() const
{
    return m_hpr->meta;
}

const Data& WorkerEngine::packageMeta() const
{
    return m_hpr->detail.package().meta();
}

void WorkerEngine::setCancelEnable(bool cancelEnable)
{
    m_hpr->cancelEnable = cancelEnable;
}

void WorkerEngine::setVersion(const std::string& version)
{
    m_hpr->version = version;
}

void WorkerEngine::setAttribute(const Data& attribute)
{
    m_hpr->attribute = attribute;
}

void WorkerEngine::setMeta(const Data& meta)
{
    m_hpr->meta = meta;
}

void WorkerEngine::subscibeDeploy(const DeployFunction& function)
{
    if (this->isRunning()) {
        LOG_WARNING("must be subscribed before the engine runs");
        return;
    }
    m_hpr->deployFunction = function;
}

void WorkerEngine::postDeployDone(bool success, int errorCode)
{
    if (m_hpr->detail.domain().state() != WR_DEPLOY && m_hpr->detail.domain().state() != WR_CANCEL) {
        LOG_WARNING("can not postDeployDone");
        return;
    }
    if (success) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_DEPLOY_DONE));
    } else {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", errorCode } }));
    }
}

void WorkerEngine::postDeployProgress(float progress, const std::string& message)
{
    if (m_hpr->detail.domain().state() != WR_DEPLOY && m_hpr->detail.domain().state() != WR_CANCEL) {
        LOG_WARNING("can not postDeployProgress");
        return;
    }
    this->postEvent(new WorkerEvent(WorkerEvent::RES_DEPLOY_PROGRESS, Data { { "progress", progress }, { "message", message } }));
}

void WorkerEngine::postCancelDone(bool success, int errorCode)
{
    if (m_hpr->detail.domain().state() != WR_CANCEL) {
        LOG_WARNING("can not postCancelDone");
        return;
    }
    if (success) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_CANCEL_DONE));
    } else {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", errorCode } }));
    }
}

void WorkerEngine::subscibeDetail(const DetailFunction& function)
{
    if (this->isRunning()) {
        LOG_WARNING("must be subscribed before the engine runs");
        return;
    }
    m_hpr->detailFunction = function;
}

void WorkerEngine::postDetailAnswer(Answer answer)
{
    if (!m_hpr->detailFunction || !Domain::mrStateIsAsk(m_hpr->masterState)) {
        LOG_WARNING("can not postDetailAnswer");
        return;
    }
    this->postEvent(new WorkerEvent(WorkerEvent::RES_ANSWER, Data { { "answer", answer } }));
}

void WorkerEngine::begin()
{
    onStart();
    m_hpr->replyControlTimer = createTimer(DCUS_PROCESS_DOMAIN_TIME, true, std::bind(&WorkerEngine::sendDomainMessage, this));
    m_hpr->replyControlTimer->start();
    sendDomainMessage();
}

void WorkerEngine::end()
{
    onStop();
}

void WorkerEngine::eventChanged(Event* event)
{
    WorkerEvent* workerEvent = (WorkerEvent*)event;
    if (workerEvent->type() == WorkerEvent::FUNCTION) {
        return;
    }
    if (workerEvent->isAccepted()) {
        return;
    }
    if (workerEvent->type() == WorkerEvent::RES_ERROR) {
        int error = workerEvent->data().value("error").toInt();
        setDomainState(WR_ERROR);
        m_hpr->detail.domain().error() = error;
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Errored !";
        sendDomainMessage();
        return;
    }
    switch (workerEvent->type()) {
    case WorkerEvent::REQ_CONTROL: {
        WorkerControlEvent* controlEvent = dynamic_cast<WorkerControlEvent*>(workerEvent);
        if (!controlEvent) {
            LOG_WARNING("get WorkerControlEvent error");
            break;
        }
        m_hpr->control = controlEvent->control();
        m_hpr->upgrade = controlEvent->upgrade();
        m_hpr->detail.domain().name() = m_hpr->name;
        m_hpr->detail.domain().guid() = m_hpr->guid;
        m_hpr->detail.domain().version() = m_hpr->version;
        m_hpr->detail.domain().attribute() = m_hpr->attribute;
        m_hpr->detail.domain().meta() = m_hpr->meta;
        for (const auto& packge : controlEvent->upgrade().packages()) {
            if (m_hpr->detail.domain().name() == packge.domain()) {
                m_hpr->detail.package() = packge;
                break;
            }
        }
        switch (controlEvent->control()) {
        case CTL_UNKNOWN: {
            LOG_WARNING("unknown control");
            break;
        }
        case CTL_RESET: {
            m_hpr->hasCancelAction = false;
            m_hpr->detail.domain().error() = 0;
            if (m_hpr->detail.domain().state() == WR_OFFLINE) {
                setDomainState(WR_IDLE);
            }
            break;
        }
        case CTL_DOWNLOAD: {
            m_hpr->hasCancelAction = false;
            if (m_hpr->detail.package().files().empty()) {
                break;
            }
            if (m_hpr->detail.domain().state() == WR_DOWNLOAD || m_hpr->detail.domain().state() == WR_ERROR) {
                break;
            }
            m_hpr->detail.domain().error() = 0;
            setDomainState(WR_DOWNLOAD);
            m_hpr->detail.domain().answer() = ANS_UNKNOWN;
            m_hpr->detail.domain().progress() = .0f;
            m_hpr->detail.domain().message() = "Download ...";
            stopThread();
            m_hpr->workThread.start(std::bind(&WorkerEngine::download, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
            break;
        }
        case CTL_DEPLOY: {
            m_hpr->hasCancelAction = false;
            if (m_hpr->detail.package().files().empty()) {
                break;
            }
            if (m_hpr->detail.domain().state() == WR_DEPLOY || m_hpr->detail.domain().state() == WR_ERROR) {
                break;
            }
            if (m_hpr->detail.detectVersionEqual() && m_hpr->detail.domain().state() == WR_IDLE) {
                break;
            }
            if (m_hpr->detail.domain().state() == WR_IDLE) {
                setDomainState(WR_WAIT);
            }
            if (m_hpr->detail.hasDepends(controlEvent->depends())) {
                break;
            }
            m_hpr->detail.domain().error() = 0;
            setDomainState(WR_DEPLOY);
            m_hpr->detail.domain().answer() = ANS_UNKNOWN;
            m_hpr->detail.domain().progress() = .0f;
            m_hpr->detail.domain().message() = "Deploy ...";
            stopThread();
            if (m_hpr->detail.detectVersionEqual()) {
                this->postEvent(new WorkerEvent(WorkerEvent::RES_DEPLOY_DONE));
                break;
            }
            if (!m_hpr->detail.detectVersionVaild()) {
                this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2900 } }));
                LOG_WARNING("deploy version not vaild(", m_hpr->detail.domain().version(), ")");
                break;
            }
            m_hpr->workThread.start(std::bind(&WorkerEngine::deploy, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
            break;
        }
        case CTL_CANCEL: {
            m_hpr->hasCancelAction = true;
            if (m_hpr->detail.package().files().empty()) {
                break;
            }
            if (m_hpr->detail.domain().state() == WR_CANCEL || m_hpr->detail.domain().state() == WR_ERROR) {
                break;
            }
            m_hpr->detail.domain().error() = 0;
            setDomainState(WR_CANCEL);
            m_hpr->detail.domain().message() = "Cancel ...";
            if (m_hpr->detail.detectVersionEqual()) {
                this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2901 } }));
                break;
            }
            if (m_hpr->detail.domain().last() == WR_IDLE) {
                this->postEvent(new WorkerEvent(WorkerEvent::RES_CANCEL_DONE));
                break;
            }
            if (m_hpr->cancelEnable) {
                if (m_hpr->detail.domain().last() == WR_WAIT) {
                    this->postEvent(new WorkerEvent(WorkerEvent::RES_CANCEL_DONE));
                }
            } else {
                if (m_hpr->detail.domain().last() == WR_DEPLOY) {
                    this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2902 } }));
                }
            }
            break;
        }
        case CTL_CLEAR: {
            m_hpr->hasCancelAction = false;
            stopThread();
            setDomainState(WR_IDLE);
            m_hpr->detail.domain().answer() = ANS_UNKNOWN;
            m_hpr->detail.domain().progress() = .0f;
            m_hpr->detail.domain().message() = "";
            break;
        }
        default:
            break;
        }
        sendDomainMessage();
        break;
    }
    case WorkerEvent::REQ_DETAIL: {
        WorkerDetailEvent* detailEvent = dynamic_cast<WorkerDetailEvent*>(workerEvent);
        if (!detailEvent) {
            LOG_WARNING("get WorkerDetailEvent error");
            break;
        }
        bool stateChanged = false;
        if (m_hpr->masterState != detailEvent->detailMessage().state()) {
            m_hpr->masterState = detailEvent->detailMessage().state();
            stateChanged = true;
        }
        m_hpr->detailFunction(detailEvent->detailMessage(), stateChanged);
        break;
    }
    case WorkerEvent::RES_DOWNLOAD: {
        setDomainState(WR_VERIFY);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Verify ...";
        stopThread();
        m_hpr->workThread.start(std::bind(&WorkerEngine::verify, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
        sendDomainMessage();
        break;
    }
    case WorkerEvent::RES_VERIFY: {
        FilePaths patchPaths;
        for (const auto& file : m_hpr->detail.package().files()) {
            if (Common::getPathSuffixName(file.name()) == "patch") {
                patchPaths.push_back(m_hpr->downloadDir + "/" + m_hpr->upgrade.id() + "/" + file.domain() + "/" + file.name());
            }
        }
        if (patchPaths.empty() || !m_hpr->patchNewFiles.empty()) {
            m_hpr->patchNewFiles.clear();
            m_hpr->patchNewFiles.shrink_to_fit();
            setDomainState(WR_WAIT);
            m_hpr->detail.domain().answer() = ANS_UNKNOWN;
            m_hpr->detail.domain().progress() = .0f;
            m_hpr->detail.domain().message() = "Waiting for deploy ...";
            sendDomainMessage();
        } else {
            setDomainState(WR_PATCH);
            m_hpr->detail.domain().answer() = ANS_UNKNOWN;
            m_hpr->detail.domain().progress() = .0f;
            m_hpr->detail.domain().message() = "Patch ...";
            stopThread();
            m_hpr->workThread.start(std::bind(&WorkerEngine::patch, this, patchPaths));
            sendDomainMessage();
        }
        break;
    }
    case WorkerEvent::RES_PATCH: {
        setDomainState(WR_VERIFY);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Verify ...";
        stopThread();
        m_hpr->workThread.start(std::bind(&WorkerEngine::verify, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
        sendDomainMessage();
        break;
    }
    case WorkerEvent::RES_ANSWER: {
        int answer = workerEvent->data().value("answer").toInt();
        m_hpr->detail.domain().answer() = (Answer)answer;
        sendDomainMessage();
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        break;
    }
    case WorkerEvent::RES_TRANSFER_PROGRESS: {
        WorkerTransferEvent* transferEvent = dynamic_cast<WorkerTransferEvent*>(workerEvent);
        if (!transferEvent) {
            LOG_WARNING("get WorkerTransferEvent error");
            break;
        }
        uint32_t current = 0;
        uint32_t total = 0;
        for (const auto& file : m_hpr->detail.package().files()) {
            total += uint32_t(file.size() / 1024);
        }
        for (const auto& transfer : transferEvent->transfers()) {
            if (transfer.domain() == m_hpr->detail.domain().name()) {
                current += transfer.current();
            }
        }
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = current * 100.0f / total;
        sendDomainMessage();
        break;
    }
    case WorkerEvent::RES_DEPLOY_DONE: {
        m_hpr->version = m_hpr->detail.package().version();
        setDomainState(WR_IDLE);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = 100.0f;
        m_hpr->detail.domain().message() = "Deploy succeed !";
        m_hpr->detail.domain().version() = m_hpr->version;
        sendDomainMessage();
        break;
    }
    case WorkerEvent::RES_CANCEL_DONE: {
        setDomainState(WR_IDLE);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Cancel done !";
        sendDomainMessage();
        break;
    }
    case WorkerEvent::RES_DEPLOY_PROGRESS: {
        double progress = workerEvent->data().value("progress").toDouble();
        std::string message = workerEvent->data().value("message").toString();
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = (float)progress;
        m_hpr->detail.domain().message() = message;
        sendDomainMessage();
        break;
    }
    default:
        break;
    }
}

bool WorkerEngine::needDiscovery() const
{
    return m_hpr->needDiscovery;
}

bool WorkerEngine::hasSubscibeDeploy() const
{
    if (m_hpr->deployFunction) {
        return true;
    }
    return false;
}

bool WorkerEngine::hasSubscibeDetail() const
{
    if (m_hpr->detailFunction) {
        return true;
    }
    return false;
}

void WorkerEngine::processControlMessage(Control control, Upgrade&& upgrade, Depends&& depends)
{
    this->postEvent(new WorkerControlEvent(control, upgrade, depends));
}

void WorkerEngine::processDetailMessage(DetailMessage&& detailMessage)
{
    if (!m_hpr->detailFunction) {
        return;
    }
    this->postEvent(new WorkerDetailEvent(detailMessage));
}

bool WorkerEngine::checkControlMessageId(uint32_t id) const
{
    if (m_hpr->controlMessageId != id) {
        m_hpr->controlMessageId = id;
        return false;
    }
    return true;
}

bool WorkerEngine::checkDetailMessageId(uint32_t id) const
{
    if (m_hpr->detailMessageId != id) {
        m_hpr->detailMessageId = id;
        return false;
    }
    return true;
}

void WorkerEngine::setDomainState(WorkerState state)
{
    if (m_hpr->detail.domain().state() != state) {
        m_hpr->detail.domain().last() = m_hpr->detail.domain().state();
        m_hpr->detail.domain().state() = state;
    }
}

void WorkerEngine::sendDomainMessage()
{
    m_hpr->detail.domain().name() = m_hpr->name;
    m_hpr->detail.domain().guid() = m_hpr->guid;
    m_hpr->detail.domain().version() = m_hpr->version;
    m_hpr->detail.domain().attribute() = m_hpr->attribute;
    m_hpr->detail.domain().meta() = m_hpr->meta;
    if (m_hpr->detailFunction) {
        m_hpr->detail.domain().watcher() = true;
    } else {
        m_hpr->detail.domain().watcher() = false;
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
    }
    bool ok = onSendDomainMessage(m_hpr->detail.domain(), m_hpr->needDiscovery);
    if (ok) {
        m_hpr->needDiscovery = false;
    } else {
        m_hpr->needDiscovery = true;
        m_hpr->controlMessageId = 0;
        m_hpr->detailMessageId = 0;
        if (m_hpr->detailFunction) {
            if (m_hpr->masterState != MR_OFFLINE) {
                DetailMessage detailMessage;
                detailMessage.state() = MR_OFFLINE;
                detailMessage.last() = m_hpr->masterState;
                m_hpr->detailFunction(detailMessage, true);
                m_hpr->masterState = MR_OFFLINE;
            }
        }
    }
}

void WorkerEngine::stopThread(bool force)
{
    m_hpr->workThread.setReadyFinished(true);
    if (force) {
        m_hpr->workThread.stop();
    } else {
        m_hpr->workThread.stop(3000);
    }
}

void WorkerEngine::download(const std::string& id, const Files& files)
{
    Core::Status status;
    auto stopFunction = [this]() {
        if (m_hpr->hasCancelAction) {
            return true;
        }
        if (m_hpr->workThread.isReadyFinished()) {
            return true;
        }
        return false;
    };
    Common::removeSubOldDirs(m_hpr->downloadDir, DCUS_DOWNLOAD_KEEP_FILE_COUNT);
    int times = 0;
    while (times < DCUS_RETRY_TIMES) {
        if (times > 0) {
            LOG_WARNING("retry download");
        }
        status = Core::download(m_hpr->downloadDir + "/" + id, files, worker_config,
            stopFunction);
        if (status.state() != Core::FAILED) {
            break;
        }
        times++;
    }
    if (status.state() == Core::SUCCEED) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_DOWNLOAD));
    } else if (status.state() == Core::FAILED) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2000 + status.error() } }));
    } else {
        if (m_hpr->hasCancelAction) {
            this->postEvent(new WorkerEvent(WorkerEvent::RES_CANCEL_DONE));
        } else {
            LOG_WARNING("force quit");
        }
    }
}

void WorkerEngine::verify(const std::string& id, const Files& files)
{
    auto stopFunction = [this]() {
        if (m_hpr->hasCancelAction) {
            return true;
        }
        if (m_hpr->workThread.isReadyFinished()) {
            return true;
        }
        return false;
    };
    Core::Status status = Core::verify(m_hpr->downloadDir + "/" + id, files,
        stopFunction,
        [this](const Transfers& transfers) {
            DCUS_UNUSED(transfers);
            // this->postEvent(new WorkerTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_VERIFY));
    } else if (status.state() == Core::FAILED) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2000 + status.error() } }));
    } else {
        if (m_hpr->hasCancelAction) {
            this->postEvent(new WorkerEvent(WorkerEvent::RES_CANCEL_DONE));
        } else {
            LOG_WARNING("force quit");
        }
    }
}

void WorkerEngine::patch(const FilePaths& patchPaths)
{
    auto stopFunction = [this]() {
        if (m_hpr->hasCancelAction) {
            return true;
        }
        if (m_hpr->workThread.isReadyFinished()) {
            return true;
        }
        return false;
    };
    m_hpr->patchNewFiles.clear();
    m_hpr->patchNewFiles.shrink_to_fit();
    Core::Status status = Core::patch(m_hpr->downloadDir, patchPaths,
        m_hpr->patchNewFiles,
        stopFunction,
        [this](const Transfers& transfers) {
            DCUS_UNUSED(transfers);
            // this->postEvent(new WorkerTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_PATCH));
    } else if (status.state() == Core::FAILED) {
        this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2000 + status.error() } }));
    } else {
        if (m_hpr->hasCancelAction) {
            this->postEvent(new WorkerEvent(WorkerEvent::RES_CANCEL_DONE));
        } else {
            LOG_WARNING("force quit");
        }
    }
}

void WorkerEngine::deploy(const std::string& id, const Files& files)
{
    std::string dir;
    FilePaths filePaths;
    for (const auto& file : files) {
        dir = m_hpr->downloadDir + "/" + id + "/" + file.domain();
        const std::string& path = dir + "/" + file.name();
        if (!Common::exists(path)) {
            LOG_WARNING("deploy file is not exists(", path, ")");
            this->postEvent(new WorkerEvent(WorkerEvent::RES_ERROR, Data { { "error", 2910 } }));
            return;
        }
        filePaths.push_back(path);
    }
    if (m_hpr->deployFunction) {
        m_hpr->deployFunction(dir, filePaths);
    }
}

DCUS_NAMESPACE_END
