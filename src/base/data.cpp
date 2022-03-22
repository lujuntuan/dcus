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

#include "dcus/base/data.h"
#include "dcus/base/log.h"
#include "importlib/json11.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

DCUS_NAMESPACE_BEGIN

Data::Data(const std::initializer_list<std::pair<std::string, Value>>& data) noexcept
{
    for (auto i = data.begin(); i != data.end(); i++) {
        m_map.emplace(*i);
    }
}

Data::Data(Data&& data) noexcept
{
    m_map.swap(data.m_map);
}

Data::Data(const Data& data) noexcept
{
    std::copy(data.m_map.begin(), data.m_map.end(), std::inserter(m_map, m_map.begin()));
}

void Data::add(const std::string& key, const Value& value) noexcept
{
    auto it = m_map.find(key);
    if (it != m_map.end()) {
        m_map.erase(key);
    }
    m_map.emplace(key, value);
}

void Data::add(int key, const Value& value) noexcept
{
    add(std::to_string(key), value);
}

void Data::sub(const std::string& key) noexcept
{
    m_map.erase(key);
}

bool Data::contains(const std::string& key) noexcept
{
    auto it = m_map.find(key);
    if (it == m_map.end()) {
        return false;
    }
    return true;
}

void Data::clear() noexcept
{
    std::map<std::string, Value> tmp;
    m_map.clear();
    m_map.swap(tmp);
}

void Data::swap(Data&& data) noexcept
{
    m_map.swap(data.m_map);
}

const Value Data::value(const std::string& key, const Value& defaultValue) const noexcept
{
    try {
        return m_map.at(key);
    } catch (...) {
        return defaultValue;
    }
}

const Value Data::value(int key, const Value& defaultValue) const noexcept
{
    try {
        return m_map.at(std::to_string(key));
    } catch (...) {
        return defaultValue;
    }
}

Value& Data::operator[](const std::string& key) noexcept
{
    return m_map[key];
}

const Value Data::operator[](const std::string& key) const noexcept
{
    try {
        return m_map.at(key);
    } catch (...) {
        return Value();
    }
}

Data& Data::operator<<(const std::pair<std::string, Value>& pair) noexcept
{
    m_map.insert(pair);
    return *this;
}

Data& Data::operator=(Data&& data) noexcept
{
    m_map.swap(data.m_map);
    return *this;
}

Data& Data::operator=(const Data& data) noexcept
{
    std::copy(data.m_map.begin(), data.m_map.end(), std::inserter(m_map, m_map.begin()));
    return *this;
}

bool Data::operator==(const Data& data) const noexcept
{
    return m_map == data.m_map;
}

bool Data::operator!=(const Data& data) const noexcept
{
    return m_map != data.m_map;
}

inline bool _isIntType(double value) noexcept
{
    return std::fmod(value, static_cast<decltype(value)>(1.0)) == 0.0;
}

Data Data::readStream(const std::string& jsonStr, bool printErrorLog) noexcept
{
    Data data;
    if (jsonStr.empty()) {
        if (printErrorLog) {
            LOG_WARNING("read error");
        }
        return data;
    }
    std::string errorStr;
    const json11::Json& json = json11::Json::parse(jsonStr, errorStr);
    if (json.type() == json11::Json::NUL || !errorStr.empty()) {
        if (printErrorLog) {
            LOG_WARNING("read error");
        }
    }
    if (!json.is_object()) {
        if (printErrorLog) {
            LOG_WARNING("open error");
        }
        return data;
    }
    for (const auto& pair : json.object_items()) {
        const std::string& key = pair.first;
        const json11::Json& value = pair.second;
        if (value.type() == json11::Json::NUL) {
            data.add(key, Value());
        } else if (value.type() == json11::Json::NUMBER) {
            double fv = value.number_value();
            if (_isIntType(fv)) {
                data.add(key, (int)fv);
            } else {
                data.add(key, fv);
            }
        } else if (value.type() == json11::Json::BOOL) {
            data.add(key, value.bool_value());
        } else if (value.type() == json11::Json::STRING) {
            data.add(key, value.string_value());
        } else if (value.type() == json11::Json::ARRAY) {
            std::vector<int> intVector;
            std::vector<double> doubleVector;
            std::vector<std::string> stringVector;
            for (const auto& v : value.array_items()) {
                if (v.type() == json11::Json::BOOL) {
                    intVector.push_back(v.bool_value());
                } else if (v.type() == json11::Json::NUMBER) {
                    double fv = v.number_value();
                    if (_isIntType(fv)) {
                        intVector.push_back((int)fv);
                    } else {
                        doubleVector.push_back(fv);
                    }
                } else if (v.type() == json11::Json::STRING) {
                    stringVector.push_back(v.string_value());
                }
            }
            if (!intVector.empty()) {
                data.add(key, std::move(intVector));
            } else if (!doubleVector.empty()) {
                data.add(key, std::move(doubleVector));
            } else if (!stringVector.empty()) {
                data.add(key, std::move(stringVector));
            }
        } else if (value.type() == json11::Json::OBJECT) {
            data.add(key, Value("{}"));
        }
    }
    return data;
}

Data Data::read(const std::string& filePath) noexcept
{
    std::ifstream rfile;
    rfile.open(filePath, std::ios::in);
    if (!rfile.is_open()) {
        return Data();
    }
    std::stringstream buffer;
    buffer << rfile.rdbuf();
    rfile.close();
    return readStream(buffer.str(), true);
}

bool Data::saveStream(std::string& jsonStr, const Data& data) noexcept
{
    json11::Json::object jsobj;
    for (const auto& pair : data) {
        const std::string& key = pair.first;
        const Value& value = pair.second;
        if (value.type() == Value::VALUE_TYPE_INVALID) {
            jsobj.emplace(key, json11::Json());
        } else if (value.type() == Value::VALUE_TYPE_BOOL) {
            jsobj.emplace(key, json11::Json(value.toBool()));
        } else if (value.type() == Value::VALUE_TYPE_INT) {
            jsobj.emplace(key, json11::Json(value.toInt()));
        } else if (value.type() == Value::VALUE_TYPE_DOUBLE) {
            jsobj.emplace(key, json11::Json(value.toDouble()));
        } else if (value.type() == Value::VALUE_TYPE_STRING) {
            jsobj.emplace(key, json11::Json(value.toString()));
        } else if (value.type() == Value::VALUE_TYPE_INT_LIST) {
            jsobj.emplace(key, value.toIntList());
        } else if (value.type() == Value::VALUE_TYPE_DOUBLE_LIST) {
            jsobj.emplace(key, value.toDoubleList());
        } else if (value.type() == Value::VALUE_TYPE_STRING_LIST) {
            jsobj.emplace(key, value.toStringList());
        }
    }
    json11::Json json(jsobj);
    jsonStr = json.dump();
    if (jsonStr.empty()) {
        return false;
    }
    return true;
}

bool Data::save(const std::string& filePath, const Data& data) noexcept
{
    std::string jsonStr;
    if (!saveStream(jsonStr, data)) {
        return false;
    }
    std::ofstream wfile;
    wfile.open(filePath, std::ios::out | std::ios::trunc);
    if (!wfile.is_open()) {
        LOG_WARNING("open error");
        return false;
    }
    wfile << jsonStr;
    wfile.close();
    return true;
}

bool Data::save(const std::string& filePath) const noexcept
{
    return save(filePath, *this);
}

std::string Data::toStream() const noexcept
{
    std::string stream;
    saveStream(stream, *this);
    return stream;
}

std::ostream& operator<<(std::ostream& ostream, const Data& data) noexcept
{
    ostream << "{\n";
    int i = 0;
    for (const auto& pair : data.m_map) {
        if (i != 0) {
            ostream << "\n";
        }
        ostream << "  [" << pair.first << "]: " << pair.second;
        i++;
    }
    ostream << "\n}";
    return ostream;
}

DCUS_NAMESPACE_END
