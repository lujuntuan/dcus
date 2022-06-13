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

#include "dcus/base/setting.h"
#if (defined(DCUS_BUILD_SERVER) && defined(DCUS_USE_DISTRIBUTE_HTTP))
#include "config_http.h"
#include "core.h"
#include "dcus/base/elapsed.h"
#include "dcus/base/log.h"
#include "dcus/utils/dir.h"
#include "dcus/utils/string.h"
#include "dcus/utils/time.h"
#include "helper.h"
#include "importlib/httplib.hpp"

DCUS_NAMESPACE_BEGIN

namespace Core {
Status distribute(DistributeHandle& handle, const std::string& url, int port, const std::string& dir, const Files& files, const Data& config,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction)
{
    StatusHelper statusHelper(breakFunction);
    if (files.empty()) {
        statusHelper.throwError(301);
        LOG_WARNING("distribute id is empty");
        return statusHelper.status;
    }
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    std::set<std::string> distributeUrls;
    Transfers transfers;
    std::mutex mutex;
    Elapsed transferElapsed;
    transferElapsed.start();
    for (const File& file : files) {
        if (distributeUrls.find(file.url()) == distributeUrls.end()) {
            distributeUrls.insert(file.url());
        } else {
            statusHelper.throwError(302);
            LOG_WARNING("distribute has repeat");
            return statusHelper.status;
        }
    }
    struct Helper {
        std::ifstream rfile;
        Elapsed totalElapsed;
        Elapsed elapsed;
        std::string fileName;
        std::vector<char> buffer;
        uint64_t beginPos = 0;
        bool hasFileflag = false;
        bool finished = false;
    };
    std::vector<Helper*> helpers;
#ifdef DCUS_USE_HTTPS
    if (!config.value("web_server_cert_path").valid() || !config.value("web_server_key_path").valid()) {
        statusHelper.throwError(304);
        LOG_WARNING("distribute web_server_cert_path or web_server_key_path  is empty");
        return statusHelper.status;
    }
    std::string certPath = config.value("web_server_cert_path").toString();
    std::string keyPath = config.value("web_server_key_path").toString();
    httplib::SSLServer server(certPath.c_str(), keyPath.c_str());
#else
    httplib::Server server;
#endif
    server.new_task_queue = [&files] {
        return new httplib::ThreadPool(files.size() > CPPHTTPLIB_THREAD_POOL_COUNT ? CPPHTTPLIB_THREAD_POOL_COUNT : files.size());
    };
    auto feedToServer = [&statusHelper](httplib::Response& res) {
        res.status = statusHelper.status.error();
        res.set_content(Utils::stringSprintf("<h1>Error %s</h1>", std::to_string(res.status)), "text/html");
    };
    if (!loadServerConfig(server, config)) {
        statusHelper.throwError(305);
        LOG_WARNING("load server config  error1");
        return statusHelper.status;
    }
    std::string serverFuncName = std::string("/") + DCUS_WEB_PULL_FUNC_NAME + "/[^\\s]*";
    server.Get(serverFuncName, [&](const httplib::Request& req, httplib::Response& res) {
        bool hasPull = false;
        const auto& spVector = Utils::stringSplit(req.path, "/");
        if (spVector.size() != 4) {
            statusHelper.throwError(306, false);
            LOG_WARNING("distribute url error");
            feedToServer(res);
            return;
        }
        if (!spVector[0].empty()) {
            statusHelper.throwError(307, false);
            LOG_WARNING("distribute url error");
            feedToServer(res);
            return;
        }
        if (spVector[1] != DCUS_WEB_PULL_FUNC_NAME) {
            statusHelper.throwError(308, false);
            LOG_WARNING("distribute url error");
            feedToServer(res);
            return;
        }
        if (spVector[2].empty() || spVector[3].empty()) {
            statusHelper.throwError(309, false);
            LOG_WARNING("distribute url error");
            feedToServer(res);
            return;
        }
        for (const auto& file : files) {
            if (statusHelper.checkDone()) {
                return;
            }
            if (file.domain() != spVector[2] || file.name() != spVector[3]) {
                continue;
            }
            Helper* helper = new Helper;
            helper->buffer.resize(CPPHTTPLIB_RECV_BUFSIZ);
            mutex.lock();
            helper->finished = false;
            helper->hasFileflag = false;
            helpers.push_back(helper);
            helper->hasFileflag = std::any_of(distributeUrls.begin(), distributeUrls.end(), [&](const std::string& one) { return one == file.url(); });
            mutex.unlock();
            if (!helper->hasFileflag) {
                // statusHelper.throwError(310);
                LOG_WARNING("distribute can not find file", " (" + file.name() + ")");
                // feedToServer(res);
                return;
            }
            //
            helper->fileName = dir + "/" + file.domain() + "/" + file.name();
            helper->beginPos = 0;
            hasPull = true;
            helper->totalElapsed.restart();
            helper->elapsed.restart();
            res.set_header("content-disposition", "attachment;filename=" + file.name());
            res.set_header("Accept-Ranges", "bytes");
            res.set_header("Content-Length", std::to_string(file.size()));
            if (req.has_header("Range")) {
                std::string range = req.get_header_value("Range"); // bytes=xxx
                res.status = 206;
                range.erase(0, 6);
                const auto& vr = Utils::stringSplit(std::move(range), "-");
                if (vr.size() >= 1) {
                    helper->beginPos = Utils::stringToInt64(vr[0]);
                }
            }
            if (helper->beginPos > file.size()) {
                statusHelper.throwError(311);
                LOG_WARNING("distribute begin pos too big", " (" + helper->fileName + ")");
                feedToServer(res);
                return;
            } else if (helper->beginPos == file.size()) {
                res.status = 200;
                res.set_content("", "");
                mutex.lock();
                distributeUrls.erase(file.url());
                if (distributeUrls.empty()) {
                    server.stop();
                }
                mutex.unlock();
                if (progressFunction) {
                    Transfer transfer;
                    transfer.domain() = file.domain();
                    transfer.name() = file.name();
                    transfer.progress() = 100.0f;
                    transfer.total() = (uint32_t)(file.size() / 1024);
                    transfer.current() = (uint32_t)(file.size() / 1024);
                    transfer.speed() = 0;
                    transfer.pass() = 0;
                    transfer.left() = 0;
                    mutex.lock();
                    transfers.update(std::move(transfer), true);
                    transfers.sort();
                    mutex.unlock();
                    progressFunction(transfers);
                }
                return;
            } else if (helper->beginPos < 0) {
                statusHelper.throwError(312);
                LOG_WARNING("distribute begin pos too small", " (" + helper->fileName + ")");
                feedToServer(res);
                return;
            }
            res.set_content_provider(
                file.size(),
                "application/x-download",
                [&, helper, file](size_t offset, size_t length, httplib::DataSink& sink) {
                    if (statusHelper.checkDone()) {
                        helper->finished = false;
                        return false;
                    }
                    DCUS_UNUSED(length);
                    if (!helper->rfile.is_open()) {
                        helper->rfile.open(helper->fileName, std::ios::in | std::ios::binary);
                        helper->rfile.seekg(helper->beginPos, std::ios::beg);
                        if (!helper->rfile.is_open()) {
                            statusHelper.throwError(313);
                            LOG_WARNING("distribute open error", " (" + helper->fileName + ")");
                            feedToServer(res);
                            helper->finished = false;
                            return false;
                        }
                    }
                    if (!sink.is_writable()) {
                        Utils::sleepMilli(10);
                        if (!sink.is_writable()) {
                            statusHelper.throwError(314, false);
                            LOG_WARNING("distribute break done", " (" + helper->fileName + ")");
                            feedToServer(res);
                            helper->finished = false;
                            return false;
                        }
                    }
                    if (helper->rfile.good()) {
                        if (statusHelper.checkDone()) {
                            helper->finished = false;
                            return false;
                        }
                        size_t size = std::min(file.size() - offset, helper->buffer.size());
                        helper->rfile.read(helper->buffer.data(), size);
                        sink.write(helper->buffer.data(), size);
                        uint64_t current = offset + size;
#if (DCUS_WEB_TRANSFER_TEST_TIME)
                        Utils::sleepMilli(DCUS_WEB_TRANSFER_TEST_TIME); // sleep_test
#endif
                        if (helper->elapsed.get() >= DCUS_WEB_TRANSFER_INTERVAL_MIN || current >= file.size()) {
                            if (current >= file.size()) {
                                helper->finished = true;
                            } else {
                                helper->finished = false;
                            }
                            if (progressFunction) {
                                helper->elapsed.restart();
                                uint32_t pass = (uint32_t)(helper->totalElapsed.get() / 1000);
                                uint32_t speed = pass <= 0 ? (uint32_t)(file.size() / 1024) : (uint32_t)((current - helper->beginPos) / 1024 / pass);
                                if (speed > (uint32_t)(file.size() / 1024)) {
                                    speed = (uint32_t)(file.size() / 1024);
                                }
                                uint32_t left = speed <= 0 ? 0 : (uint32_t)((file.size() - current) / 1024 / speed);
                                Transfer transfer;
                                transfer.domain() = file.domain();
                                transfer.name() = file.name();
                                transfer.progress() = file.size() <= 0 ? 100.0f : current * 100.0f / file.size();
                                transfer.total() = (uint32_t)(file.size() / 1024);
                                transfer.current() = (uint32_t)(current / 1024);
                                transfer.speed() = speed;
                                transfer.pass() = pass;
                                transfer.left() = left;
                                mutex.lock();
                                transfers.update(std::move(transfer), true);
                                mutex.unlock();
                                if (transferElapsed.get() > DCUS_WEB_TRANSFER_INTERVAL && current < file.size()) {
                                    mutex.lock();
                                    transferElapsed.restart();
                                    transfers.sort();
                                    progressFunction(transfers);
                                    mutex.unlock();
                                }
                            }
                        }
                    } else {
                        if (sink.done) {
                            sink.done();
                        } else {
                            if (statusHelper.checkDone()) {
                                return false;
                            }
                            statusHelper.throwError(315, false);
                            LOG_WARNING("distribute break done", " (" + helper->fileName + ")");
                            feedToServer(res);
                            helper->finished = false;
                            return false;
                        }
                    }
                    return true;
                },
                [&, helper, file](bool success) {
                    if (helper->rfile.is_open()) {
                        helper->rfile.close();
                    }
                    if (success) {
                        helper->finished = true;
                    }
                    if (helper->finished) {
                        mutex.lock();
                        distributeUrls.erase(file.url());
                        if (distributeUrls.empty()) {
                            server.stop();
                        }
                        mutex.unlock();
                        if (progressFunction) {
                            if (!transfers.empty()) {
                                progressFunction(transfers);
                            }
                        }
                    } else {
                        statusHelper.throwError(316, false);
                        LOG_WARNING("distribute not finished", " (" + helper->fileName + ")");
                    }
                });
        }
        if (!hasPull && !statusHelper.checkDone()) {
            statusHelper.throwError(317, false);
            LOG_WARNING("distribute url can not find");
            feedToServer(res);
            return;
        }
    });
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    handle = &server;
    server.listen(url.c_str(), port);

    for (Helper* helper : helpers) {
        if (helper->rfile.is_open()) {
            helper->rfile.close();
        }
        delete helper;
        helper = nullptr;
    }
    helpers.clear();
    helpers.shrink_to_fit();
    if (statusHelper.checkDone()) {
        handle = nullptr;
        return statusHelper.status;
    }
    if (!distributeUrls.empty()) {
        statusHelper.throwError(318);
        LOG_WARNING("distribute final error");
    }
    handle = nullptr;
    return statusHelper.status;
}

void stopDistribute(const DistributeHandle& handle)
{
    if (handle) {
        void* ptr = handle;
        httplib::Server* server = (httplib::Server*)ptr;
        server->stop();
    }
}
}

DCUS_NAMESPACE_END

#endif
