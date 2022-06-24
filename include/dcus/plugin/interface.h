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

#ifndef DCUS_INTERFACE_H
#define DCUS_INTERFACE_H

#include "dcus/base/define.h"
#include "dcus/base/log.h"
#include <memory>

DCUS_NAMESPACE_BEGIN

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
    inline std::unique_ptr<T> interface(bool showWarn = true) const
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
        m_interface = std::make_unique<TargetClass>();
    }

private:
    std::unique_ptr<T> m_interface = nullptr;
};

DCUS_NAMESPACE_END

#endif // DCUS_INTERFACE_H
