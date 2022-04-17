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

#ifndef DCUS_CORE_HELPER_H
#define DCUS_CORE_HELPER_H

#include "status.h"
#include <mutex>

DCUS_NAMESPACE_BEGIN

namespace Core {
struct StatusHelper {
    Status status;
    explicit StatusHelper(const BreakFunction& breakFunction = nullptr)
        : m_breakFunction(breakFunction)
    {
        status.state() = SUCCEED;
        status.error() = 0;
    }
    bool checkDone()
    {
        if (status.state() == FAILED) {
            return true;
        }
        if (m_breakFunction) {
            if (m_breakFunction()) {
                m_mutex.lock();
                if (status.state() == SUCCEED) {
                    status.state() = CANCELED;
                    status.error() = 0;
                }
                m_mutex.unlock();
                return true;
            }
        }
        return false;
    }
    void throwError(int error, bool critical = true)
    {
        m_mutex.lock();
        if (critical) {
            status.state() = FAILED;
        }
        status.error() = error;
        m_mutex.unlock();
    }

private:
    std::mutex m_mutex;
    BreakFunction m_breakFunction;
};
}

DCUS_NAMESPACE_END

#endif //DCUS_CORE_HELPER_H
