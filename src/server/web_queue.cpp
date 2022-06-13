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

#include "dcus/server/web_queue.h"
#include "core/core.h"
#include "dcus/base/thread.h"
#include "dcus/server/server_engine.h"
#include "dcus/server/server_event.h"
#include "dcus/server/web_event.h"
#include "dcus/utils/dir.h"
#include "dcus/utils/host.h"
#include <atomic>

#define m_hpr m_webHelper

DCUS_NAMESPACE_BEGIN

struct WebHelper {
    bool hasInited = false;
    int distributePort = 0;
    int checkInterval = DCUS_WEB_CHECK_INTERVAL;
    std::atomic<WebQueue::State> state;
    std::string downloadDir;
    std::string localUrl;
    std::string ipAddress;
    std::string distributeUrl;
    Timer* checkTimer = nullptr;
    Core::DistributeHandle distributeHandle = { nullptr };
    Thread workThread;
};

WebQueue::WebQueue()
    : Queue(DCUS_QUEUE_ID_WEB)
{
    if (!m_hpr) {
        m_hpr = new WebHelper;
    }
    if (server_config.value("net_interface").valid()) {
        m_hpr->ipAddress = Utils::getIpAddress(server_config.value("net_interface").toString());
    } else {
        m_hpr->ipAddress = Utils::getIpAddress();
    }
    if (server_config.value("download_dir").valid()) {
        m_hpr->downloadDir = server_config.value("download_dir").toString();
    } else {
        m_hpr->downloadDir = Utils::getTempDir() + "/dcus_server_tmp";
    }
    if (server_config.value("distribute_url").valid()) {
        m_hpr->distributeUrl = server_config.value("distribute_url").toString();
    } else {
        m_hpr->distributeUrl = "0.0.0.0";
    }
    if (server_config.value("distribute_port").valid()) {
        m_hpr->distributePort = server_config.value("distribute_port").toInt();
    } else {
#ifdef DCUS_USE_HTTPS
        m_hpr->distributePort = 9443;
#else
        m_hpr->distributePort = 9080;
#endif
    }
    if (server_config.value("upgrade_check_interval").valid()) {
        m_hpr->checkInterval = (uint32_t)server_config.value("upgrade_check_interval").toInt();
    }
#if defined(DCUS_USE_DOWNLOAD_HTTP) && defined(DCUS_USE_DISTRIBUTE_HTTP)
#ifdef DCUS_USE_HTTPS
    std::string protocName = "https://";
#else
    std::string protocName = "http://";
#endif
#endif
    m_hpr->localUrl = protocName + m_hpr->ipAddress + ":" + std::to_string(m_hpr->distributePort) + "/" + DCUS_WEB_PULL_FUNC_NAME;

    m_hpr->checkTimer = createTimer(m_hpr->checkInterval, true, std::bind(&WebQueue::checkUpgrade, this));
    m_hpr->workThread.setEndFunction([this]() {
        m_hpr->state = WEB_IDLE;
    });
}

WebQueue::~WebQueue()
{
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
}

WebQueue::State WebQueue::state() const
{
    return m_hpr->state;
}

const std::string& WebQueue::localUrl() const
{
    return m_hpr->localUrl;
}

void WebQueue::setCheckInterval(int time)
{
    m_hpr->checkTimer->setInterval(time);
}

void WebQueue::begin()
{
}

void WebQueue::end()
{
    Core::stopDistribute(m_hpr->distributeHandle);
}

void WebQueue::eventChanged(Event* event)
{
    WebEvent* webEvent = (WebEvent*)event;
    if (webEvent->type() == WebEvent::FUNCTION) {
        return;
    }
    if (webEvent->isAccepted()) {
        return;
    }
    switch (webEvent->type()) {
    case WebEvent::REQ_INIT: {
        WebInitEvent* webInitEvent = dynamic_cast<WebInitEvent*>(event);
        if (!webInitEvent) {
            LOG_WARNING("get WebInitEvent error");
            break;
        }
        if (m_hpr->hasInited) {
            LOG_WARNING("has inited");
            break;
        }
        bool success = init(webInitEvent->webInit());
        server_engine->postEvent(new ServerEvent(ServerEvent::RES_INIT, Data { { "success", success } }));
        if (success) {
            checkUpgrade();
            m_hpr->checkTimer->start();
            m_hpr->hasInited = true;
        } else {
            LOG_WARNING("init error");
        }
        break;
    }
    case WebEvent::REQ_CHECK: {
        checkUpgrade();
        break;
    }
    case WebEvent::REQ_DOWNLOAD: {
        m_hpr->state = WEB_DOWNLOAD;
        stopThread();
        std::string id;
        Files files;
        transformFiles(id, files, true);
        m_hpr->workThread.start(std::bind(&WebQueue::download, this, id, files));
        break;
    }
    case WebEvent::REQ_VERIFY: {
        m_hpr->state = WEB_VERIFY;
        stopThread();
        std::string id;
        Files files;
        transformFiles(id, files, false);
        m_hpr->workThread.start(std::bind(&WebQueue::verify, this, id, files));
        break;
    }
    case WebEvent::REQ_DISTRIBUTE: {
        m_hpr->state = WEB_DISTRIBUTE;
        stopThread();
        std::string id;
        Files files;
        transformFiles(id, files, false);
        m_hpr->workThread.start(std::bind(&WebQueue::distribute, this, id, files));
        bool canPull = false;
        while (m_hpr->workThread.isRunning() && !isReadyToQuit()) {
            wait(10);
            if (m_hpr->distributeHandle) {
                wait(10);
                canPull = true;
                break;
            }
        }
        if (canPull) {
            server_engine->postEvent(new ServerEvent(ServerEvent::REQ_PULL));
        }
        break;
    }
    case WebEvent::REQ_STOP: {
        m_hpr->state = WEB_IDLE;
        stopThread();
        break;
    }
    case WebEvent::REQ_CLEAR: {
        m_hpr->state = WEB_IDLE;
        stopThread();
        removeCache(false);
        break;
    }
    case WebEvent::REQ_CLEARALL: {
        m_hpr->state = WEB_IDLE;
        stopThread();
        removeCache(true);
        break;
    }
    case WebEvent::REQ_FEEDBACK: {
        WebFeedEvent* webFeedEvent = dynamic_cast<WebFeedEvent*>(event);
        if (!webFeedEvent) {
            LOG_WARNING("get WebFeedbackEvent error");
            break;
        }
        bool success = feedback(webFeedEvent->webFeed());
        server_engine->postEvent(new ServerEvent(ServerEvent::RES_FEEDBACK_DONE, Data { { "success", success } }));
        break;
    }
    default:
        break;
    }
}

void WebQueue::postError(int errorCode)
{
    server_engine->postEvent(new ServerEvent(ServerEvent::RES_ERROR, Data { { "error", errorCode } }));
}

void WebQueue::postIdle()
{
    server_engine->postEvent(new ServerEvent(ServerEvent::REQ_IDLE));
}

void WebQueue::postUpgrade(Upgrade&& upgrade)
{
    lock();
    if (server_engine->upgrade() == upgrade) {
        unlock();
        return;
    }
    unlock();
    server_engine->postEvent(new ServerUpgradeEvent(upgrade));
}

void WebQueue::postCancel(std::string&& id)
{
    lock();
    if (server_engine->cancelId() == id) {
        unlock();
        return;
    }
    unlock();
    server_engine->postEvent(new ServerEvent(ServerEvent::REQ_CANCEL, Data { { "id", id } }));
}

void WebQueue::setCheckTimerInterval(int interval)
{
    m_hpr->checkTimer->setInterval(interval);
}

void WebQueue::checkUpgrade()
{
    bool active = detect();
    if (server_engine->isActive() != active) {
        server_engine->postEvent(new ServerEvent(ServerEvent::REQ_ACTIVE, Data { { "active", active } }));
    }
}

void WebQueue::removeCache(bool all)
{
    lock();
    std::string id = server_engine->upgrade().id();
    std::string cancelId = server_engine->cancelId();
    unlock();
    std::string downloadDir = m_hpr->downloadDir;
    if (all) {
        if (Utils::exists(downloadDir)) {
            Utils::removeAll(downloadDir);
        }
    } else {
        if (!id.empty()) {
            if (Utils::exists(downloadDir + "/" + id)) {
                Utils::removeAll(downloadDir + "/" + id);
            }
        }
        if (!cancelId.empty()) {
            if (Utils::exists(downloadDir + "/" + cancelId)) {
                Utils::removeAll(downloadDir + "/" + cancelId);
            }
        }
    }
}

void WebQueue::stopThread(bool force)
{
    Core::stopDistribute(m_hpr->distributeHandle);
    m_hpr->workThread.setReadyFinished(true);
    if (force) {
        m_hpr->workThread.stop();
    } else {
        m_hpr->workThread.stop(3000);
    }
}

void WebQueue::transformFiles(std::string& id, Files& files, bool webInstead)
{
    lock();
    id = server_engine->upgrade().id();
    files.clear();
    files.shrink_to_fit();
    for (const Package& package : server_engine->upgrade().packages()) {
        for (File file : package.files()) {
            if (webInstead) {
                file.url() = file.web_url();
            }
            files.push_back(std::move(file));
        }
    }
    unlock();
}

void WebQueue::download(const std::string& id, const Files& files)
{
    Utils::removeSubOldDirs(m_hpr->downloadDir, DCUS_DOWNLOAD_KEEP_FILE_COUNT);
    auto status = Core::pull(
        m_hpr->downloadDir + "/" + id, files, server_config,
        [this]() {
            return m_hpr->workThread.isReadyFinished();
        },
        [](const Transfers& transfers) {
            server_engine->postEvent(new ServerTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        server_engine->postEvent(new ServerEvent(ServerEvent::RES_DOWNLOAD_DONE));
    } else if (status.state() == Core::FAILED) {
        postError(status.error());
    }
}

void WebQueue::verify(const std::string& id, const Files& files)
{
    auto status = Core::verify(
        m_hpr->downloadDir + "/" + id, files,
        [this]() {
            return m_hpr->workThread.isReadyFinished();
        },
        [](const Transfers& transfers) {
            Transfers t = transfers;
            server_engine->postEvent(new ServerTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        server_engine->postEvent(new ServerEvent(ServerEvent::RES_VERIFY_DONE));
    } else if (status.state() == Core::FAILED) {
        postError(status.error());
    }
}

void WebQueue::distribute(const std::string& id, const Files& files)
{
    auto status = Core::distribute(
        m_hpr->distributeHandle,
        m_hpr->distributeUrl, m_hpr->distributePort, m_hpr->downloadDir + "/" + id, files, server_config,
        [this]() {
            return m_hpr->workThread.isReadyFinished();
        },
        [](const Transfers& transfers) {
            Transfers t = transfers;
            server_engine->postEvent(new ServerTransferEvent(Transfers(transfers)));
        });
    if (status.state() == Core::SUCCEED) {
        server_engine->postEvent(new ServerEvent(ServerEvent::RES_DISTRUBUTE_DONE));
    } else if (status.state() == Core::FAILED) {
        postError(status.error());
    }
}

DCUS_NAMESPACE_END
