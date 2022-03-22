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

#include "string.h"
#include <iomanip>
#include <sstream>
#include <thread>

DCUS_NAMESPACE_BEGIN

namespace Common {

void stringReplace(std::string& strBase, const std::string& strSrc, const std::string& strDes) noexcept
{
    std::string::size_type pos = 0;
    std::string::size_type srcLen = strSrc.size();
    std::string::size_type desLen = strDes.size();
    pos = strBase.find(strSrc, pos);
    while ((pos != std::string::npos)) {
        strBase.replace(pos, srcLen, strDes);
        pos = strBase.find(strSrc, (pos + desLen));
    }
}

void trimString(std::string& str) noexcept
{
    if (!str.empty()) {
        int s = (int)str.find_first_not_of(" ");
        int e = (int)str.find_last_not_of(" ");
        str = str.substr(s, e - s + 1);
    }
}

std::vector<std::string> stringSplit(const std::string& s, const std::string& delim) noexcept
{
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0)
        return elems;
    while (pos < len) {
        int find_pos = (int)s.find(delim, pos);
        if (find_pos < 0) {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

int64_t stringToInt64(const std::string& s) noexcept
{
    std::stringstream ss;
    ss << s;
    int64_t value = 0;
    ss >> value;
    return value;
}

bool stringEndWith(const std::string& str, const std::string& tail) noexcept
{
    if (str.empty()) {
        return false;
    }
    return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

bool stringStartWith(const std::string& str, const std::string& head) noexcept
{
    if (str.empty()) {
        return false;
    }
    return str.compare(0, head.size(), head) == 0;
}

std::pair<std::string, std::string> getIpaddrMethod(const std::string& url) noexcept
{
    std::pair<std::string, std::string> pair;
    if (url.empty()) {
        return pair;
    }
    std::string topStr;
    const std::string httpStr = "http://";
    const std::string httpsStr = "https://";
    std::string tmpUrl = url;
    if (stringStartWith(tmpUrl, httpStr)) {
        tmpUrl = tmpUrl.substr(httpStr.size(), tmpUrl.size() - httpStr.size());
        topStr = httpStr;
    } else if (stringStartWith(tmpUrl, httpsStr)) {
        tmpUrl = tmpUrl.substr(httpsStr.size(), tmpUrl.size() - httpsStr.size());
        topStr = httpsStr;
    }
    int pos = (int)tmpUrl.find("/");
    if (pos < 0) {
        return pair;
    }
    pair.first = topStr + tmpUrl.substr(0, pos);
    pair.second = tmpUrl.substr(pos, tmpUrl.size());
    return pair;
}

std::string doubleToString(const double& value, int f) noexcept
{
    std::stringstream ss;
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(f) << value;
    std::string str = ss.str();
    return str;
}

int getNumForString(const std::string& numStr, bool* ok) noexcept
{
    int targetNum = -1;
    try {
        targetNum = std::stoi(numStr);
        if (ok) {
            *ok = true;
        }
    } catch (...) {
        if (ok) {
            *ok = false;
        }
        targetNum = -1;
    }
    return targetNum;
}

}
DCUS_NAMESPACE_END
