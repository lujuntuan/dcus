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

#include "dcus/base/thread.h"
#include "commonlib/time.h"
#include "dcus/base/log.h"
#include <atomic>
#include <condition_variable>
#include <thread>
#include <unordered_map>

#define m_hpr m_threadHelper

DCUS_NAMESPACE_BEGIN

struct ThreadHelper {
    std::thread* thread = nullptr;
    std::atomic_bool isRunning { false };
    std::atomic_bool isReadyFinished { true };
    Thread::Function startFunction = nullptr;
    Thread::Function endFunction = nullptr;
    std::mutex mutex;
    std::condition_variable_any condition;
    static std::unordered_map<std::thread::id, Thread*> _ThreadMap;
};

std::unordered_map<std::thread::id, Thread*> ThreadHelper::_ThreadMap;

Thread::Thread()
{
    createHelper();
}

Thread::~Thread()
{
    if (m_hpr->thread) {
        stop();
    }
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
}

Thread* Thread::currentThread()
{
    return ThreadHelper::_ThreadMap.find(std::this_thread::get_id())->second;
}

void Thread::sleep(uint32_t s)
{
    Common::sleep(s);
}

void Thread::sleepMilli(uint32_t milli_s)
{
    Common::sleepMilli(milli_s);
}

void Thread::sleepNano(uint64_t nano_s)
{
    Common::sleepNano(nano_s);
}

void Thread::yield()
{
    std::this_thread::yield();
}

bool Thread::isRunning() const
{
    return m_hpr->isRunning;
}

bool Thread::isReadyFinished() const
{
    return m_hpr->isReadyFinished;
}

void Thread::setReadyFinished(bool readFinished)
{
    m_hpr->isReadyFinished = readFinished;
}

void Thread::setStartFunction(const Function& startFunction)
{
    m_hpr->startFunction = startFunction;
}

void Thread::setEndFunction(const Function& endFunction)
{
    m_hpr->endFunction = endFunction;
}

bool Thread::start()
{
    if (!m_runFunction) {
        return false;
    }
    if (m_hpr->isRunning) {
        return false;
    }
    if (m_hpr->thread) {
        stop();
    }
    m_hpr->isReadyFinished = false;
    m_hpr->isRunning = true;
    try {
        m_hpr->thread = new std::thread([this]() {
            ThreadHelper::_ThreadMap.emplace(std::this_thread::get_id(), this);
            if (m_hpr->startFunction) {
                m_hpr->startFunction();
            }
            m_runFunction();
            if (m_hpr->endFunction) {
                m_hpr->endFunction();
            }
            // wakeup
            m_hpr->isRunning = false;
            std::lock_guard<std::mutex> lock(m_hpr->mutex);
            m_hpr->condition.notify_one();
            //
            ThreadHelper::_ThreadMap.erase(std::this_thread::get_id());
        });
        return true;
    } catch (...) {
        LOG_WARNING("create thread error");
        return false;
    }
}

bool Thread::stop(uint32_t milli_s)
{
    m_hpr->isReadyFinished = true;
    if (m_hpr->isRunning) {
        if (!m_hpr->thread) {
            return false;
        }
        if (!m_hpr->thread->joinable()) {
            delete m_hpr->thread;
            m_hpr->thread = nullptr;
            return true;
        }
    } else {
        if (m_hpr->thread) {
            if (m_hpr->thread->joinable()) {
                m_hpr->thread->join();
            }
            delete m_hpr->thread;
            m_hpr->thread = nullptr;
        }
        return true;
    }
    // sleep
    std::unique_lock<std::mutex> lock(m_hpr->mutex);
    if (milli_s > 0) {
        m_hpr->condition.wait_for(m_hpr->mutex, std::chrono::milliseconds(std::move(milli_s)));
    } else {
        m_hpr->condition.wait(m_hpr->mutex);
    }
    //
    if (m_hpr->isRunning) {
        LOG_WARNING("thread is forced to end");
        try {
            m_hpr->thread->detach();
            auto nativeHandle = m_hpr->thread->native_handle();
            DCUS_UNUSED(nativeHandle);
#ifdef HAS_PTHREAD
            pthread_cancel(nativeHandle);
#else
#ifdef WIN32
            // kill thread
#endif
#endif
            delete m_hpr->thread;
        } catch (...) {
            LOG_WARNING("destroy thread error");
            m_hpr->isRunning = false;
            m_hpr->thread = nullptr;
            return false;
        }
    } else {
        m_hpr->thread->join();
        delete m_hpr->thread;
    }
    m_hpr->isRunning = false;
    m_hpr->thread = nullptr;
    return true;
}

void Thread::createHelper()
{
    if (!m_hpr) {
        m_hpr = new ThreadHelper;
    }
}

DCUS_NAMESPACE_END
