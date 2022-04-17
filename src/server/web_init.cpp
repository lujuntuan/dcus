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

#include "dcus/server/web_init.h"
#include "dcus/utils/verify.h"
#include <iostream>

DCUS_NAMESPACE_BEGIN

WebInit::WebInit(const std::string& url, const std::string& tenant, const std::string& id, const Token& token) noexcept
    : m_url(url)
    , m_tenant(tenant)
    , m_id(id)
    , m_token(token)
{
}

std::ostream& operator<<(std::ostream& ostream, const WebInit& webInit) noexcept
{
    ostream << "{\n";
    ostream << "  [url]: " << webInit.url() << "\n";
    ostream << "  [tenant]: " << webInit.tenant() << "\n";
    ostream << "  [id]: " << webInit.id() << "\n";
    ostream << "  [token]: " << webInit.token().first << " " << webInit.token().second << "\n";
    ostream << "}";
    return ostream;
}

DCUS_NAMESPACE_END
