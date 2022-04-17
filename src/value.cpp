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

#include "dcus/value.h"
#include <cstring>
#include <iostream>
#include <math.h>

DCUS_NAMESPACE_BEGIN

Value::Value(Value&& value) noexcept
{
    this->swap(std::move(value));
}

Value::Value(const Value& value) noexcept
{
    this->copy(value);
}

Value::Value(bool value) noexcept
{
    m_size = 0;
    m_data = malloc(sizeof(bool));
    memcpy(m_data, &value, sizeof(bool));
    m_type = VALUE_TYPE_BOOL;
}

Value::Value(int value) noexcept
{
    m_size = 0;
    m_data = malloc(sizeof(int));
    memcpy(m_data, &value, sizeof(int));
    m_type = VALUE_TYPE_INT;
}

Value::Value(double value) noexcept
{
    m_size = 0;
    m_data = malloc(sizeof(double));
    memcpy(m_data, &value, sizeof(double));
    m_type = VALUE_TYPE_DOUBLE;
}

Value::Value(const char* value) noexcept
{
    m_size = strlen(value);
    if (m_size > 0) {
        m_data = malloc((m_size + 1) * sizeof(char));
        memcpy(m_data, value, (m_size + 1) * sizeof(char));
    } else {
        m_data = nullptr;
    }
    m_type = VALUE_TYPE_STRING;
}

Value::Value(const std::string& value) noexcept
{
    m_size = value.size();
    if (m_size > 0) {
        m_data = malloc((m_size + 1) * sizeof(char));
        memcpy(m_data, value.data(), (m_size + 1) * sizeof(char));
    } else {
        m_data = nullptr;
    }
    m_type = VALUE_TYPE_STRING;
}

Value::Value(const IntList& value) noexcept
{
    m_size = value.size();
    if (m_size > 0) {
        m_data = malloc(m_size * sizeof(int));
        memcpy(m_data, value.data(), m_size * sizeof(int));
    } else {
        m_data = nullptr;
    }
    m_type = VALUE_TYPE_INT_LIST;
}

Value::Value(const DoubleList& value) noexcept
{
    m_size = value.size();
    if (m_size > 0) {
        m_data = malloc(m_size * sizeof(double));
        memcpy(m_data, value.data(), m_size * sizeof(double));
    } else {
        m_data = nullptr;
    }
    m_type = VALUE_TYPE_DOUBLE_LIST;
}

Value::Value(const StringList& value) noexcept
{
    m_size = value.size();
    if (m_size > 0) {
        m_data = malloc(m_size * sizeof(char*));
        for (size_t i = 0; i < m_size; i++) {
            size_t psize = strlen(value.at(i).data()) + 1;
            ((char**)m_data)[i] = (char*)malloc(psize * sizeof(char));
            memcpy(((char**)m_data)[i], value[i].data(), psize * sizeof(char));
        }
    } else {
        m_data = nullptr;
    }
    m_type = VALUE_TYPE_STRING_LIST;
}

Value::Value(const std::initializer_list<int>& data) noexcept
    : Value((IntList)std::move(data))
{
}

Value::Value(const std::initializer_list<double>& data) noexcept
    : Value((DoubleList)std::move(data))
{
}

Value::Value(const std::initializer_list<std::string>& data) noexcept
    : Value((StringList)std::move(data))
{
}

Value::~Value() noexcept
{
    clear();
}

void Value::copy(const Value& value) noexcept
{
    clear();
    m_type = value.type();
    m_size = value.size();
    if (value.data() == nullptr) {
        return;
    }
    switch (m_type) {
    case VALUE_TYPE_INVALID:
        m_data = nullptr;
        return;
    case VALUE_TYPE_BOOL:
        m_data = malloc(sizeof(bool));
        memcpy(m_data, value.data(), sizeof(bool));
        return;
    case VALUE_TYPE_INT:
        m_data = malloc(sizeof(int));
        memcpy(m_data, value.data(), sizeof(int));
        return;
    case VALUE_TYPE_DOUBLE:
        m_data = malloc(sizeof(double));
        memcpy(m_data, value.data(), sizeof(double));
        return;
    case VALUE_TYPE_STRING:
        m_data = malloc((m_size + 1) * sizeof(char));
        memcpy(m_data, value.data(), (m_size + 1) * sizeof(char));
        return;
    case VALUE_TYPE_INT_LIST:
        m_data = malloc(m_size * sizeof(int));
        memcpy(m_data, value.data(), m_size * sizeof(int));
        return;
    case VALUE_TYPE_DOUBLE_LIST:
        m_data = malloc(m_size * sizeof(double));
        memcpy(m_data, value.data(), m_size * sizeof(double));
        return;
    case VALUE_TYPE_STRING_LIST:
        m_data = malloc(m_size * sizeof(char*));
        for (size_t i = 0; i < m_size; i++) {
            char* s = ((char**)value.data())[i];
            size_t psize = strlen(s) + 1;
            ((char**)m_data)[i] = (char*)malloc(psize * sizeof(char));
            memcpy(((char**)m_data)[i], s, psize * sizeof(char));
        }
        return;
    default:
        return;
    }
}

void Value::clear() noexcept
{
    m_type = VALUE_TYPE_INVALID;
    m_size = 0;
    if (m_data == nullptr) {
        return;
    }
    if (m_type == VALUE_TYPE_STRING_LIST) {
        for (size_t i = 0; i < m_size; i++) {
            free(((char**)m_data)[i]);
        }
    }
    free(m_data);
    m_data = nullptr;
}

void Value::swap(Value&& value) noexcept
{
    m_type = value.m_type;
    m_size = value.m_size;
    m_data = value.m_data;
    value.m_data = nullptr;
}

bool Value::equal(const Value& value) const noexcept
{
    if (m_type != value.type()) {
        return false;
    }
    switch (m_type) {
    case VALUE_TYPE_INVALID:
        return false;
    case VALUE_TYPE_BOOL:
        return toBool() == value.toBool();
    case VALUE_TYPE_INT:
        return toInt() == value.toInt();
    case VALUE_TYPE_DOUBLE:
        return toDouble() == value.toDouble();
    case VALUE_TYPE_STRING:
        return toString() == value.toString();
    case VALUE_TYPE_INT_LIST:
        return toIntList() == value.toIntList();
    case VALUE_TYPE_DOUBLE_LIST:
        return toDoubleList() == value.toDoubleList();
    case VALUE_TYPE_STRING_LIST:
        return toStringList() == value.toStringList();
    default:
        return false;
    }
    return false;
}

bool Value::toBool() const noexcept
{
    if (!m_data) {
        return false;
    }
    if (m_type == VALUE_TYPE_BOOL) {
        return (bool)*((bool*)m_data);
    } else if (m_type == VALUE_TYPE_INT) {
        return (bool)*((int*)m_data);
    } else if (m_type == VALUE_TYPE_DOUBLE) {
        return (bool)*((double*)m_data);
    }
    return false;
}

int Value::toInt() const noexcept
{
    if (!m_data) {
        return -1;
    }
    if (m_type == VALUE_TYPE_BOOL) {
        return (int)*((bool*)m_data);
    } else if (m_type == VALUE_TYPE_INT) {
        return (int)*((int*)m_data);
    } else if (m_type == VALUE_TYPE_DOUBLE) {
        return (int)*((double*)m_data);
    }
    return -1;
}

double Value::toDouble() const noexcept
{
    if (!m_data) {
        return -1;
    }
    if (m_type == VALUE_TYPE_BOOL) {
        return (double)*((bool*)m_data);
    } else if (m_type == VALUE_TYPE_INT) {
        return (double)*((int*)m_data);
    } else if (m_type == VALUE_TYPE_DOUBLE) {
        return (double)*((double*)m_data);
    }
    return -1;
}

std::string Value::toString() const noexcept
{
    if (m_type == VALUE_TYPE_STRING && m_data) {
        return std::string((char*)m_data, m_size);
    }
    return std::string();
}

const char* Value::toStringCStr() const noexcept
{
    if (m_type == VALUE_TYPE_STRING && m_data) {
        return (const char*)m_data;
    }
    return "";
}

Value::IntList Value::toIntList() const noexcept
{
    if (m_type == VALUE_TYPE_INT_LIST && m_data) {
        return IntList((int*)m_data, (int*)m_data + m_size);
    }
    return IntList();
}

Value::DoubleList Value::toDoubleList() const noexcept
{
    if (m_type == VALUE_TYPE_DOUBLE_LIST && m_data) {
        return DoubleList((double*)m_data, (double*)m_data + m_size);
    }
    return DoubleList();
}

Value::StringList Value::toStringList() const noexcept
{
    if (m_type == VALUE_TYPE_STRING_LIST && m_data) {
        return StringList((char**)m_data, (char**)m_data + m_size);
    }
    return StringList();
}

Value& Value::operator=(const Value& value) noexcept
{
    this->copy(value);
    return *this;
}

Value& Value::operator=(Value&& value) noexcept
{
    this->swap(std::move(value));
    return *this;
}

bool Value::operator==(const Value& value) const noexcept
{
    return this->equal(value);
}

bool Value::operator!=(const Value& value) const noexcept
{
    return !this->equal(value);
}

template <typename T>
inline void ValueOstream(std::ostream& ostream, const T& value) noexcept
{
    int i = 0;
    for (const auto& v : value) {
        if (i != 0) {
            ostream << ", ";
        }
        ostream << v;
        i++;
    }
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) noexcept
{
    switch (value.type()) {
    case Value::VALUE_TYPE_INVALID:
        ostream << "[unknown type]";
        break;
    case Value::VALUE_TYPE_BOOL:
        ostream << value.toBool();
        break;
    case Value::VALUE_TYPE_INT:
        ostream << value.toInt();
        break;
    case Value::VALUE_TYPE_DOUBLE:
        ostream << value.toDouble();
        break;
    case Value::VALUE_TYPE_STRING:
        ostream << "\"" + value.toString() + "\"";
        break;
    case Value::VALUE_TYPE_INT_LIST:
        ValueOstream<Value::IntList>(ostream, value.toIntList());
        break;
    case Value::VALUE_TYPE_DOUBLE_LIST:
        ValueOstream<Value::DoubleList>(ostream, value.toDoubleList());
        break;
    case Value::VALUE_TYPE_STRING_LIST:
        ValueOstream<Value::StringList>(ostream, value.toStringList());
        break;
    default:
        break;
    }
    return ostream;
}

DCUS_NAMESPACE_END
