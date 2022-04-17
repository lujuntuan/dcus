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

#include "dcus/setting.h"
#if (defined(DCUS_BUILD_CLIENT) && defined(DCUS_USE_DOWNLOAD_HTTP)) || (defined(DCUS_BUILD_SERVER) && defined(DCUS_USE_PULL_HTTP))
#include "dcus/utils/dir.h"
#include "dcus/utils/string.h"
#include "dcus/utils/time.h"
#include "config_http.h"
#include "core.h"
#include "dcus/elapsed.h"
#include "dcus/log.h"
#include "helper.h"
#include "importlib/httplib.hpp"

DCUS_NAMESPACE_BEGIN

namespace Core {

#ifdef DCUS_USE_DOWNLOAD_HTTP

extern Status httpDownloadCommon(const std::string& dir, const Files& files, const Data& config,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction);

Status download(const std::string& dir, const Files& files, const Data& config,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction)
{
    return httpDownloadCommon(dir, files, config, breakFunction, progressFunction);
}

#endif

Status httpDownloadCommon(const std::string& dir, const Files& files, const Data& config,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction)
{
    StatusHelper statusHelper(breakFunction);
    if (files.empty()) {
        statusHelper.throwError(101);
        LOG_WARNING("download files is empty");
        return statusHelper.status;
    }
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    if (!Utils::exists(dir)) {
        Utils::mkPath(dir);
    }
    std::set<std::string> downloadUrls;
    Transfers transfers;
    std::mutex mutex;
    Elapsed transferElapsed;
    transferElapsed.start();
    for (const File& file : files) {
        if (downloadUrls.find(file.url()) == downloadUrls.end()) {
            downloadUrls.insert(file.url());
        } else {
            statusHelper.throwError(102);
            LOG_WARNING("download has repeat");
            return statusHelper.status;
        }
    }
    struct Helper {
        httplib::Headers headers;
        std::ofstream wfile;
        std::fstream wCachefile;
        std::string currentSizeStr;
        std::string fileName;
        Elapsed totalElapsed;
        Elapsed elapsed;
        uint64_t offsetSize = 0;
        uint64_t currentSize = 0;
        bool finished = false;
        bool exists = false;
    };
    std::vector<Helper*> helpers;
    httplib::ThreadPool threadPool(files.size() > CPPHTTPLIB_THREAD_POOL_COUNT ? CPPHTTPLIB_THREAD_POOL_COUNT : files.size());
    for (const auto& file : files) {
        threadPool.enqueue([&, file]() {
            if (statusHelper.checkDone()) {
                return;
            }
            const auto& clientUrlPair = Utils::getIpaddrMethod(file.url());
            if (clientUrlPair.first.empty()) {
                statusHelper.throwError(105);
                LOG_WARNING("download url error", " (" + file.name() + ")");
                return;
            }
            if (clientUrlPair.second.empty()) {
                statusHelper.throwError(106);
                LOG_WARNING("download url error", " (" + file.name() + ")");
                return;
            }
            httplib::Client client(clientUrlPair.first);
            if (!loadClientConfig(client, config)) {
                statusHelper.throwError(107);
                LOG_WARNING("load client config error", " (" + file.name() + ")");
                return;
            }
            Helper* helper = new Helper();
            helper->finished = false;
            helper->exists = false;
            mutex.lock();
            helpers.push_back(helper);
            mutex.unlock();
            if (!Utils::exists(dir + "/" + file.domain())) {
                Utils::mkPath(dir + "/" + file.domain());
            }
            helper->totalElapsed.restart();
            helper->elapsed.restart();
            helper->headers.clear();
            if (helper->wCachefile.is_open()) {
                helper->wCachefile.close();
            }
            helper->fileName = dir + "/" + file.domain() + "/" + file.name();
            if (Utils::exists(helper->fileName + ".cache")) {
                helper->wCachefile.open(helper->fileName + ".cache", std::ios::in);
                if (!helper->wCachefile.is_open()) {
                    statusHelper.throwError(108);
                    LOG_WARNING("download open error", " (" + helper->fileName + ")");
                    helper->finished = false;
                    return;
                }
                std::stringstream buffer;
                buffer << helper->wCachefile.rdbuf();
                helper->currentSizeStr = buffer.str();
                buffer >> helper->currentSize;
                const auto& range = httplib::make_range_header({ { helper->currentSize, file.size() } });
                helper->headers.insert(std::move(range));
                helper->offsetSize = helper->currentSize;
                helper->wCachefile.close();
                helper->wCachefile.open(helper->fileName + ".cache", std::ios::out | std::ios::trunc);
                if (!helper->wCachefile.is_open()) {
                    statusHelper.throwError(109);
                    LOG_WARNING("download open error", " (" + helper->fileName + ")");
                    helper->finished = false;
                    return;
                }
            } else {
                if (Utils::exists(helper->fileName)) {
                    if (helper->wfile.is_open()) {
                        helper->wfile.close();
                    }
                    helper->wCachefile.open(helper->fileName, std::ios::in | std::ios::binary);
                    if (!helper->wCachefile.is_open()) {
                        statusHelper.throwError(110);
                        LOG_WARNING("download open error", " (" + helper->fileName + ")");
                        helper->finished = false;
                        return;
                    }
                    helper->wCachefile.seekg(0, helper->wCachefile.end);
                    size_t checkfileSize = helper->wCachefile.tellg();
                    helper->wCachefile.close();
                    if (checkfileSize == file.size()) {
                        helper->finished = true;
                        helper->currentSize = file.size();
                        const auto& range = httplib::make_range_header({ { helper->currentSize, file.size() } });
                        helper->headers.insert(std::move(range));
                        helper->offsetSize = helper->currentSize;
                        helper->exists = true;
                        mutex.lock();
                        downloadUrls.erase(file.url());
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
                        // return;
                    } else {
                        Utils::remove(helper->fileName);
                    }
                }
                helper->wCachefile.open(helper->fileName + ".cache", std::ios::out | std::ios::trunc);
                if (!helper->wCachefile.is_open()) {
                    statusHelper.throwError(111);
                    LOG_WARNING("download open error", " (" + helper->fileName + ")");
                    helper->finished = false;
                    return;
                }
                helper->currentSize = 0;
                helper->currentSizeStr.clear();
                helper->currentSizeStr.shrink_to_fit();
                helper->offsetSize = 0;
            }
            if (helper->wfile.is_open()) {
                helper->wfile.close();
            }
            helper->wfile.open(helper->fileName, std::ios::out | std::ios::app | std::ios::binary);
            if (!helper->wfile.is_open()) {
                statusHelper.throwError(112);
                LOG_WARNING("download open error", " (" + helper->fileName + ")");
                helper->finished = false;
                return;
            }
            auto res = client.Get(
                clientUrlPair.second.c_str(), helper->headers,
                [&](const char* data, size_t data_length) {
                    if (statusHelper.checkDone()) {
                        helper->finished = false;
                        return false;
                    }
                    if (data_length <= 0) {
                        return true;
                    }
                    helper->wfile.write(data, data_length);
                    helper->currentSize += data_length;
                    helper->currentSizeStr = std::to_string(helper->currentSize);
                    helper->wCachefile.seekp(0, std::ios::beg);
                    helper->wCachefile.clear();
                    helper->wCachefile.write(helper->currentSizeStr.data(), helper->currentSizeStr.length());
                    helper->wfile.flush();
                    helper->wCachefile.flush();
#if (DCUS_WEB_TRANSFER_TEST_TIME)
                    Utils::sleepMilli(DCUS_WEB_TRANSFER_TEST_TIME); // sleep_test
#endif
                    return true;
                },
                [&](uint64_t current, uint64_t total) {
                    if (statusHelper.checkDone()) {
                        helper->finished = false;
                        return false;
                    }
                    if (current >= total) { // wanning
                        helper->finished = true;
                    } else {
                        helper->finished = false;
                    }
                    if (statusHelper.checkDone()) {
                        helper->finished = false;
                        return false;
                    }
                    if (progressFunction) {
                        if (helper->elapsed.get() >= DCUS_WEB_TRANSFER_INTERVAL_MIN || current >= total) {
                            if (total + helper->offsetSize != file.size()) {
                                statusHelper.throwError(113);
                                LOG_WARNING("download size error", " (" + helper->fileName + ")");
                                helper->finished = false;
                                return false;
                            }
                            helper->elapsed.restart();
                            uint32_t pass = (uint32_t)(helper->totalElapsed.get() / 1000);
                            uint32_t speed = pass <= 0 ? (uint32_t)(file.size() / 1024) : (uint32_t)(current / 1024 / pass);
                            if (speed > (uint32_t)(file.size() / 1024)) {
                                speed = (uint32_t)(file.size() / 1024);
                            }
                            uint32_t left = speed <= 0 ? 0 : (uint32_t)((total - current) / 1024 / speed);
                            Transfer transfer;
                            transfer.domain() = file.domain();
                            transfer.name() = file.name();
                            transfer.progress() = file.size() <= 0 ? 100.0f : (current + helper->offsetSize) * 100.0f / file.size();
                            transfer.total() = (uint32_t)(file.size() / 1024);
                            transfer.current() = (uint32_t)((current + helper->offsetSize) / 1024);
                            transfer.speed() = speed;
                            transfer.pass() = pass;
                            transfer.left() = left;
                            mutex.lock();
                            transfers.update(std::move(transfer), true);
                            mutex.unlock();
                            if (transferElapsed.get() > DCUS_WEB_TRANSFER_INTERVAL && current < total) {
                                mutex.lock();
                                transferElapsed.restart();
                                transfers.sort();
                                progressFunction(transfers);
                                mutex.unlock();
                            }
                        }
                    }
                    return true;
                });
            if (!res) {
                if (res.error() != httplib::Error::Canceled && statusHelper.status.state() != CANCELED) {
                    if (Utils::exists(helper->fileName + ".cache")) {
                        if (helper->wCachefile.is_open()) {
                            helper->wCachefile.close();
                        }
                        Utils::remove(helper->fileName + ".cache");
                    }
                    if (!helper->exists) {
                        statusHelper.throwError(114);
                        LOG_WARNING("download core error, code=", res.error(), " (" + helper->fileName + ")");
                        helper->finished = false;
                    }
                }
            } else {
                if (Utils::exists(helper->fileName + ".cache")) {
                    if (helper->wCachefile.is_open()) {
                        helper->wCachefile.close();
                    }
                    Utils::remove(helper->fileName + ".cache");
                }
                if (helper->finished) {
                    mutex.lock();
                    downloadUrls.erase(file.url());
                    mutex.unlock();
                    if (!transfers.empty()) {
                        if (progressFunction) {
                            progressFunction(transfers);
                        }
                    }
                } else {
                    if (statusHelper.checkDone()) {
                        return;
                    }
                    statusHelper.throwError(115);
                    LOG_WARNING("not finished", " (" + helper->fileName + ")");
                }
            }
            if (helper->wfile.is_open()) {
                helper->wfile.close();
            }
            if (helper->wCachefile.is_open()) {
                helper->wCachefile.close();
            }
        });
    }
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    threadPool.shutdown();
    for (Helper* helper : helpers) {
        if (helper->wfile.is_open()) {
            helper->wfile.close();
        }
        if (helper->wCachefile.is_open()) {
            helper->wCachefile.close();
        }
        delete helper;
        helper = nullptr;
    }
    helpers.clear();
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    if (!downloadUrls.empty()) {
        statusHelper.throwError(119);
        LOG_WARNING("download final error");
    }
    return statusHelper.status;
}
}

DCUS_NAMESPACE_END

#endif
