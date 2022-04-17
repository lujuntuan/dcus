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

#ifndef DCUS_SINGLETON_H
#define DCUS_SINGLETON_H

#include "dcus/define.h"

DCUS_NAMESPACE_BEGIN
template <typename T>
class Singleton {
public:
    static T* getInstance()
    {
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
T* Singleton<T>::m_instance = nullptr;

DCUS_NAMESPACE_END

#endif // DCUS_SINGLETON_H
