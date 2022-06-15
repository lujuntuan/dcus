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

#include "dcus/server/hawkbit_queue.h"
#include "core/core.h"
#include "dcus/base/setting.h"
#include "dcus/server/server_engine.h"
#include "dcus/utils/string.h"
#include "dcus/utils/time.h"
#include "importlib/json11.hpp"

#define m_hpr m_hawkbitHelper

DCUS_NAMESPACE_BEGIN

struct HawkbitHelper {
    std::string url;
    Core::Token token;
    std::string path;
};

HawkbitQueue::HawkbitQueue()
    : WebQueue()
{
    if (!m_hpr) {
        m_hpr = new HawkbitHelper;
    }
}

HawkbitQueue::~HawkbitQueue()
{
    if (m_hpr) {
        delete m_hpr;
        m_hpr = nullptr;
    }
}

bool HawkbitQueue::init(const WebInit& webInit)
{
    m_hpr->url = webInit.url();
    m_hpr->token = webInit.token();
    m_hpr->path = Utils::stringSprintf("/%s/controller/v1/%s", webInit.tenant(), webInit.id());
    return true;
}

bool HawkbitQueue::detect()
{
    Core::Status status;
    std::string body;
    status = Core::getMessage(m_hpr->url, m_hpr->path, body, dcus_server_config, m_hpr->token);
    if (status.state() != Core::SUCCEED) {
        // postError(801);
        return false;
    }
    std::string jsonErrorStr;
    json11::Json json = json11::Json::parse(body, jsonErrorStr);
    if (!jsonErrorStr.empty() || json.is_null()) {
        LOG_WARNING("error=", jsonErrorStr);
        return false;
    }
    if (json.is_null()) {
        return false;
    }
    if (!json["_links"].is_object()) {
        postIdle();
        return true;
    }
#if (DCUS_WEB_USE_POLLING)
    std::string nextTimeStr = json["config"]["polling"]["sleep"].string_value();
    int ms = Utils::getCurrentTimeSecForString(nextTimeStr, "%H:%M:%S") * 1000;
    setCheckTimerInterval(ms); // reset timer
#endif
    std::string path = json["_links"]["configData"]["href"].string_value();
    if (!path.empty()) {
        std::string errorString;
        json11::Json::object rootJson;
        rootJson["mode"] = json11::Json("merge");
        rootJson["id"] = json11::Json("");
        rootJson["time"] = json11::Json(Utils::getCurrentTimeString("%Y%m%dT%H%M%S"));
        lock();
        std::string attributesStr = dcus_server_engine->attributes().toStream();
        unlock();
        rootJson["data"] = json11::Json::parse(attributesStr, errorString);
        std::string contentStr;
        json11::Json(std::move(rootJson)).dump(contentStr);
        status = Core::putMessage(m_hpr->url, path, std::move(contentStr), dcus_server_config, m_hpr->token);
        if (status.state() != Core::SUCCEED) {
            LOG_WARNING("put json error");
            // postError(status.error());
            return true;
        }
    }
    path = json["_links"]["deploymentBase"]["href"].string_value();
    bool isUpdateAction = true;
    if (path.empty()) {
        isUpdateAction = false;
        path = json["_links"]["cancelAction"]["href"].string_value();
    }
    if (path.empty()) {
        return true;
    }
    status = Core::getMessage(m_hpr->url, path, body, dcus_server_config, m_hpr->token);
    if (status.state() != Core::SUCCEED) {
        LOG_WARNING("get deployment url error");
        // postError(802);
        return true;
    }
    if (body.empty()) {
        LOG_WARNING("get deployment body empty");
        // postError(803);
        return true;
    }
    if (isUpdateAction) {
        Upgrade upgrade;
        if (transformUpgrade(upgrade, body)) {
            postUpgrade(std::move(upgrade));
        } else {
            LOG_WARNING("transform upgrade error");
            return true;
        }
    } else {
        json = json11::Json::parse(body, jsonErrorStr);
        if (jsonErrorStr.empty() && !json.is_null()) {
            std::string cancelAction = json["cancelAction"]["stopId"].string_value();
            if (cancelAction.empty()) {
                LOG_WARNING("transform cancel error");
                return true;
            } else {
                postCancel(std::move(cancelAction));
            }
        } else {
            LOG_WARNING("transform json error, error=", jsonErrorStr);
            return true;
        }
    }
    return true;
}

bool HawkbitQueue::feedback(const WebFeed& webFeed)
{
    if (webFeed.id().empty()) {
        LOG_WARNING("webFeed id is empty");
        postError(805);
        return false;
    }
    if (webFeed.type() == WebFeed::TP_UNKNOWN || webFeed.execution() == WebFeed::EXE_UNKNOWN || webFeed.result() == WebFeed::RS_UNKNOWN) {
        LOG_WARNING("feedback data error");
        postError(806);
        return false;
    }
    const std::string& id = webFeed.id();
    const WebFeed::Details& details = webFeed.details();
    const WebFeed::Progress& progress = webFeed.progress();
    std::string execution = WebFeed::getExecutionStr(webFeed.execution());
    std::string result = WebFeed::getResultStr(webFeed.result());
    std::string urlStr;
    if (webFeed.type() == WebFeed::TP_DEPLOY) {
        urlStr = "/deploymentBase/" + id + "/feedback";
    } else if (webFeed.type() == WebFeed::TP_CANCEL) {
        urlStr = "/cancelAction/" + id + "/feedback";
    } else {
        LOG_WARNING("feedback unknown type");
        postError(807);
        return false;
    }
    json11::Json::object rootJson;
    rootJson["id"] = json11::Json(id);
    rootJson["time"] = json11::Json(Utils::getCurrentTimeString("%Y%m%dT%H%M%S"));
    json11::Json::object statusJson;
    statusJson["execution"] = json11::Json(std::move(execution));
    statusJson["details"] = json11::Json(details);
    json11::Json::object resultJson;
    resultJson["finished"] = json11::Json(std::move(result));
    if (webFeed.type() == WebFeed::TP_DEPLOY) {
        json11::Json::object progressJson;
        progressJson["of"] = progress.first;
        progressJson["cnt"] = progress.second;
        resultJson["progress"] = json11::Json(std::move(progressJson));
    }
    statusJson["result"] = json11::Json(std::move(resultJson));
    rootJson["status"] = json11::Json(std::move(statusJson));
    //
    std::string contentStr;
    json11::Json(std::move(rootJson)).dump(contentStr);
    Core::Status status = Core::postMessage(m_hpr->url, m_hpr->path + urlStr, std::move(contentStr), dcus_server_config, m_hpr->token);
    if (status.state() != Core::SUCCEED) {
        postError(status.error());
        return false;
    }
    return true;
}

bool HawkbitQueue::transformUpgrade(Upgrade& upgrade, const std::string& jsonString)
{
    std::string targetLocalUrl = this->localUrl();
    upgrade.packages().clear();
    upgrade.packages().shrink_to_fit();
    std::string jsonErrorStr;
    json11::Json json = json11::Json::parse(jsonString, jsonErrorStr);
    if (!jsonErrorStr.empty()) {
        return false;
    }
    if (json["id"].string_value().empty()) {
        return false;
    }
    upgrade.id() = json["id"].string_value();
    if (json["deployment"]["download"].string_value() == "attempt") {
        upgrade.download() = Upgrade::MTHD_ATTEMPT;
    } else if (json["deployment"]["download"].string_value() == "forced") {
        upgrade.download() = Upgrade::MTHD_FORCED;
    } else {
        upgrade.download() = Upgrade::MTHD_SKIP;
    }
    if (json["deployment"]["update"].string_value() == "attempt") {
        upgrade.deploy() = Upgrade::MTHD_ATTEMPT;
    } else if (json["deployment"]["update"].string_value() == "forced") {
        upgrade.deploy() = Upgrade::MTHD_FORCED;
    } else {
        upgrade.deploy() = Upgrade::MTHD_SKIP;
    }
    if (json["deployment"]["maintenanceWindow"].string_value() == "available") {
        upgrade.maintenance() = true;
    } else {
        upgrade.maintenance() = false;
    }
    for (const json11::Json& packageJson : json["deployment"]["chunks"].array_items()) {
        Package package;
        package.domain() = packageJson["name"].string_value();
        package.part() = packageJson["part"].string_value();
        package.version() = packageJson["version"].string_value();
        Data metaData;
        for (const json11::Json& metaDataJson : packageJson["metadata"].array_items()) {
            if (metaDataJson["key"].is_null() || metaDataJson["value"].is_null()) {
                continue;
            }
            if (metaDataJson["value"].is_bool()) {
                metaData.add(metaDataJson["key"].string_value(), metaDataJson["value"].bool_value());
            } else if (metaDataJson["value"].is_number()) {
                metaData.add(metaDataJson["key"].string_value(), metaDataJson["value"].number_value());
            } else if (metaDataJson["value"].is_string()) {
                metaData.add(metaDataJson["key"].string_value(), metaDataJson["value"].string_value());
            }
        }
        package.meta() = metaData;
        for (const json11::Json& fileJson : packageJson["artifacts"].array_items()) {
            File file;
            file.domain() = package.domain();
            file.name() = fileJson["filename"].string_value();
            if (targetLocalUrl.empty()) {
                file.url() = "";
            } else {
                file.url() = targetLocalUrl + "/" + package.domain() + "/" + file.name();
            }
            file.md5() = fileJson["hashes"]["md5"].string_value();
            // file.sha1() = fileJson["hashes"]["sha1"].string_value();
            file.sha256() = fileJson["hashes"]["sha256"].string_value();
            file.size() = fileJson["size"].int_value();
            if (!fileJson["_links"]["download"]["href"].string_value().empty()) {
                file.web_url() = fileJson["_links"]["download"]["href"].string_value();
            } else {
                file.web_url() = fileJson["_links"]["download-http"]["href"].string_value();
            }
            package.files().push_back(std::move(file));
        }
        upgrade.packages().push_back(std::move(package));
    }
    return true;
}

DCUS_NAMESPACE_END
