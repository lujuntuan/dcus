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

#ifndef DCUS_CORE_CONFIG_HTTP_H
#define DCUS_CORE_CONFIG_HTTP_H

#include "dcus/base/log.h"
#include "dcus/base/variant.h"
#include "dcus/setting.h"
#include "dcus/utils/host.h"
#include "importlib/httplib.hpp"
#include <regex>

DCUS_NAMESPACE_BEGIN

namespace Core {

inline bool loadClientConfig(httplib::Client& client, const VariantMap& config)
{
    if (config.value("net_interface").isValid()) {
        client.set_interface(config.value("net_interface").toCString());
    }
    if (config.value("web_timeout").isValid()) {
        client.set_read_timeout(config.value("web_timeout").toInt() / 1000);
        client.set_write_timeout(config.value("web_timeout").toInt() / 1000);
    } else {
        client.set_read_timeout(DCUS_WEB_TIMEOUT / 1000);
        client.set_write_timeout(DCUS_WEB_TIMEOUT / 1000);
    }
#ifdef DCUS_USE_HTTPS
    if (config.value("web_ca_cert_path").isValid()) {
        client.set_ca_cert_path(config.value("web_ca_cert_path").toStringCStr());
        client.enable_server_certificate_verification(true);
    } else {
        client.enable_server_certificate_verification(false);
    }
#else
    client.enable_server_certificate_verification(false);
#endif
    //---------
    if (config.value("web_username").isValid() && config.value("web_password").isValid()) {
        client.set_basic_auth(config.value("web_username").toCString(), config.value("web_password").toCString());
        client.set_digest_auth(config.value("web_username").toCString(), config.value("web_password").toCString());
    }
    if (config.value("web_proxy_url").isValid() && config.value("web_proxy_port").isValid()) {
        client.set_proxy(config.value("web_proxy_url").toCString(), config.value("web_proxy_port").toInt());
        if (config.value("web_proxy_username").isValid() && config.value("web_proxy_password").isValid()) {
            client.set_proxy_basic_auth(config.value("web_proxy_username").toCString(), config.value("web_proxy_password").toCString());
            client.set_proxy_digest_auth(config.value("web_proxy_username").toCString(), config.value("web_proxy_password").toCString());
        }
    } else {
        std::string proxyUrl;
        if (Utils::getEnvironment("https_proxy").empty()) {
            proxyUrl = Utils::getEnvironment("https_proxy");
        } else if (Utils::getEnvironment("HTTPS_PROXY").empty()) {
            proxyUrl = Utils::getEnvironment("HTTPS_PROXY");
        } else if (Utils::getEnvironment("http_proxy").empty()) {
            proxyUrl = Utils::getEnvironment("http_proxy");
        } else if (Utils::getEnvironment("HTTP_PROXY").empty()) {
            proxyUrl = Utils::getEnvironment("HTTP_PROXY");
        }
        if (!proxyUrl.empty()) {
            std::regex proxyRegex(R"(^(?:(\w+:\/\/)?(?:(\w+))(?::(\w+))?@)?(\S+)(?::(\d{1,5})??)$)");
            std::smatch matchs;
            bool isMatch = std::regex_match(proxyUrl, matchs, proxyRegex);
            if (isMatch && matchs.size() == 6) {
                std::string url = matchs[4];
                std::string port = matchs[5];
                std::string name = matchs[2];
                std::string password = matchs[3];
                client.set_proxy(url.c_str(), std::atoi(port.c_str()));
                client.set_proxy_basic_auth(name.c_str(), password.c_str());
                client.set_proxy_digest_auth(name.c_str(), password.c_str());
            }
        }
    }
    return true;
}

inline bool loadServerConfig(httplib::Server& server, const VariantMap& config)
{
    if (config.value("web_timeout").isValid()) {
        server.set_read_timeout(config.value("web_timeout").toInt() / 1000);
        server.set_write_timeout(config.value("web_timeout").toInt() / 1000);
    } else {
        server.set_read_timeout(DCUS_WEB_TIMEOUT / 1000);
        server.set_write_timeout(DCUS_WEB_TIMEOUT / 1000);
    }
    if (config.value("web_html_dir").isValid()) {
        server.set_base_dir(config.value("web_html_dir").toCString(), "/");
    }
    return true;
}
}

DCUS_NAMESPACE_END

#endif // DCUS_CORE_CONFIG_HTTP_H
