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

#ifndef DCUS_SEMAPHORE_H
#define DCUS_SEMAPHORE_H

#include "dcus/base/define.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT Semaphore final {
public:
    explicit Semaphore(uint32_t count = 0) noexcept;
    uint32_t count() const noexcept;
    bool acquire(int32_t milli_s = -1) noexcept;
    bool acquire_nano(int64_t nano_s = -1) noexcept;
    bool release() noexcept;
    bool reset(uint32_t count = 0) noexcept;

private:
    std::atomic<int32_t> m_count { 0 };
    std::mutex m_mutex;
    std::condition_variable m_condition;
};

DCUS_NAMESPACE_END

#endif // DCUS_SEMAPHORE_H
