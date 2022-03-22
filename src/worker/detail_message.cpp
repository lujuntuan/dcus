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

#include "dcus/worker/detail_message.h"
#include "commonlib/string.h"
#include <iostream>

DCUS_NAMESPACE_BEGIN

std::ostream& operator<<(std::ostream& ostream, const DetailMessage& detail) noexcept
{
    ostream << "{\n";
    ostream << "  [state]: " << Domain::getMrStateStr(detail.state()) << "\n";
    ostream << "  [last]: " << Domain::getMrStateStr(detail.last()) << "\n";
    ostream << "  [active]: " << (detail.active() ? std::string("true") : std::string("false")) << "\n";
    ostream << "  [error]: " << detail.error() << "\n";
    ostream << "  [step]: " << (Common::doubleToString(detail.step()) + " %, ") << "\n";
    ostream << "  [progress]: " << (Common::doubleToString(detail.progress()) + " %, ") << "\n";
    ostream << "  [messgae]: " << detail.message() << "\n";
    ostream << "  [domains-size]: " << detail.details().size() << "\n";
    ostream << "}";
    return ostream;
}

DCUS_NAMESPACE_END
