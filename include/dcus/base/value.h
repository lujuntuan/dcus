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

#ifndef DCUS_VALUE_H
#define DCUS_VALUE_H

#include "dcus/base/define.h"
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct DCUS_EXPORT Value final {
public:
    using IntList = std::vector<int>;
    using DoubleList = std::vector<double>;
    using StringList = std::vector<std::string>;
    enum Type {
        VALUE_TYPE_INVALID = 0,
        VALUE_TYPE_BOOL,
        VALUE_TYPE_INT,
        VALUE_TYPE_DOUBLE,
        VALUE_TYPE_STRING,
        VALUE_TYPE_INT_LIST,
        VALUE_TYPE_DOUBLE_LIST,
        VALUE_TYPE_STRING_LIST,
    };
    Value() noexcept = default;
    Value(Value&& value) noexcept;
    Value(const Value& value) noexcept;
    Value(bool value) noexcept;
    Value(int value) noexcept;
    Value(double value) noexcept;
    Value(const char* value) noexcept;
    Value(const std::string& value) noexcept;
    Value(const IntList& value) noexcept;
    Value(const DoubleList& value) noexcept;
    Value(const StringList& value) noexcept;
    Value(const std::initializer_list<int>& data) noexcept;
    Value(const std::initializer_list<double>& data) noexcept;
    Value(const std::initializer_list<std::string>& data) noexcept;
    ~Value() noexcept;
    void copy(const Value& value) noexcept;
    void clear() noexcept;
    void swap(Value&& value) noexcept;
    bool equal(const Value& value) const noexcept;
    inline Type type() const noexcept
    {
        return m_type;
    }
    inline size_t size() const noexcept
    {
        return m_size;
    }
    inline void* data() const noexcept
    {
        return m_data;
    }
    inline bool valid() const noexcept
    {
        return m_type != VALUE_TYPE_INVALID;
    }
    bool toBool() const noexcept;
    int toInt() const noexcept;
    double toDouble() const noexcept;
    std::string toString() const noexcept;
    const char* toStringCStr() const noexcept;
    IntList toIntList() const noexcept;
    DoubleList toDoubleList() const noexcept;
    StringList toStringList() const noexcept;
    Value& operator=(const Value& value) noexcept;
    Value& operator=(Value&& value) noexcept;
    bool operator==(const Value& value) const noexcept;
    bool operator!=(const Value& value) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Value& value) noexcept;

private:
    void* m_data = nullptr;
    Type m_type = VALUE_TYPE_INVALID;
    size_t m_size = 0;
};

DCUS_NAMESPACE_END

#endif // DCUS_VALUE_H
