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

#ifndef FIBOIVI_INTERFACE_H
#define FIBOIVI_INTERFACE_H

#include "fiboivi/base/define.h"
#include "fiboivi/base/log.h"
#include <memory>

FIBOIVI_NAMESPACE_BEGIN

class InterfaceBase {
    CLASS_DISSABLE_COPY_AND_ASSIGN(InterfaceBase)
public:
    InterfaceBase() = default;
    virtual ~InterfaceBase() = default;
};

template <class T>
class InterfaceProxy {
public:
    InterfaceProxy<T>() = default;
    ~InterfaceProxy<T>() = default;
    inline std::shared_ptr<T> interface(bool showWarn = true) const
    {
        if (showWarn && !m_interface) {
            LOG_WARNING("instance is null");
        }
        return m_interface;
    }
    template <class TargetClass>
    void loadInterface()
    {
        if (m_interface) {
            LOG_WARNING("instance has set");
        }
        m_interface = std::make_shared<TargetClass>();
    }

private:
    std::shared_ptr<T> m_interface = nullptr;
};

FIBOIVI_NAMESPACE_END

#endif // FIBOIVI_INTERFACE_H
