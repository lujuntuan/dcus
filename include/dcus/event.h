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

#ifndef DCUS_EVENT_H
#define DCUS_EVENT_H

#include "dcus/define.h"
#include <functional>

DCUS_NAMESPACE_BEGIN

class Event {
public:
    using Function = std::function<void()>;
    enum EventType {
        UNKNOWN = -1
    };

protected:
    explicit Event(int queueId, int type = UNKNOWN, const Function& function = nullptr) noexcept
        : m_queueId(queueId)
        , m_type(type)
        , m_function(function)
    {
    }
    virtual ~Event() noexcept
    {
    }

public:
    inline bool valid() const noexcept
    {
        return m_type >= 0;
    }
    inline int queueId() const noexcept
    {
        return m_queueId;
    }
    inline int type() const noexcept
    {
        return m_type;
    }
    inline Function function() const noexcept
    {
        return m_function;
    }
    inline int isAccepted() const noexcept
    {
        return m_accepted;
    }
    inline void setFunction(const Function& function) noexcept
    {
        m_function = function;
    }
    inline void setAccept(bool accepted) noexcept
    {
        m_accepted = accepted;
    }

private:
    friend class Queue;
    int m_queueId = -1;
    int m_type = UNKNOWN;
    Function m_function = nullptr;
    bool m_accepted = false;
};

DCUS_NAMESPACE_END

#endif // DCUS_EVENT_H
