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
#if (defined(DCUS_BUILD_WORKER) && defined(DCUS_USE_PATCH_BSDIFF))
#include "commonlib/dir.h"
#include "commonlib/dispatch.h"
#include "commonlib/verify.h"
#include "core.h"
#include "dcus/base/log.h"
#include "helper.h"
#include "importlib/httplib.hpp"

DCUS_NAMESPACE_BEGIN
namespace Core {
Status patch(const std::string& dir, const std::vector<std::string>& paths, Files& newFiles,
    const BreakFunction& breakFunction,
    const ProgressFunction& progressFunction)
{
    DCUS_UNUSED(progressFunction);
    Files targetNewFiles;
    StatusHelper statusHelper;
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    httplib::ThreadPool threadPool(paths.size() > CPPHTTPLIB_THREAD_POOL_COUNT ? CPPHTTPLIB_THREAD_POOL_COUNT : paths.size());
    for (const auto& path : paths) {
        threadPool.enqueue([&]() {
            const std::string& infoPath = path + ".info";
            const std::string& newPath = path.substr(0, path.length() - 6);
            if (!Common::exists(path)) {
                statusHelper.throwError(701);
                return;
            }
            if (Common::getPathSuffixName(path) != "patch") {
                statusHelper.throwError(702);
                return;
            }
            if (!Common::exists(infoPath)) {
                statusHelper.throwError(703);
                return;
            }
            const Data& info = Data::read(infoPath);
            const std::string& oldName = info.value("basename").toString();
            if (oldName.empty()) {
                statusHelper.throwError(704);
                return;
            }
            const std::string& sha256 = info.value("sha256").toString();
            const std::string& sha1 = info.value("sha1").toString();
            const std::string& md5 = info.value("md5").toString();
            std::string oldPath;
            for (const auto& p : Common::getAllFiles(dir)) {
                if (Common::getPathFileName(p) == oldName) {
                    oldPath = p;
                    break;
                }
            }
            if (oldPath.empty()) {
                statusHelper.throwError(705);
                return;
            }
            bool dispathOk = Common::dispatch(oldPath, newPath, path, breakFunction);
            if (statusHelper.checkDone()) {
                return;
            }
            if (!dispathOk) {
                statusHelper.throwError(706);
                return;
            }
            File file;
            file.name() = Common::getPathFileName(path);
            file.sha256() = sha256;
            file.sha1() = sha1;
            file.md5() = md5;
            targetNewFiles.push_back(file);
        });
    }
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    threadPool.shutdown();
    if (statusHelper.checkDone()) {
        return statusHelper.status;
    }
    newFiles = targetNewFiles;
    return statusHelper.status;
}

}
DCUS_NAMESPACE_END

#endif
