/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2021
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef DCUS_SERVER_EVENT_H
#define DCUS_SERVER_EVENT_H

#include "dcus/domain.h"
#include "dcus/base/event.h"
#include "dcus/base/setting.h"
#include "dcus/upgrade.h"

DCUS_NAMESPACE_BEGIN

class ServerEvent : public Event {
public:
    enum ServerEventType {
        FUNCTION = 0,
        REQ_ACTIVE,
        REQ_IDLE,
        REQ_UPGRADE,
        REQ_CANCEL,
        REQ_PULL,

        RES_INIT,
        RES_ERROR,
        RES_DOWNLOAD_DONE,
        RES_VERIFY_DONE,
        RES_DISTRUBUTE_DONE,
        RES_FEEDBACK_DONE,
        RES_TRANSFER_PROGRESS,

        RES_DOMAIN,
    };
    explicit ServerEvent(ServerEventType type, const Data& data = Data()) noexcept
        : Event(DCUS_QUEUE_ID_SERVER, type)
        , m_data(data)
    {
    }
    inline const Data& data() const noexcept
    {
        return m_data;
    }

private:
    Data m_data;
};

class ServerUpgradeEvent : public ServerEvent {
public:
    template <typename T>
    explicit ServerUpgradeEvent(T&& upgrade) noexcept
        : ServerEvent(REQ_UPGRADE)
        , m_upgrade(std::forward<T>(upgrade))
    {
    }
    inline const Upgrade& upgrade() const noexcept
    {
        return m_upgrade;
    }

private:
    Upgrade m_upgrade;
};

class ServerTransferEvent : public ServerEvent {
public:
    template <typename T>
    explicit ServerTransferEvent(T&& transfers) noexcept
        : ServerEvent(RES_TRANSFER_PROGRESS)
        , m_transfers(std::forward<T>(transfers))
    {
    }
    inline const Transfers& transfers() const noexcept
    {
        return m_transfers;
    }

private:
    Transfers m_transfers;
};

class ServerDomainEvent : public ServerEvent {
public:
    template <typename T>
    explicit ServerDomainEvent(T&& domain, bool discovery) noexcept
        : ServerEvent(RES_DOMAIN)
        , m_domain(std::forward<T>(domain))
        , m_discovery(discovery)
    {
    }
    inline const Domain& domain() const noexcept
    {
        return m_domain;
    }
    bool discovery() const noexcept
    {
        return m_discovery;
    }

private:
    Domain m_domain;
    bool m_discovery = false;
};

DCUS_NAMESPACE_END

#endif // DCUS_SERVER_EVENT_H
