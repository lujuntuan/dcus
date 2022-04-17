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

#include "dcus/plugin.h"
#include "dcus/log.h"
#include "dcus/utils/dir.h"
#include "dcus/utils/host.h"
#include "importlib/dynalo.hpp"

DCUS_NAMESPACE_BEGIN

Plugin::Handle Plugin::create(const std::string& libName, const std::string& className, const std::string& versionName, const std::string& functionName) noexcept
{
    Handle handle = nullptr;
    std::string pluginName = dynalo::to_native_name(libName);
    auto getPluginPath = [&]() -> std::string {
        std::string checkPath = Utils::getEnvironment("DCUS_PLUGINS_PATH") + "/" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        checkPath = "./" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        checkPath = "../" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        checkPath = "../lib" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        checkPath = "../lib64" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        checkPath = "/usr/lib/" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        checkPath = "/usr/lib64/" + pluginName;
        if (Utils::exists(checkPath)) {
            return checkPath;
        }
        return std::string();
    };
    const std::string& pluginPath = getPluginPath();
    if (getPluginPath().empty()) {
        LOG_WARNING("load plugin path is empty");
        return handle;
    }
    LOG_DEBUG("loading plugin:", pluginPath);
    try {
        dynalo::library lib(pluginPath);
        try {
            auto createFunction = lib.get_function<Handle(const char*, const char*)>(functionName);
            if (!createFunction) {
                LOG_WARNING("failed to load plugin(function is null)");
            }
            handle = createFunction(className.data(), versionName.data());
            if (!handle) {
                LOG_WARNING("failed to load plugin(handle is null)");
            }
            LOG_DEBUG("loaded successfully");
            return handle;
        } catch (...) {
            LOG_WARNING("failed to load plugin(function throw error)");
            return nullptr;
        }
    } catch (...) {
        LOG_WARNING("failed to load plugin(create throw error)");
        return nullptr;
    }
    return nullptr;
}

DCUS_NAMESPACE_END
