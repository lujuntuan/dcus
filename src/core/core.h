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

#ifndef DCUS_CORE_H
#define DCUS_CORE_H

#include "dcus/config.h"
#include "dcus/transfer.h"
#include "dcus/upgrade.h"
#include "status.h"
#include <atomic>

DCUS_NAMESPACE_BEGIN

namespace Core {
using ProgressFunction = std::function<void(const Transfers&)>;
extern Status download(const std::string& dir, const Files& files, const VariantMap& config,
    const BreakFunction& breakFunction = nullptr,
    const ProgressFunction& progressFunction = nullptr);
extern Status verify(const std::string& dir, const Files& files,
    const BreakFunction& breakFunction = nullptr,
    const ProgressFunction& progressFunction = nullptr);

#ifdef DCUS_BUILD_CLIENT
extern Status patch(const std::string& dir, const std::vector<std::string>& paths, Files& newFiles,
    const BreakFunction& breakFunction = nullptr,
    const ProgressFunction& progressFunction = nullptr);
#endif

#ifdef DCUS_BUILD_SERVER
using DistributeHandle = std::atomic<void*>;
using Token = std::pair<std::string, std::string>;
extern Status pull(const std::string& dir, const Files& files, const VariantMap& config,
    const BreakFunction& breakFunction = nullptr,
    const ProgressFunction& progressFunction = nullptr);
extern Status distribute(DistributeHandle& handle, const std::string& url, int port, const std::string& dir, const Files& files, const VariantMap& config,
    const BreakFunction& breakFunction = nullptr,
    const ProgressFunction& progressFunction = nullptr);
extern void stopDistribute(const DistributeHandle& handle);
extern Status getMessage(const std::string& url, const std::string& path, std::string& body, const VariantMap& config,
    const Token& token = { "", "" });
extern Status putMessage(const std::string& url, const std::string& path, const std::string& body, const VariantMap& config,
    const Token& token = { "", "" });
extern Status postMessage(const std::string& url, const std::string& path, const std::string& body, const VariantMap& config,
    const Token& token = { "", "" });

#endif
}

DCUS_NAMESPACE_END

#endif // DCUS_CORE_H
