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
#include "dcus/utils/time.h"

DCUS_NAMESPACE_BEGIN

Timer::Timer(uint32_t interval_milli_s, bool loop, const Function& function) noexcept
    : m_interval(interval_milli_s)
    , m_loop(loop)
    , m_function(function)
{
}

Timer::~Timer() noexcept
{
    m_function = nullptr;
    stop();
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
