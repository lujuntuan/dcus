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

#include "dcus/queue.h"
#include "dcus/utils/time.h"
#include "dcus/log.h"
#include "dcus/semaphore.h"
#include "dcus/thread.h"
#include <iostream>
#include <list>
#include <queue>
#include <thread>

#define QUEUE_MAX_NUM 4096
#define m_hpr m_queueHelper

DCUS_NAMESPACE_BEGIN

struct TimeMethod {
    int64_t miniTime = 0;
    int64_t remainTime = 0;
    int64_t intervalTime = 0;
    uint64_t currentTime = 0;
#if TIMER_USE_PRECISION
    uint64_t expactCount = 0;
#endif
};

struct QueueHelper {
    int queueId = -1;
    std::atomic_bool isRunning { false };
    std::atomic_bool quitFlag { false };
    std::atomic_bool isBusy { false };
    std::atomic_int quitCode { 0 };
    std::thread::id currentThreadID = std::this_thread::get_id();
    std::queue<Event*> eventQueue;
    std::queue<Timer::Function> timerFunctionQueue;
    std::list<Timer*> timerList;
    std::mutex eventMutex;
    std::thread::id workThreadId;
    Semaphore eventSemaPhore { 0 };
    Semaphore waitSemaPhore { 0 };
    Semaphore quitSemaPhore { 0 };
    Thread* queueThread = nullptr;
    std::mutex* commonMutex = nullptr;
    TimeMethod timeMethod;
};

Queue::Queue(int queueId)
{
    if (queueId < 0) {
        LOG_WARNING("queue id not set");
    }
    if (!m_hpr) {
        m_hpr = new QueueHelper;
    }
    m_hpr->queueId = queueId;
}

Queue::~Queue()
{
    m_hpr->eventSemaPhore.reset();
    m_hpr->waitSemaPhore.reset();
    m_hpr->quitSemaPhore.reset();
    //
    m_hpr->eventMutex.lock();
    for (; !m_hpr->eventQueue.empty();) {
        Event* event = m_hpr->eventQueue.front();
        if (event) {
            delete event;
        } else {
            LOG_WARNING("event is null");
        }
        m_hpr->eventQueue.pop();
    }
    m_hpr->eventMutex.unlock();

    for (Timer* timer : m_hpr->timerList) {
        if (timer) {
            delete timer;
        } else {
            LOG_WARNING("timer is null");
        }
    }
    if (m_hpr->queueThread) {
        delete m_hpr->queueThread;
    }
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
}

int Queue::runInBlock()
{
    m_hpr->workThreadId = std::this_thread::get_id();
    if (m_hpr->isRunning && !m_hpr->queueThread) {
        LOG_WARNING("queue has run");
        return -1;
    }
    m_hpr->isRunning = true;
    if (m_hpr->quitFlag) {
        return m_hpr->quitCode;
    }
    begin();
    for (; !m_hpr->quitFlag;) {
        processEvent();
        if (m_hpr->quitFlag) {
            break;
        }
        processNextSleepTime();
        m_hpr->eventSemaPhore.acquire_nano(m_hpr->timeMethod.miniTime);
    }
    processEvent();
    end();
    m_hpr->isRunning = false;
    m_hpr->quitSemaPhore.reset();
    return m_hpr->quitCode;
}

void Queue::runInThread()
{
    if (m_hpr->isRunning || m_hpr->queueThread) {
        LOG_WARNING("queue has run");
        return;
    }
    m_hpr->isRunning = true;
    m_hpr->queueThread = new Thread([this]() {
        runInBlock();
    });
    m_hpr->queueThread->start();
}

void Queue::quit(int quitCode)
{
    if (m_hpr->quitFlag) {
        return;
    }
    m_hpr->quitFlag = true;
    m_hpr->quitCode = quitCode;
    m_hpr->eventSemaPhore.reset();
    m_hpr->waitSemaPhore.reset();
    if (m_hpr->queueThread) {
        m_hpr->queueThread->stop();
    }
}

void Queue::waitforQuit(uint32_t milli_s)
{
    if (std::this_thread::get_id() == m_hpr->workThreadId) {
        LOG_WARNING("can not wait in same thread");
    } else {
        if (m_hpr->queueThread) {
            m_hpr->queueThread->stop(milli_s);
        } else {
            m_hpr->quitSemaPhore.reset();
            int32_t time = milli_s;
            if (time <= 0) {
                time = -1;
            }
            if (m_hpr->quitSemaPhore.acquire(time) == false) {
                LOG_WARNING("wait time out");
            }
        }
    }
}

void Queue::wait(uint32_t milli_s)
{
    m_hpr->waitSemaPhore.acquire(milli_s);
}

void Queue::wakeUp()
{
    m_hpr->waitSemaPhore.reset();
}

bool Queue::isRunning() const
{
    return m_hpr->isRunning;
}

bool Queue::isReadyToQuit() const
{
    return m_hpr->quitFlag;
}

bool Queue::isBusy() const
{
    return m_hpr->isBusy;
}

bool Queue::isRunInthread() const
{
    if (m_hpr->queueThread) {
        return true;
    }
    return false;
}

int Queue::quitCode() const
{
    return m_hpr->quitCode;
}

int Queue::eventCount() const
{
    return (int)m_hpr->eventQueue.size();
}

Timer* Queue::createTimer(uint32_t interval_milli_s, bool loop, const Timer::Function& function)
{
    Timer* timer = new Timer(interval_milli_s, loop, function);
    m_hpr->timerList.push_back(timer);
    return timer;
}

void Queue::destroyTimer(Timer* timer)
{
    if (!timer) {
        LOG_WARNING("timer is null");
    }
    m_hpr->timerList.remove(timer);
    delete timer;
}

void Queue::onceTimer(uint32_t interval_milli_s, const Timer::Function& function)
{
    Timer* timer = createTimer(interval_milli_s, false, nullptr);
    timer->setFunction([timer, function, this] {
        if (function) {
            function();
            destroyTimer(timer);
        }
    });
    timer->start();
}

void Queue::postEvent(Event* event)
{
    if (event->queueId() < 0) {
        LOG_WARNING("queue id not set");
        return;
    } else {
        if (event->queueId() != m_hpr->queueId) {
            LOG_WARNING("queue not equal target id");
            return;
        }
    }
    if (m_hpr->eventQueue.size() > QUEUE_MAX_NUM) {
        LOG_WARNING("queue is full");
        std::terminate();
    }
    m_hpr->eventMutex.lock();
    m_hpr->eventQueue.push(event);
    m_hpr->eventMutex.unlock();
    m_hpr->eventSemaPhore.reset();
}

void Queue::setMutex(std::mutex& mutex)
{
    m_hpr->commonMutex = &mutex;
}

void Queue::lock() const
{
    if (m_hpr->commonMutex) {
        m_hpr->commonMutex->lock();
    } else {
        LOG_WARNING("mutex not set");
    }
}

void Queue::unlock() const
{
    if (m_hpr->commonMutex) {
        m_hpr->commonMutex->unlock();
    } else {
        LOG_WARNING("mutex not set");
    }
}

void Queue::processEvent()
{
    m_hpr->isBusy = true;
    for (; !m_hpr->timerFunctionQueue.empty();) {
        auto func = m_hpr->timerFunctionQueue.front();
        if (func) {
            func();
        }
        m_hpr->timerFunctionQueue.pop();
    }
    for (; !m_hpr->eventQueue.empty();) {
        Event* event = m_hpr->eventQueue.front();
        if (event) {
            if (event->type() != Event::UNKNOWN) {
                eventChanged(event);
            }
            if (event->function()) {
                event->function()();
            }
            delete event;
        } else {
            LOG_WARNING("event is null");
        }
        m_hpr->eventMutex.lock();
        m_hpr->eventQueue.pop();
        m_hpr->eventMutex.unlock();
    }
    m_hpr->isBusy = false;
}

void Queue::processNextSleepTime()
{
    m_hpr->timeMethod.miniTime = -1;
    m_hpr->timeMethod.remainTime = -1;
    m_hpr->timeMethod.intervalTime = 0;
    m_hpr->timeMethod.currentTime = Utils::getNanoCurrent();
#if TIMER_USE_PRECISION
    m_hpr->timeMethod.expactCount = 0;
    for (Timer* timer : m_hpr->timerList) {
        if (!timer->active()) {
            continue;
        }
        m_hpr->timeMethod.intervalTime = (uint64_t)timer->interval() * 1000000U;
        if (m_hpr->timeMethod.intervalTime <= 0) {
            m_hpr->timeMethod.intervalTime = 100000U;
        }
        m_hpr->timeMethod.expactCount = (m_hpr->timeMethod.currentTime - timer->m_startTimer + TIMER_ERROR_OFFSET) / m_hpr->timeMethod.intervalTime;
        if (m_hpr->timeMethod.expactCount > timer->m_loopCount) {
            if (timer->function()) {
                for (uint64_t offset_i = 0; offset_i < m_hpr->timeMethod.expactCount - timer->m_loopCount; offset_i++) {
                    m_hpr->timerFunctionQueue.push(timer->function());
                    if (!timer->loop()) {
                        timer->stop();
                        break;
                    }
                }
            }
            if (timer->active()) {
                timer->m_loopCount = m_hpr->timeMethod.expactCount;
            }
            m_hpr->timeMethod.miniTime = 0;
        } else {
            m_hpr->timeMethod.remainTime = m_hpr->timeMethod.intervalTime - (m_hpr->timeMethod.currentTime - timer->m_startTimer) % m_hpr->timeMethod.intervalTime;
            if (m_hpr->timeMethod.remainTime < 0) {
                m_hpr->timeMethod.remainTime = 0;
            }
            if (m_hpr->timeMethod.miniTime < 0) {
                m_hpr->timeMethod.miniTime = m_hpr->timeMethod.remainTime;
            } else {
                if (m_hpr->timeMethod.remainTime < m_hpr->timeMethod.miniTime) {
                    m_hpr->timeMethod.miniTime = m_hpr->timeMethod.remainTime;
                }
            }
        }
    }
#else
    for (Timer* timer : m_hpr->timerList) {
        if (!timer->active()) {
            continue;
        }
        m_hpr->timeMethod.intervalTime = (uint64_t)timer->interval() * 1000000U;
        if (m_hpr->timeMethod.intervalTime <= 0) {
            m_hpr->timeMethod.intervalTime = 100000U;
        }
        m_hpr->timeMethod.remainTime = timer->m_startTimer + m_hpr->timeMethod.intervalTime - m_hpr->timeMethod.currentTime - TIMER_ERROR_OFFSET;
        if (m_hpr->timeMethod.remainTime <= 0) {
            timer->m_startTimer = m_hpr->timeMethod.currentTime;
            if (timer->function()) {
                m_hpr->timerFunctionQueue.push(timer->function());
                if (!timer->loop()) {
                    timer->stop();
                    break;
                }
            }
            m_hpr->timeMethod.miniTime = 0;
        } else {
            if (m_hpr->timeMethod.miniTime < 0) {
                m_hpr->timeMethod.miniTime = m_hpr->timeMethod.remainTime;
            } else {
                if (m_hpr->timeMethod.remainTime < m_hpr->timeMethod.miniTime) {
                    m_hpr->timeMethod.miniTime = m_hpr->timeMethod.remainTime;
                }
            }
        }
    }
#endif
}

DCUS_NAMESPACE_END
