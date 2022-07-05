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

#include "dcus/base/timer.h"
#include "dcus/base/log.h"
#include "dcus/base/queue.h"
#include "dcus/utils/time.h"

DCUS_NAMESPACE_BEGIN

Timer::Timer(const TimerPrivate&, uint32_t interval_milli_s, bool loop, const Function& function) noexcept
    : m_interval(interval_milli_s)
    , m_loop(loop)
    , m_function(function)
{
}

Timer::~Timer() noexcept
{
    m_function = nullptr;
    stop();
    if (m_queue) {
        m_queue->removeTimer(this);
    }
}

std::shared_ptr<Timer> Timer::create(Queue* queue, uint32_t interval_milli_s, bool loop, const Function& function) noexcept
{
    if (!queue) {
        LOG_CRITICAL("queue is null");
    }
    const std::shared_ptr<Timer>& timer = std::make_shared<Timer>(TimerPrivate { 0 }, interval_milli_s, loop, function);
    queue->addTimer(timer);
    return timer;
}

void Timer::once(Queue* queue, uint32_t interval_milli_s, const Function& function) noexcept
{
    if (!queue) {
        LOG_CRITICAL("queue is null");
    }
    queue->onceTimer(interval_milli_s, function);
}

void Timer::start(const Function& function) noexcept
{
    if (function) {
        m_function = function;
    }
    m_startTimer = Utils::getNanoCurrent();
#if TIMER_USE_PRECISION
    m_loopCount = 0;
#endif
}

void Timer::stop() noexcept
{
    m_startTimer = 0;
#if TIMER_USE_PRECISION
    m_loopCount = 0;
#endif
}

bool Timer::active() const noexcept
{
    return m_startTimer > 0;
}

uint32_t Timer::interval() const noexcept
{
    return m_interval;
}

bool Timer::loop() const noexcept
{
    return m_loop;
}

Timer::Function Timer::function() const noexcept
{
    return m_function;
}

void Timer::setInterval(uint32_t interval_milli_s) noexcept
{
    m_interval = interval_milli_s;
}

void Timer::setLoop(bool loop) noexcept
{
    m_loop = loop;
}

void Timer::setFunction(const Function& function) noexcept
{
    m_function = function;
}

DCUS_NAMESPACE_END
