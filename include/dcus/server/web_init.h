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

#ifndef DCUS_WEB_INIT_H
#define DCUS_WEB_INIT_H

#include "dcus/base/define.h"
#include <string>

DCUS_NAMESPACE_BEGIN

struct DCUS_EXPORT WebInit final {
    using Token = std::pair<std::string, std::string>;
    WebInit() = default;
    explicit WebInit(const std::string& _url, const std::string& _tenant, const std::string& _id, const Token& _token = Token()) noexcept
        : url(_url)
        , tenant(_tenant)
        , id(_id)
        , token(_token)
    {
    }
    std::string url;
    std::string tenant;
    std::string id;
    Token token;

public:
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const WebInit& webInit) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_WEB_INIT_H
