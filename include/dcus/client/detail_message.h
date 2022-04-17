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

#ifndef DCUS_DETAIL_MESSAGE_H
#define DCUS_DETAIL_MESSAGE_H

#include "dcus/domain.h"
#include "dcus/upgrade.h"

DCUS_NAMESPACE_BEGIN

struct DetailMessage {
    DetailMessage() = default;
    template <typename T>
    explicit DetailMessage(ServerState state, ServerState last, bool active, int error, float step, float progress, const std::string& messgae, T&& details) noexcept
        : m_state(state)
        , m_last(last)
        , m_active(active)
        , m_error(error)
        , m_step(step)
        , m_progress(progress)
        , m_message(messgae)
        , m_details(std::forward<T>(details))
    {
    }
    CREATE_PUBLIC_PROPERTY(ServerState, state, MR_UNKNOWN)
    CREATE_PUBLIC_PROPERTY(ServerState, last, MR_UNKNOWN)
    CREATE_PUBLIC_PROPERTY(bool, active, false)
    CREATE_PUBLIC_PROPERTY(int, error, 0)
    CREATE_PUBLIC_PROPERTY(float, step, .0f)
    CREATE_PUBLIC_PROPERTY(float, progress, .0f)
    CREATE_PUBLIC_PROPERTY(std::string, message, "")
    CREATE_PUBLIC_PROPERTY(Details, details, Details())

public:
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const DetailMessage& webStatus) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_DETAIL_MESSAGE_H
