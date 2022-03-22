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

#include "dcus/master/web_feed.h"
#include <iostream>

DCUS_NAMESPACE_BEGIN

std::string WebFeed::getTypeStr(int type) noexcept
{
    switch (type) {
    case TP_UNKNOWN:
        return "unknown";
    case TP_DEPLOY:
        return "deploy";
    case TP_CANCEL:
        return "cancel";
    default:
        return "unknown";
    }
}

std::string WebFeed::getExecutionStr(int execution) noexcept
{
    switch (execution) {
    case EXE_UNKNOWN:
        return "unknown";
    case EXE_CLOSED:
        return "closed";
    case EXE_CANCELED:
        return "canceled";
    case EXE_REJECTED:
        return "rejected";
    case EXE_PROCEEDING:
        return "proceeding";
    case EXE_SCHEDULED:
        return "scheduled";
    case EXE_RESUMED:
        return "resumed";
    default:
        return "closed";
    }
}

std::string WebFeed::getResultStr(int result) noexcept
{
    switch (result) {
    case RS_UNKNOWN:
        return "unknown";
    case RS_SUCCESS:
        return "success";
    case RS_FAILURE:
        return "failure";
    case RS_NONE:
        return "none";
    default:
        return "success";
    }
}

std::ostream& operator<<(std::ostream& ostream, const WebFeed& webFeed) noexcept
{
    ostream << "{\n";
    ostream << "  [id]: " << webFeed.id() << "\n";
    ostream << "  [type]: " << WebFeed::getTypeStr(webFeed.type()) << "\n";
    ostream << "  [execution]: " << WebFeed::getExecutionStr(webFeed.execution()) << "\n";
    ostream << "  [result]: " << WebFeed::getResultStr(webFeed.result()) << "\n";
    if (!webFeed.details().empty()) {
        ostream << "  [details]: ";
        for (size_t i = 0; i < webFeed.details().size(); i++) {
            if (i > 0) {
                ostream << ",";
            }
            ostream << webFeed.details().at(i);
        }
        ostream << "\n";
    }
    if (webFeed.type() == WebFeed::TP_DEPLOY) {
        ostream << "  [progress]: " << webFeed.progress().first << "," << webFeed.progress().second << "\n";
    }
    ostream << "}";
    return ostream;
}

DCUS_NAMESPACE_END
