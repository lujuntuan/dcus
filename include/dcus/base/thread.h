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

#ifndef DCUS_THREAD_H
#define DCUS_THREAD_H

#include "dcus/base/define.h"
#include <functional>
#include <stdint.h>

DCUS_NAMESPACE_BEGIN

class DCUS_EXPORT Thread final {
    CLASS_DISSABLE_COPY_AND_ASSIGN(Thread)
private:
    void create();

public:
    using Function = std::function<void()>;
    Thread();
    template <typename T, typename... ARGS>
    explicit Thread(T&& function, ARGS&&... args)
    {
        create();
        m_runFunction = std::function<typename std::result_of<T(ARGS...)>::type()>(std::bind(std::forward<T>(function), std::forward<ARGS>(args)...));
    }
    ~Thread();
    static Thread* currentThread();
    static void sleep(uint32_t s);
    static void sleepMilli(uint32_t milli_s);
    static void sleepNano(uint64_t nano_s);
    static void yield();

public:
    bool isRunning() const;
    bool isReadyFinished() const;
    void setReadyFinished(bool readFinished);
    void setStartFunction(const Function& startFunction);
    void setEndFunction(const Function& endFunction);
    bool start();
    template <typename T, typename... ARGS>
    bool start(T&& function, ARGS&&... args)
    {
        std::function<typename std::result_of<T(ARGS...)>::type()> task(std::bind(std::forward<T>(function), std::forward<ARGS>(args)...));
        m_runFunction = task;
        return start();
    }
    bool stop(uint32_t milli_s = 100);

private:
    struct ThreadHelper* m_threadHelper = nullptr;
    Function m_runFunction = nullptr;
};

DCUS_NAMESPACE_END

#endif // DCUS_THREAD_H
