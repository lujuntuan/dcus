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
#include "dcus/server/server_engine.h"
#include "dcus/setting.h"
#include "dcus/utils/string.h"
#include "dcus/utils/time.h"

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
    const Variant& data = Variant::fromJson(body, &jsonErrorStr);
    if (!jsonErrorStr.empty() || data.isNull()) {
        LOG_WARNING("error=", jsonErrorStr);
        return false;
    }
    if (data.isNull()) {
        return false;
    }
    if (!data["_links"].isMap()) {
        postIdle();
        return true;
    }
#if (DCUS_WEB_USE_POLLING)
    std::string nextTimeStr = data["config"]["polling"]["sleep"].toString();
    int ms = Utils::getCurrentTimeSecForString(nextTimeStr, "%H:%M:%S") * 1000;
    setCheckTimerInterval(ms); // reset timer
#endif
    std::string path = data["_links"]["configData"]["href"].toString();
    if (!path.empty()) {
        std::string errorString;
        VariantMap rootData;
        rootData["mode"] = Variant("merge");
        rootData["id"] = Variant("");
        rootData["time"] = Variant(Utils::getCurrentTimeString("%Y%m%dT%H%M%S"));
        lock();
        std::string attributesStr = dcus_server_engine->attributes().toJson();
        unlock();
        rootData["data"] = Variant::fromJson(attributesStr, &errorString);
        std::string contentStr = Variant(std::move(rootData)).toJson();
        status = Core::putMessage(m_hpr->url, path, std::move(contentStr), dcus_server_config, m_hpr->token);
        if (status.state() != Core::SUCCEED) {
            LOG_WARNING("put json error");
            // postError(status.error());
            return true;
        }
    }
    path = data["_links"]["deploymentBase"]["href"].toString();
    bool isUpdateAction = true;
    if (path.empty()) {
        isUpdateAction = false;
        path = data["_links"]["cancelAction"]["href"].toString();
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
        const Variant& cancelData = Variant::fromJson(body, &jsonErrorStr);
        if (jsonErrorStr.empty() && !cancelData.isNull()) {
            std::string cancelAction = cancelData["cancelAction"]["stopId"].toString();
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
    VariantMap rootData;
    rootData["id"] = Variant(id);
    rootData["time"] = Variant(Utils::getCurrentTimeString("%Y%m%dT%H%M%S"));
    VariantMap statusData;
    statusData["execution"] = Variant(std::move(execution));
    statusData["details"] = Variant(details);
    VariantMap resultData;
    resultData["finished"] = Variant(std::move(result));
    if (webFeed.type() == WebFeed::TP_DEPLOY) {
        VariantMap progressJson;
        progressJson["of"] = progress.first;
        progressJson["cnt"] = progress.second;
        resultData["progress"] = Variant(std::move(progressJson));
    }
    statusData["result"] = Variant(std::move(resultData));
    rootData["status"] = Variant(std::move(statusData));
    //
    std::string contentStr = Variant(std::move(rootData)).toJson();
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
    Variant data = Variant::fromJson(jsonString, &jsonErrorStr);
    if (!jsonErrorStr.empty()) {
        return false;
    }
    if (data["id"].toString().empty()) {
        return false;
    }
    upgrade.id() = data["id"].toString();
    if (data["deployment"]["download"].toString() == "attempt") {
        upgrade.download() = Upgrade::MTHD_ATTEMPT;
    } else if (data["deployment"]["download"].toString() == "forced") {
        upgrade.download() = Upgrade::MTHD_FORCED;
    } else {
        upgrade.download() = Upgrade::MTHD_SKIP;
    }
    if (data["deployment"]["update"].toString() == "attempt") {
        upgrade.deploy() = Upgrade::MTHD_ATTEMPT;
    } else if (data["deployment"]["update"].toString() == "forced") {
        upgrade.deploy() = Upgrade::MTHD_FORCED;
    } else {
        upgrade.deploy() = Upgrade::MTHD_SKIP;
    }
    if (data["deployment"]["maintenanceWindow"].toString() == "available") {
        upgrade.maintenance() = true;
    } else {
        upgrade.maintenance() = false;
    }
    for (const auto& packageData : data["deployment"]["chunks"].toList()) {
        Package package;
        package.domain() = packageData["name"].toString();
        package.part() = packageData["part"].toString();
        package.version() = packageData["version"].toString();
        VariantMap metaData;
        for (const auto& subMetaData : packageData["metadata"].toList()) {
            if (subMetaData["key"].isNull() || subMetaData["value"].isNull()) {
                continue;
            }
            if (subMetaData["value"].isBool()) {
                metaData.insert(subMetaData["key"].toString(), subMetaData["value"].toBool());
            } else if (subMetaData["value"].isInt()) {
                metaData.insert(subMetaData["key"].toString(), subMetaData["value"].toInt());
            } else if (subMetaData["value"].isDouble()) {
                metaData.insert(subMetaData["key"].toString(), subMetaData["value"].toDouble());
            } else if (subMetaData["value"].isString()) {
                metaData.insert(subMetaData["key"].toString(), subMetaData["value"].toString());
            }
        }
        package.meta() = metaData;
        for (const auto& fileData : packageData["artifacts"].toList()) {
            File file;
            file.domain() = package.domain();
            file.name() = fileData["filename"].toString();
            if (targetLocalUrl.empty()) {
                file.url() = "";
            } else {
                file.url() = targetLocalUrl + "/" + package.domain() + "/" + file.name();
            }
            file.md5() = fileData["hashes"]["md5"].toString();
            // file.sha1() = fileData["hashes"]["sha1"].toString();
            file.sha256() = fileData["hashes"]["sha256"].toString();
            file.size() = fileData["size"].toInt();
            if (!fileData["_links"]["download"]["href"].toString().empty()) {
                file.web_url() = fileData["_links"]["download"]["href"].toString();
            } else {
                file.web_url() = fileData["_links"]["download-http"]["href"].toString();
            }
            package.files().push_back(std::move(file));
        }
        upgrade.packages().push_back(std::move(package));
    }
    return true;
}

DCUS_NAMESPACE_END
