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

#ifndef DCUS_PLUGIN_H
#define DCUS_PLUGIN_H

#include "dcus/define.h"
#include <memory>
#include <string>

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT Plugin {
public:
    using Handle = void*;
    template <class T>
    static std::shared_ptr<T> create(const std::string& libName, const std::string& versionName = "", const std::string& functionName = "dcus_create_handle") noexcept
    {
        Handle hanlde = create(libName, typeid(T).name(), versionName, functionName);
        return std::shared_ptr<T>(static_cast<T>(hanlde));
    }

private:
    static Handle create(const std::string& libName, const std::string& className, const std::string& versionName, const std::string& functionName) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_PLUGIN_H
