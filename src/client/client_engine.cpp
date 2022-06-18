/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#include "dcus/client/client_engine.h"
#include "core/core.h"
#include "dcus/base/thread.h"
#include "dcus/client/client_event.h"
#include "dcus/setting.h"
#include "dcus/utils/dir.h"
#include "dcus/utils/system.h"

#define m_hpr m_clientHelper

#define USAGE_NAME_LIST       \
    {                         \
        "-n", "--n", "--name" \
    }
#define USAGE_GUID_LIST       \
    {                         \
        "-g", "--g", "--guid" \
    }

DCUS_NAMESPACE_BEGIN

struct ClientHelper {
    Thread workThread;
    Upgrade upgrade;
    Detail detail;
    VariantMap attribute;
    VariantMap meta;
    std::string name;
    std::string guid;
    std::string version;
    std::string downloadDir;
    Timer* replyControlTimer = nullptr;
    ClientEngine::DeployFunction deployFunction = nullptr;
    ClientEngine::DetailFunction detailFunction = nullptr;
    std::atomic<uint32_t> controlMessageId { 0 };
    std::atomic<uint32_t> detailMessageId { 0 };
    std::atomic_bool hasCancelAction { false };
    std::atomic_bool cancelEnable { true };
    std::atomic_bool needDiscovery { true };
    std::atomic<ServerState> serverState { MR_OFFLINE };
    std::atomic<Control> control = { CTL_UNKNOWN };
    Files patchNewFiles;
    static std::string _clientName;
    static std::string _clientGuid;
    static bool _hasRegister;
};

std::string ClientHelper::_clientName;
std::string ClientHelper::_clientGuid;
bool ClientHelper::_hasRegister = false;

void ClientEngine::registerDomain(const std::string& name, const std::string& guid)
{
    if (ClientEngine::getInstance(false)) {
        LOG_WARNING("must be registered in client engine creation");
        return;
    }
    ClientHelper::_clientName = name;
    ClientHelper::_clientGuid = guid;
    ClientHelper::_hasRegister = true;
}

void ClientEngine::destroy()
{
    if (getInstance(false)) {
        delete getInstance();
    }
}

ClientEngine::ClientEngine(int argc, char** argv)
    : Queue(DCUS_QUEUE_ID_CLIENT)
    , Application(argc, argv, "dcus_client")
{
    if (getInstance(false)) {
        std::terminate();
        return;
    }
    setInstance(this);
    loadUseage({ USAGE_NAME_LIST, USAGE_GUID_LIST });

    if (!m_hpr) {
        m_hpr = new ClientHelper;
    }
    static std::mutex mutex;
    setMutex(mutex);

    if (dcus_client_config.value("download_dir").isValid()) {
        m_hpr->downloadDir = dcus_client_config.value("download_dir").toString();
    } else {
        m_hpr->downloadDir = Utils::getTempDir() + "/dcus_client_tmp";
    }

    if (ClientHelper::_hasRegister) {
        m_hpr->name = ClientHelper::_clientName;
        m_hpr->guid = ClientHelper::_clientGuid;
    } else {
        Variant name = "unknown";
        getOptions(name, USAGE_NAME_LIST, "name");
        m_hpr->name = name.toString();
        Variant guid = "";
        getOptions(guid, USAGE_GUID_LIST, "guid");
        m_hpr->guid = guid.toString();
    }

    m_hpr->detail.domain().state() = WR_IDLE;
    m_hpr->detail.domain().last() = WR_OFFLINE;
}

ClientEngine::~ClientEngine()
{
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
    setInstance(nullptr);
}

const std::string& ClientEngine::name() const
{
    return m_hpr->name;
}

const std::string& ClientEngine::guid() const
{
    return m_hpr->guid;
}

const Upgrade& ClientEngine::upgrade() const
{
    return m_hpr->upgrade;
}

Control ClientEngine::control() const
{
    return m_hpr->control;
}

bool ClientEngine::hasCancelAction() const
{
    return m_hpr->hasCancelAction;
}

bool ClientEngine::hasStopAction() const
{
    return m_hpr->workThread.isReadyFinished();
}

bool ClientEngine::cancelEnable() const
{
    return m_hpr->cancelEnable;
}

const std::string& ClientEngine::version() const
{
    return m_hpr->version;
}

const VariantMap& ClientEngine::attribute() const
{
    return m_hpr->attribute;
}

const VariantMap& ClientEngine::meta() const
{
    return m_hpr->meta;
}

const VariantMap& ClientEngine::packageMeta() const
{
    return m_hpr->detail.package().meta();
}

void ClientEngine::setCancelEnable(bool cancelEnable)
{
    m_hpr->cancelEnable = cancelEnable;
}

void ClientEngine::setVersion(const std::string& version)
{
    m_hpr->version = version;
}

void ClientEngine::setAttribute(const VariantMap& attribute)
{
    m_hpr->attribute = attribute;
}

void ClientEngine::setMeta(const VariantMap& meta)
{
    m_hpr->meta = meta;
}

void ClientEngine::subscibeDeploy(const DeployFunction& function)
{
    if (this->isRunning()) {
        LOG_WARNING("must be subscribed before the engine runs");
        return;
    }
    m_hpr->deployFunction = function;
}

void ClientEngine::postDeployDone(bool success, int errorCode)
{
    if (m_hpr->detail.domain().state() != WR_DEPLOY && m_hpr->detail.domain().state() != WR_CANCEL) {
        LOG_WARNING("can not postDeployDone");
        return;
    }
    if (success) {
        this->postEvent(new ClientEvent(ClientEvent::RES_DEPLOY_DONE));
    } else {
        this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", errorCode } }));
    }
}

void ClientEngine::postDeployProgress(float progress, const std::string& message)
{
    if (m_hpr->detail.domain().state() != WR_DEPLOY && m_hpr->detail.domain().state() != WR_CANCEL) {
        LOG_WARNING("can not postDeployProgress");
        return;
    }
    this->postEvent(new ClientEvent(ClientEvent::RES_DEPLOY_PROGRESS, VariantMap { { "progress", progress }, { "message", message } }));
}

void ClientEngine::postCancelDone(bool success, int errorCode)
{
    if (m_hpr->detail.domain().state() != WR_CANCEL) {
        LOG_WARNING("can not postCancelDone");
        return;
    }
    if (success) {
        this->postEvent(new ClientEvent(ClientEvent::RES_CANCEL_DONE));
    } else {
        this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", errorCode } }));
    }
}

void ClientEngine::subscibeDetail(const DetailFunction& function)
{
    if (this->isRunning()) {
        LOG_WARNING("must be subscribed before the engine runs");
        return;
    }
    m_hpr->detailFunction = function;
}

void ClientEngine::postDetailAnswer(Answer answer)
{
    if (!m_hpr->detailFunction || !Domain::mrStateIsAsk(m_hpr->serverState)) {
        LOG_WARNING("can not postDetailAnswer");
        return;
    }
    this->postEvent(new ClientEvent(ClientEvent::RES_ANSWER, VariantMap { { "answer", answer } }));
}

void ClientEngine::begin()
{
    onStart();
    m_hpr->replyControlTimer = createTimer(DCUS_PROCESS_DOMAIN_TIME, true, std::bind(&ClientEngine::sendDomainMessage, this));
    m_hpr->replyControlTimer->start();
    sendDomainMessage();
}

void ClientEngine::end()
{
    onStop();
}

void ClientEngine::eventChanged(Event* event)
{
    ClientEvent* clientEvent = (ClientEvent*)event;
    if (clientEvent->type() == ClientEvent::FUNCTION) {
        return;
    }
    if (clientEvent->isAccepted()) {
        return;
    }
    if (clientEvent->type() == ClientEvent::RES_ERROR) {
        int error = clientEvent->data().value("error").toInt();
        setDomainState(WR_ERROR);
        m_hpr->detail.domain().error() = error;
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Errored !";
        sendDomainMessage();
        return;
    }
    switch (clientEvent->type()) {
    case ClientEvent::REQ_CONTROL: {
        ClientControlEvent* controlEvent = dynamic_cast<ClientControlEvent*>(clientEvent);
        if (!controlEvent) {
            LOG_WARNING("get ClientControlEvent error");
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
            m_hpr->workThread.start(std::bind(&ClientEngine::download, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
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
                this->postEvent(new ClientEvent(ClientEvent::RES_DEPLOY_DONE));
                break;
            }
            if (!m_hpr->detail.detectVersionVaild()) {
                this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2900 } }));
                LOG_WARNING("deploy version not vaild(", m_hpr->detail.domain().version(), ")");
                break;
            }
            m_hpr->workThread.start(std::bind(&ClientEngine::deploy, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
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
                this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2901 } }));
                break;
            }
            if (m_hpr->detail.domain().last() == WR_IDLE) {
                this->postEvent(new ClientEvent(ClientEvent::RES_CANCEL_DONE));
                break;
            }
            if (m_hpr->cancelEnable) {
                if (m_hpr->detail.domain().last() == WR_WAIT) {
                    this->postEvent(new ClientEvent(ClientEvent::RES_CANCEL_DONE));
                }
            } else {
                if (m_hpr->detail.domain().last() == WR_DEPLOY) {
                    this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2902 } }));
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
            Utils::freeUnusedMemory();
            break;
        }
        default:
            break;
        }
        sendDomainMessage();
        break;
    }
    case ClientEvent::REQ_DETAIL: {
        ClientDetailEvent* detailEvent = dynamic_cast<ClientDetailEvent*>(clientEvent);
        if (!detailEvent) {
            LOG_WARNING("get ClientDetailEvent error");
            break;
        }
        bool stateChanged = false;
        if (m_hpr->serverState != detailEvent->detailMessage().state()) {
            m_hpr->serverState = detailEvent->detailMessage().state();
            stateChanged = true;
        }
        m_hpr->detailFunction(detailEvent->detailMessage(), stateChanged);
        break;
    }
    case ClientEvent::RES_DOWNLOAD: {
        setDomainState(WR_VERIFY);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Verify ...";
        stopThread();
        m_hpr->workThread.start(std::bind(&ClientEngine::verify, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
        sendDomainMessage();
        break;
    }
    case ClientEvent::RES_VERIFY: {
        FilePaths patchPaths;
        for (const auto& file : m_hpr->detail.package().files()) {
            if (Utils::getPathSuffixName(file.name()) == "patch") {
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
            m_hpr->workThread.start(std::bind(&ClientEngine::patch, this, patchPaths));
            sendDomainMessage();
        }
        break;
    }
    case ClientEvent::RES_PATCH: {
        setDomainState(WR_VERIFY);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Verify ...";
        stopThread();
        m_hpr->workThread.start(std::bind(&ClientEngine::verify, this, m_hpr->upgrade.id(), m_hpr->detail.package().files()));
        sendDomainMessage();
        break;
    }
    case ClientEvent::RES_ANSWER: {
        int answer = clientEvent->data().value("answer").toInt();
        m_hpr->detail.domain().answer() = (Answer)answer;
        sendDomainMessage();
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        break;
    }
    case ClientEvent::RES_TRANSFER_PROGRESS: {
        ClientTransferEvent* transferEvent = dynamic_cast<ClientTransferEvent*>(clientEvent);
        if (!transferEvent) {
            LOG_WARNING("get ClientTransferEvent error");
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
    case ClientEvent::RES_DEPLOY_DONE: {
        m_hpr->version = m_hpr->detail.package().version();
        setDomainState(WR_IDLE);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = 100.0f;
        m_hpr->detail.domain().message() = "Deploy succeed !";
        m_hpr->detail.domain().version() = m_hpr->version;
        sendDomainMessage();
        break;
    }
    case ClientEvent::RES_CANCEL_DONE: {
        setDomainState(WR_IDLE);
        m_hpr->detail.domain().answer() = ANS_UNKNOWN;
        m_hpr->detail.domain().progress() = .0f;
        m_hpr->detail.domain().message() = "Cancel done !";
        sendDomainMessage();
        break;
    }
    case ClientEvent::RES_DEPLOY_PROGRESS: {
        double progress = clientEvent->data().value("progress").toDouble();
        std::string message = clientEvent->data().value("message").toString();
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

bool ClientEngine::needDiscovery() const
{
    return m_hpr->needDiscovery;
}

bool ClientEngine::hasSubscibeDeploy() const
{
    if (m_hpr->deployFunction) {
        return true;
    }
    return false;
}

bool ClientEngine::hasSubscibeDetail() const
{
    if (m_hpr->detailFunction) {
        return true;
    }
    return false;
}

void ClientEngine::processControlMessage(Control control, Upgrade&& upgrade, Depends&& depends)
{
    this->postEvent(new ClientControlEvent(control, upgrade, depends));
}

void ClientEngine::processDetailMessage(DetailMessage&& detailMessage)
{
    if (!m_hpr->detailFunction) {
        return;
    }
    this->postEvent(new ClientDetailEvent(detailMessage));
}

bool ClientEngine::checkControlMessageId(uint32_t id) const
{
    if (m_hpr->controlMessageId != id) {
        m_hpr->controlMessageId = id;
        return false;
    }
    return true;
}

bool ClientEngine::checkDetailMessageId(uint32_t id) const
{
    if (m_hpr->detailMessageId != id) {
        m_hpr->detailMessageId = id;
        return false;
    }
    return true;
}

void ClientEngine::setDomainState(ClientState state)
{
    if (m_hpr->detail.domain().state() != state) {
        m_hpr->detail.domain().last() = m_hpr->detail.domain().state();
        m_hpr->detail.domain().state() = state;
    }
}

void ClientEngine::sendDomainMessage()
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
            if (m_hpr->serverState != MR_OFFLINE) {
                DetailMessage detailMessage;
                detailMessage.state() = MR_OFFLINE;
                detailMessage.last() = m_hpr->serverState;
                m_hpr->detailFunction(detailMessage, true);
                m_hpr->serverState = MR_OFFLINE;
            }
        }
    }
}

void ClientEngine::stopThread(bool force)
{
    m_hpr->workThread.setReadyFinished(true);
    if (force) {
        m_hpr->workThread.stop();
    } else {
        m_hpr->workThread.stop(3000);
    }
}

void ClientEngine::download(const std::string& id, const Files& files)
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
    Utils::removeSubOldDirs(m_hpr->downloadDir, DCUS_DOWNLOAD_KEEP_FILE_COUNT);
    int times = 0;
    while (times < DCUS_RETRY_TIMES) {
        if (times > 0) {
            LOG_WARNING("retry download");
        }
        status = Core::download(m_hpr->downloadDir + "/" + id, files, dcus_client_config,
            stopFunction);
        if (status.state() != Core::FAILED) {
            break;
        }
        times++;
    }
    if (status.state() == Core::SUCCEED) {
        this->postEvent(new ClientEvent(ClientEvent::RES_DOWNLOAD));
    } else if (status.state() == Core::FAILED) {
        this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2000 + status.error() } }));
    } else {
        if (m_hpr->hasCancelAction) {
            this->postEvent(new ClientEvent(ClientEvent::RES_CANCEL_DONE));
        } else {
            LOG_WARNING("force quit");
        }
    }
}

void ClientEngine::verify(const std::string& id, const Files& files)
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
            // this->postEvent(new ClientTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        this->postEvent(new ClientEvent(ClientEvent::RES_VERIFY));
    } else if (status.state() == Core::FAILED) {
        this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2000 + status.error() } }));
    } else {
        if (m_hpr->hasCancelAction) {
            this->postEvent(new ClientEvent(ClientEvent::RES_CANCEL_DONE));
        } else {
            LOG_WARNING("force quit");
        }
    }
}

void ClientEngine::patch(const FilePaths& patchPaths)
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
            // this->postEvent(new ClientTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        this->postEvent(new ClientEvent(ClientEvent::RES_PATCH));
    } else if (status.state() == Core::FAILED) {
        this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2000 + status.error() } }));
    } else {
        if (m_hpr->hasCancelAction) {
            this->postEvent(new ClientEvent(ClientEvent::RES_CANCEL_DONE));
        } else {
            LOG_WARNING("force quit");
        }
    }
}

void ClientEngine::deploy(const std::string& id, const Files& files)
{
    std::string dir;
    FilePaths filePaths;
    for (const auto& file : files) {
        dir = m_hpr->downloadDir + "/" + id + "/" + file.domain();
        const std::string& path = dir + "/" + file.name();
        if (!Utils::exists(path)) {
            LOG_WARNING("deploy file is not exists(", path, ")");
            this->postEvent(new ClientEvent(ClientEvent::RES_ERROR, VariantMap { { "error", 2910 } }));
            return;
        }
        filePaths.push_back(path);
    }
    if (m_hpr->deployFunction) {
        m_hpr->deployFunction(dir, filePaths);
    }
}

DCUS_NAMESPACE_END
