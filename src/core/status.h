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

#ifndef DCUS_CORE_STATUS_H
#define DCUS_CORE_STATUS_H

#include "dcus/base/define.h"
#include <functional>

DCUS_NAMESPACE_BEGIN

namespace Core {
using BreakFunction = std::function<bool()>;
enum State {
    SUCCEED = 0,
    FAILED,
    CANCELED,
};
struct Status {
    Status() = default;
    explicit Status(State _state, int _error)
        : state(_state)
        , error(_error)
    {
    }
    State state = SUCCEED;
    int error = 0;
};
}
DCUS_NAMESPACE_END

#endif // DCUS_CORE_STATUS_H
