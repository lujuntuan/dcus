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

class VariantValue;
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
    Type type() const noexcept;
    bool isValid() const noexcept;
    bool isNull() const noexcept;
    bool isBool() const noexcept;
    bool isInt() const noexcept;
    bool isDouble() const noexcept;
    bool isNumber() const noexcept;
    bool isString() const noexcept;
    bool isList() const noexcept;
    bool isMap() const noexcept;
    bool toBool(bool defaultValue = false) const noexcept;
    int toInt(int defaultValue = 0) const noexcept;
    double toDouble(double defaultValue = 0) const noexcept;
    const std::string& toString(const std::string& defaultValue = std::string()) const noexcept;
    const char* toCString(const char* defaultValue = "") const noexcept;
    std::vector<int> toIntList() const noexcept;
    std::vector<double> toDoubleList() const noexcept;
    std::vector<std::string> toStringList() const noexcept;
    const VariantList& toList() const noexcept;
    const VariantMap& toMap() const noexcept;
    const Variant& listValue(int index, const Variant& defaultValue = Variant()) noexcept;
    const Variant& mapValue(const std::string& key, const Variant& defaultValue = Variant()) noexcept;
    const Variant& operator[](size_t i) const noexcept;
    const Variant& operator[](const std::string& key) const noexcept;
    bool operator==(const Variant& rhs) const noexcept;
    bool operator<(const Variant& rhs) const noexcept;
    bool operator!=(const Variant& rhs) const noexcept;
    bool operator<=(const Variant& rhs) const noexcept;
    bool operator>(const Variant& rhs) const noexcept;
    bool operator>=(const Variant& rhs) const noexcept;
    std::string toJson(ParserType parserType = PARSER_OUT_SMALL) const noexcept;
    bool saveJson(const std::string& filePath, ParserType parserType = PARSER_OUT_SMALL) const noexcept;
    static Variant fromJson(const std::string& json, std::string* errorString = nullptr, ParserType parserType = PARSER_IN_STANDARD) noexcept;
    static Variant readJson(const std::string& filePath, std::string* errorString = nullptr, ParserType parserType = PARSER_IN_STANDARD) noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Variant& data) noexcept;

private:
    friend class VariantParser;
    std::shared_ptr<VariantValue> m_ptr;
};

class DCUS_EXPORT VariantList : public std::vector<Variant> {
    using std::vector<Variant>::vector;

public:
    VariantList() noexcept = default;
    VariantList(const Variant& values) noexcept;
    VariantList(Variant&& values) noexcept;
    template <class LIST>
    VariantList(LIST&& list) noexcept
        : VariantList(std::forward<LIST>(list))
    {
    }
    void add(const Variant& data) noexcept;
    bool contains(const Variant& data) const noexcept;
    const Variant& value(int index, const Variant& defaultValue = Variant()) const noexcept;
    Variant toVariant() const noexcept;
    std::string toJson(Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const noexcept;
    bool saveJson(const std::string& filePath, Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const VariantList& data) noexcept;
};

class DCUS_EXPORT VariantMap : public std::map<std::string, Variant> {
    using std::map<std::string, Variant>::map;

public:
    VariantMap() noexcept = default;
    VariantMap(const Variant& values) noexcept;
    VariantMap(Variant&& values) noexcept;
    template <class MAP>
    VariantMap(MAP&& map) noexcept
        : Variant(std::forward<MAP>(map))
    {
    }
    void add(const std::string& key, const Variant& data) noexcept;
    void add(int key, const Variant& data) noexcept;
    void sub(const std::string& key) noexcept;
    bool contains(const std::string& key) const noexcept;
    const Variant& value(const std::string& key, const Variant& defaultValue = Variant()) const noexcept;
    Variant toVariant() const noexcept;
    std::string toJson(Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const noexcept;
    bool saveJson(const std::string& filePath, Variant::ParserType parserType = Variant::PARSER_OUT_SMALL) const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const VariantMap& data) noexcept;
};

class DCUS_EXPORT VariantValue {
protected:
    virtual ~VariantValue() noexcept { }
    virtual void dump(std::string& json, int depth) const noexcept = 0;
    virtual Variant::Type type() const noexcept = 0;
    virtual bool isEqual(const VariantValue* value) const noexcept = 0;
    virtual bool isLess(const VariantValue* value) const noexcept = 0;
    virtual bool toBool() const noexcept;
    virtual int toInt() const noexcept;
    virtual double toDouble() const noexcept;
    virtual const std::string& toString() const noexcept;
    virtual const VariantList& toList() const noexcept;
    virtual const VariantMap& toMap() const noexcept;
    virtual const Variant& operator[](size_t i) const noexcept;
    virtual const Variant& operator[](const std::string& key) const noexcept;

private:
    friend class Variant;
    friend class VariantInt;
    friend class VariantDouble;
    friend class VariantParser;
};

DCUS_NAMESPACE_END

#endif // DCUS_VARIANT_H
