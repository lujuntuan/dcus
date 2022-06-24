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

#ifndef DCUS_WEB_FEED_H
#define DCUS_WEB_FEED_H

#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct DCUS_EXPORT WebFeed final {
    using Progress = std::pair<int, int>;
    using Details = std::vector<std::string>;
    enum Type {
        TP_UNKNOWN = 0,
        TP_DEPLOY,
        TP_CANCEL,
    };
    enum Execution {
        EXE_UNKNOWN = 0,
        EXE_CLOSED,
        EXE_PROCEEDING,
        EXE_CANCELED,
        EXE_SCHEDULED,
        EXE_REJECTED,
        EXE_RESUMED,
    };
    enum Result {
        RS_UNKNOWN = 0,
        RS_SUCCESS,
        RS_FAILURE,
        RS_NONE,
    };
    WebFeed() = default;
    explicit WebFeed(const std::string& _id, Type _type, Execution _execution, Result _result, const Details& _details = Details(), const Progress& _progress = { 1, 1 }) noexcept
        : id(_id)
        , type(_type)
        , execution(_execution)
        , result(_result)
        , details(_details)
        , progress(_progress)
    {
    }
    std::string id;
    Type type = TP_UNKNOWN;
    Execution execution = EXE_UNKNOWN;
    Result result = RS_UNKNOWN;
    Details details;
    Progress progress = std::make_pair(0, 0);

public:
    static std::string getTypeStr(int type) noexcept;
    static std::string getExecutionStr(int execution) noexcept;
    static std::string getResultStr(int result) noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const WebFeed& webStatus) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_WEB_FEED_H
