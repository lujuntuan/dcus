/*********************************************************************************
 *Copyright(C): Juntuan.Lu 2022
 *Author:  Juntuan.Lu
 *Version: 1.0
 *Date:  2022/04/01
 *Phone: 15397182986
 *Description:
 *Others:
 *Function List:
 *History:
 **********************************************************************************/

#ifndef DCUS_VARIANT_H
#define DCUS_VARIANT_H

#include "dcus/base/define.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

class Value;
class VariantList;
class VariantMap;

class DCUS_EXPORT Variant final {
public:
    enum Type {
        TYPE_NULL = 0,
        TYPE_BOOL,
        TYPE_INT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_LIST,
        TYPE_MAP
    };
    enum ParserType {
        PARSER_UNKNOWN = 0,
        PARSER_OUT_FORMAT,
        PARSER_OUT_SMALL,
        PARSER_IN_STANDARD,
        PARSER_IN_COMMENTS
    };
    Variant() noexcept;
    Variant(std::nullptr_t value) noexcept;
    Variant(bool value) noexcept;
    Variant(int value) noexcept;
    Variant(double value) noexcept;
    Variant(const std::string& value) noexcept;
    Variant(std::string&& value) noexcept;
    Variant(const char* value) noexcept;
    Variant(const VariantList& values) noexcept;
    Variant(VariantList&& values) noexcept;
    Variant(const VariantMap& values) noexcept;
    Variant(VariantMap&& values) noexcept;
    Variant(const std::initializer_list<std::pair<std::string, Variant>>& values) noexcept;
    template <class LIST, typename std::enable_if<std::is_constructible<Variant, decltype(*std::declval<LIST>().begin())>::value, int>::type = 0>
    Variant(const LIST& list) noexcept
        : Variant(VariantList(list.begin(), list.end()))
    {
    }
    template <class MAP, typename std::enable_if<std::is_constructible<std::string, decltype(std::declval<MAP>().begin()->first)>::value && std::is_constructible<Variant, decltype(std::declval<MAP>().begin()->second)>::value, int>::type = 0>
    Variant(const MAP& map) noexcept
        : Variant(VariantMap(map.begin(), map.end()))
    {
    }
    Type type() const;
    bool isValid() const;
    bool isNull() const;
    bool isBool() const;
    bool isInt() const;
    bool isDouble() const;
    bool isNumber() const;
    bool isString() const;
    bool isList() const;
    bool isMap() const;
    bool toBool(bool defaultValue = false) const;
    int toInt(int defaultValue = 0) const;
    double toDouble(double defaultValue = 0) const;
    const std::string& toString(const std::string& defaultValue = std::string()) const;
    const char* toCString(const char* defaultValue = "") const;
    std::vector<std::string> toStringList() const;
    std::vector<int> toIntList() const;
    std::vector<double> toDoubleList() const;
    const VariantList& toList() const;
    const VariantMap& toMap() const;
    const Variant& listValue(int index, const Variant& defaultValue = Variant());
    const Variant& mapValue(const std::string& key, const Variant& defaultValue = Variant());
    const Variant& operator[](size_t i) const;
    const Variant& operator[](const std::string& key) const;
    bool operator==(const Variant& rhs) const;
    bool operator<(const Variant& rhs) const;
    bool operator!=(const Variant& rhs) const;
    bool operator<=(const Variant& rhs) const;
    bool operator>(const Variant& rhs) const;
    bool operator>=(const Variant& rhs) const;
    std::string toJson(ParserType parserType = PARSER_OUT_SMALL) const;
    bool saveJson(const std::string& filePath, ParserType parserType = PARSER_OUT_SMALL) const noexcept;
    static Variant fromJson(const std::string& json, std::string* errorString = nullptr, ParserType parserType = PARSER_IN_STANDARD);
    static Variant readJson(const std::string& filePath, std::string* errorString = nullptr, ParserType parserType = PARSER_IN_STANDARD) noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Variant& data) noexcept;

private:
    friend class VariantParser;
    std::shared_ptr<Value> m_ptr;
};

class DCUS_EXPORT VariantList : public std::vector<Variant> {
    using std::vector<Variant>::vector;

public:
    VariantList() = default;
    VariantList(const Variant& values) noexcept;
    VariantList(Variant&& values) noexcept;
    void add(const Variant& data);
    bool contains(const Variant& data) const;
    const Variant& value(int index, const Variant& defaultValue = Variant()) const;
    Variant toVariant() const;
    std::string toJson(Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const;
    bool saveJson(const std::string& filePath, Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const noexcept;
};

class DCUS_EXPORT VariantMap : public std::map<std::string, Variant> {
    using std::map<std::string, Variant>::map;

public:
    VariantMap() = default;
    VariantMap(const Variant& values) noexcept;
    VariantMap(Variant&& values) noexcept;
    void add(const std::string& key, const Variant& data);
    void add(int key, const Variant& data);
    void sub(const std::string& key);
    bool contains(const std::string& key) const;
    const Variant& value(const std::string& key, const Variant& defaultValue = Variant()) const;
    Variant toVariant() const;
    std::string toJson(Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const;
    bool saveJson(const std::string& filePath, Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const noexcept;
};

class DCUS_EXPORT Value {
protected:
    virtual ~Value() { }
    virtual void dump(std::string& json, int depth) const = 0;
    virtual Variant::Type type() const = 0;
    virtual bool isEqual(const Value* value) const = 0;
    virtual bool isLess(const Value* value) const = 0;
    virtual bool toBool() const;
    virtual int toInt() const;
    virtual double toDouble() const;
    virtual const std::string& toString() const;
    virtual const VariantList& toList() const;
    virtual const VariantMap& toMap() const;
    virtual const Variant& operator[](size_t i) const;
    virtual const Variant& operator[](const std::string& key) const;

private:
    friend class Variant;
    friend class VariantInt;
    friend class VariantDouble;
    friend class VariantParser;
};

DCUS_NAMESPACE_END

#endif // DCUS_VARIANT_H
