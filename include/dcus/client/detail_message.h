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

#ifndef DCUS_DETAIL_MESSAGE_H
#define DCUS_DETAIL_MESSAGE_H

#include "dcus/base/variant.h"
#include "dcus/domain.h"
#include "dcus/upgrade.h"

DCUS_NAMESPACE_BEGIN

struct DetailMessage {
    DetailMessage() = default;
    template <typename T>
    explicit DetailMessage(ServerState _state, ServerState _last, bool _active, int _error, float _step, float _progress, const std::string& _messgae, T&& _details) noexcept
        : state(_state)
        , last(_last)
        , active(_active)
        , error(_error)
        , step(_step)
        , progress(_progress)
        , message(_messgae)
        , details(std::forward<T>(_details))
    {
    }
    ServerState state = MR_UNKNOWN;
    ServerState last = MR_UNKNOWN;
    bool active = false;
    int error = 0;
    float step = .0f;
    float progress = .0f;
    std::string message;
    Details details;

public:
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const DetailMessage& webStatus) noexcept;
};

VARIANT_DECLARE_TYPE(DetailMessage);

DCUS_NAMESPACE_END

#endif // DCUS_DETAIL_MESSAGE_H
