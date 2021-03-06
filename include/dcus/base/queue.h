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

#ifndef DCUS_QUEUE_H
#define DCUS_QUEUE_H

#include "dcus/base/define.h"
#include "dcus/base/event.h"
#include "dcus/base/timer.h"
#include <memory>
#include <mutex>

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT Queue {
    CLASS_DISSABLE_COPY_AND_ASSIGN(Queue)
public:
    explicit Queue(int queueId);
    virtual ~Queue();
    int runInBlock();
    void runInThread();
    void quit(int quitCode = 0);
    void waitforQuit(uint32_t milli_s = 100);
    void wait(uint32_t milli_s);
    void wakeUp();
    bool isRunning() const;
    bool isReadyToQuit() const;
    bool isBusy() const;
    bool isRunInthread() const;
    int quitCode() const;
    int eventCount() const;
    std::shared_ptr<Timer> createTimer(uint32_t interval_milli_s, bool loop = false, const Timer::Function& function = nullptr);
    void onceTimer(uint32_t interval_milli_s, const Timer::Function& function = nullptr);
    void postEvent(const std::shared_ptr<Event>& event);
    void setMutex(std::mutex& mutex);
    void lock() const;
    void unlock() const;

protected:
    virtual void begin() { }
    virtual void end() { }
    virtual void eventChanged(const std::shared_ptr<Event>& event) = 0;
    virtual void processEvent();

private:
    void addTimer(const std::shared_ptr<Timer>& timer);
    void removeTimer(const std::shared_ptr<Timer>& timer);
    void removeTimer(Timer* timer);
    void processNextSleepTime();

private:
    friend Timer;
    struct QueueHelper* m_queueHelper = nullptr;
};

DCUS_NAMESPACE_END

#endif // DCUS_QUEUE_H
