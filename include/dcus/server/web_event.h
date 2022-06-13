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

#ifndef DCUS_WEB_EVENT_H
#define DCUS_WEB_EVENT_H

#include "dcus/base/data.h"
#include "dcus/base/event.h"
#include "dcus/base/setting.h"
#include "dcus/server/web_feed.h"
#include "dcus/server/web_init.h"

DCUS_NAMESPACE_BEGIN

class WebEvent : public Event {
public:
    enum WebEventType {
        FUNCTION = 0,
        REQ_INIT,
        REQ_CHECK,
        REQ_DOWNLOAD,
        REQ_VERIFY,
        REQ_DISTRIBUTE,
        REQ_STOP,
        REQ_CLEAR,
        REQ_CLEARALL,
        REQ_FEEDBACK,
    };
    explicit WebEvent(WebEventType type) noexcept
        : Event(DCUS_QUEUE_ID_WEB, type)
    {
    }
};

class WebInitEvent : public WebEvent {
public:
    explicit WebInitEvent(const WebInit& webInit) noexcept
        : WebEvent(REQ_INIT)
        , m_webInit(webInit)
    {
    }

public:
    inline const WebInit& webInit() const noexcept
    {
        return m_webInit;
    }

private:
    WebInit m_webInit;
};

class WebFeedEvent : public WebEvent {
public:
    explicit WebFeedEvent(const WebFeed& webFeed) noexcept
        : WebEvent(REQ_FEEDBACK)
        , m_webFeed(webFeed)
    {
    }

public:
    inline const WebFeed& webFeed() const noexcept
    {
        return m_webFeed;
    }

private:
    WebFeed m_webFeed;
};

DCUS_NAMESPACE_END

#endif // DCUS_WEB_EVENT_H
