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
    explicit WebInit(const std::string& url, const std::string& tenant, const std::string& id, const Token& token = Token()) noexcept;
    CREATE_PUBLIC_PROPERTY(std::string, url, "")
    CREATE_PUBLIC_PROPERTY(std::string, tenant, "")
    CREATE_PUBLIC_PROPERTY(std::string, id, "")
    CREATE_PUBLIC_PROPERTY(Token, token, Token())
public:
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const WebInit& webInit) noexcept;
};

DCUS_NAMESPACE_END

#endif // DCUS_WEB_INIT_H
