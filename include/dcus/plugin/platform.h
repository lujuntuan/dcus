/*********************************************************************************
 *Copyright @ Fibocom Technologies Co., Ltd. 2019-2030. All rights reserved.
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef FIBOIVI_PLATFORM_H
#define FIBOIVI_PLATFORM_H

#include "fiboivi/base/define.h"
#include "fiboivi/base/log.h"
#include "fiboivi/base/plugin.h"
#include <stdlib.h>

FIBOIVI_NAMESPACE_BEGIN

#define FIBOIVI_PLATFORM_FUNCTION_NAME create_fiboivi_handle
#define FIBOIVI_PLATFORM_VERSION_MAJOR 1
#define FIBOIVI_PLATFORM_VERSION_MINOR 0

#define FIBOIVI_PLATFORM_ENV_NAME "FIBOIVI_PLATFORM_PATH"
#define FIBOIVI_PLATFORM_DIR_NAME "fiboivi_platforms"
#define FIBOIVI_PLATFORM_PREFIX_NAME "fiboivi_"
#define FIBOIVI_PLATFORM_MID_NAME "_platform_"

class PlatformBase {
    CLASS_DISSABLE_COPY_AND_ASSIGN(PlatformBase)
public:
    PlatformBase() = default;
    virtual ~PlatformBase() = default;
    template <class RealClass, int RealVersionMajor = FIBOIVI_PLATFORM_VERSION_MAJOR, int RealVersionMinor = FIBOIVI_PLATFORM_VERSION_MINOR>
    static Plugin::Handle createPlatform(const char* pluginId, uint16_t versionMajor, uint16_t versionMinor)
    {
        const std::string& RealPluginId = RealClass::GetPluginId();
        std::cout << "platform id: " << RealPluginId << std::endl;
        std::cout << "platform version: " << RealVersionMajor << "." << RealVersionMinor << std::endl;
        if (RealPluginId.empty() || RealPluginId != pluginId) {
            std::cerr << "compare platform ID error: " << pluginId << std::endl;
            return nullptr;
        }
        if (RealVersionMajor != versionMajor || RealVersionMinor < versionMinor) {
            std::cerr << "compare platform version error: " << versionMajor << "." << versionMinor << std::endl;
            return nullptr;
        }
        return new RealClass;
    }
};

template <class T>
class PlatformProxy {
public:
    PlatformProxy<T>() = default;
    ~PlatformProxy<T>()
    {
        m_platform = nullptr;
        m_plugin.clear();
    }
    inline std::shared_ptr<T> platform(bool showWarn = true) const
    {
        if (showWarn && !m_platform) {
            LOG_WARNING("instance is null");
        }
        return m_platform;
    }
    void loadPlatform(const std::string& libName, const std::string& platformName, uint16_t versionMajor = FIBOIVI_PLATFORM_VERSION_MAJOR, uint16_t versionMinor = FIBOIVI_PLATFORM_VERSION_MINOR)
    {
        if (m_platform) {
            LOG_WARNING("instance has set");
        }
        auto seartPath = m_plugin.defaultSearchPath();
        const char* path = getenv(FIBOIVI_PLATFORM_ENV_NAME);
        if (path) {
            seartPath.push_front(path);
        }
        m_platform = m_plugin.load<T>(
            FIBOIVI_PLATFORM_PREFIX_NAME + libName + FIBOIVI_PLATFORM_MID_NAME + platformName,
            std::move(seartPath),
            FIBOIVI_PLATFORM_DIR_NAME,
            FIBOIVI_GETNAME(FIBOIVI_PLATFORM_FUNCTION_NAME),
            versionMajor,
            versionMinor);
    }

private:
    Plugin m_plugin;
    std::shared_ptr<T> m_platform = nullptr;
};

#define FIBOIVI_CREATE_PLATFORM(...)                                                                                  \
    C_INTERFACE_BEGIN                                                                                                 \
    Plugin::Handle FIBOIVI_PLATFORM_FUNCTION_NAME(const char* pluginId, uint16_t versionMajor, uint16_t versionMinor) \
    {                                                                                                                 \
        return PlatformBase::createPlatform<__VA_ARGS__>(pluginId, versionMajor, versionMinor);                       \
    }                                                                                                                 \
    C_INTERFACE_END

FIBOIVI_NAMESPACE_END

#endif // FIBOIVI_PLATFORM_H
