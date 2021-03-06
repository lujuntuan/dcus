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

#ifndef DCUS_TIMER_H
#define DCUS_TIMER_H

#include "dcus/base/define.h"
#include <functional>
#include <memory>
#include <stdint.h>

DCUS_NAMESPACE_BEGIN

#define TIMER_USE_PRECISION 1
#define TIMER_ERROR_OFFSET 1000000U

class Queue;

class DCUS_EXPORT Timer final {
    CLASS_DISSABLE_COPY_AND_ASSIGN(Timer)

private:
    struct TimerPrivate {
        explicit TimerPrivate(int) { }
    };

public:
    using Function = std::function<void()>;
    explicit Timer(const TimerPrivate&, uint32_t interval_milli_s, bool loop = false, const Function& function = nullptr) noexcept; // 0milli_s==100000ns
    ~Timer() noexcept;

public:
    static std::shared_ptr<Timer> create(Queue* queue, uint32_t interval_milli_s, bool loop = false, const Function& function = nullptr) noexcept;
    static void once(Queue* queue, uint32_t interval_milli_s, const Function& function = nullptr) noexcept;
    void start(const Function& function = nullptr) noexcept;
    void stop() noexcept;
    bool active() const noexcept;
    uint32_t interval() const noexcept;
    bool loop() const noexcept;
    Function function() const noexcept;
    void setInterval(uint32_t interval_milli_s) noexcept;
    void setLoop(bool loop) noexcept;
    void setFunction(const Function& function) noexcept;

private:
    friend Queue;
    uint32_t m_interval = 0;
    uint64_t m_startTimer = 0;
#if TIMER_USE_PRECISION
    uint64_t m_loopCount = 0;
#endif
    bool m_loop = false;
    Function m_function = nullptr;
    Queue* m_queue = nullptr;
};

DCUS_NAMESPACE_END

#endif // DCUS_TIMER_H
