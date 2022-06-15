/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2022
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef DCUS_SINGLETON_H
#define DCUS_SINGLETON_H

#include "dcus/base/define.h"
#include "dcus/base/log.h"

DCUS_NAMESPACE_BEGIN
template <typename T>
class SingletonProxy {
public:
    static T* getInstance(bool showWarn = true)
    {
        if (showWarn && !m_instance) {
            LOG_WARNING("instance is null");
        }
        return m_instance;
    }

protected:
    static void setInstance(T* instance)
    {
        m_instance = instance;
    }

private:
    static T* m_instance;
};

template <typename T>
T* SingletonProxy<T>::m_instance = nullptr;

DCUS_NAMESPACE_END

#endif // DCUS_SINGLETON_H
