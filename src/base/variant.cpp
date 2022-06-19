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

#include "dcus/base/variant.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory.h>
#include <sstream>

#define VAR_PARSE_MAX_DEPTH 500
#define VAR_PARSE_OUT_FORMAT_SPACE "    "
#define VAR_DOUBLE_PRECISION (double)0.0000001

DCUS_NAMESPACE_BEGIN

struct VariantNullPtr {
    bool operator==(VariantNullPtr) const
    {
        return true;
    }
    bool operator<(VariantNullPtr) const
    {
        return false;
    }
};

class VariantParser {
public:
    explicit VariantParser(const std::string& str, std::string* err, Variant::ParseType type) noexcept
        : inString(str)
        , errorString(err)
        , parseType(type)
    {
    }

private:
    friend class Variant;
    size_t index = 0;
    bool isFailed = false;
    const std::string& inString;
    std::string* errorString = nullptr;
    const Variant::ParseType parseType = Variant::PARSE_UNKNOWN;

public:
    static inline void parseOutBeautify(std::string& out, int depth) noexcept
    {
        if (depth < 0) {
            return;
        }
        out += "\n";
        for (size_t i = 0; i < depth; i++) {
            out += VAR_PARSE_OUT_FORMAT_SPACE;
        }
    }
    static void parseOut(VariantNullPtr, std::string& out, int depth) noexcept
    {
        out += "null";
    }
    static void parseOut(double value, std::string& out, int depth) noexcept
    {
        if (std::isfinite(value)) {
            char buf[32];
            snprintf(buf, sizeof buf, "%.17g", value);
            out += buf;
        } else {
            out += "null";
        }
    }
    static void parseOut(int value, std::string& out, int depth) noexcept
    {
        char buf[32];
        snprintf(buf, sizeof buf, "%d", value);
        out += buf;
    }
    static void parseOut(bool value, std::string& out, int depth) noexcept
    {
        out += value ? "true" : "false";
    }
    static void parseOut(const std::string& value, std::string& out, int depth) noexcept
    {
        out += '"';
        for (size_t i = 0; i < value.length(); i++) {
            const char ch = value[i];
            if (ch == '\\') {
                out += "\\\\";
            } else if (ch == '"') {
                out += "\\\"";
            } else if (ch == '\b') {
                out += "\\b";
            } else if (ch == '\f') {
                out += "\\f";
            } else if (ch == '\n') {
                out += "\\n";
            } else if (ch == '\r') {
                out += "\\r";
            } else if (ch == '\t') {
                out += "\\t";
            } else if (static_cast<uint8_t>(ch) <= 0x1f) {
                char buf[8];
                snprintf(buf, sizeof buf, "\\u%04x", ch);
                out += buf;
            } else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(value[i + 1]) == 0x80
                && static_cast<uint8_t>(value[i + 2]) == 0xa8) {
                out += "\\u2028";
                i += 2;
            } else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(value[i + 1]) == 0x80
                && static_cast<uint8_t>(value[i + 2]) == 0xa9) {
                out += "\\u2029";
                i += 2;
            } else {
                out += ch;
            }
        }
        out += '"';
    }
    static void parseOut(const VariantList& values, std::string& out, int depth) noexcept
    {
        if (depth >= 0) {
            if (depth > VAR_PARSE_MAX_DEPTH) {
                out.clear();
                return;
            }
            depth++;
        }
        bool first = true;
        out += "[";
        if (!values.empty()) {
            for (const auto& value : values) {
                if (!first) {
                    out += ", ";
                }
                parseOutBeautify(out, depth);
                value.m_ptr->parseOut(out, depth);
                first = false;
            }
            parseOutBeautify(out, depth - 1);
        }
        out += "]";
    }
    static void parseOut(const VariantMap& values, std::string& out, int depth) noexcept
    {
        if (depth >= 0) {
            if (depth > VAR_PARSE_MAX_DEPTH) {
                out.clear();
                return;
            }
            depth++;
        }
        bool first = true;
        out += "{";
        if (!values.empty()) {
            for (const auto& kv : values) {
                if (!first) {
                    out += ", ";
                }
                parseOutBeautify(out, depth);
                parseOut(kv.first, out, depth);
                out += ": ";
                kv.second.m_ptr->parseOut(out, depth);
                first = false;
            }
            parseOutBeautify(out, depth - 1);
        }
        out += "}";
    }
    static inline std::string parseInEsc(char c) noexcept
    {
        char buf[12];
        if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
            snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
        } else {
            snprintf(buf, sizeof buf, "(%d)", c);
        }
        return std::string(buf);
    }
    static inline bool parseInRange(long x, long lower, long upper) noexcept
    {
        return (x >= lower && x <= upper);
    }
    inline Variant parseInFail(std::string&& msg) noexcept
    {
        return parseInFail(move(msg), Variant());
    }
    template <typename T>
    inline T parseInFail(std::string&& msg, const T err_ret) noexcept
    {
        if (!isFailed) {
            if (errorString) {
                *errorString = std::move(msg);
            }
        }
        isFailed = true;
        return err_ret;
    }
    void parseInConsumeWhitespace() noexcept
    {
        while (inString[index] == ' ' || inString[index] == '\r' || inString[index] == '\n' || inString[index] == '\t')
            index++;
    }
    bool parseInConsumeComment() noexcept
    {
        bool commentFound = false;
        if (inString[index] == '/') {
            index++;
            if (index == inString.size()) {
                return parseInFail("unexpected end of input after start of comment", false);
            }
            if (inString[index] == '/') {
                index++;
                while (index < inString.size() && inString[index] != '\n') {
                    index++;
                }
                commentFound = true;
            } else if (inString[index] == '*') {
                index++;
                if (index > inString.size() - 2) {
                    return parseInFail("unexpected end of input inside multi-line comment", false);
                }
                while (!(inString[index] == '*' && inString[index + 1] == '/')) {
                    index++;
                    if (index > inString.size() - 2) {
                        return parseInFail("unexpected end of input inside multi-line comment", false);
                    }
                }
                index += 2;
                commentFound = true;
            } else {
                return parseInFail("malformed comment", false);
            }
        }
        return commentFound;
    }
    void parseInConsumeGarbage() noexcept
    {
        parseInConsumeWhitespace();
        if (parseType == Variant::PARSE_IN_COMMENTS) {
            bool comment_found = false;
            do {
                comment_found = parseInConsumeComment();
                if (isFailed) {
                    return;
                }
                parseInConsumeWhitespace();
            } while (comment_found);
        }
    }
    char parseInGetNextToken() noexcept
    {
        parseInConsumeGarbage();
        if (isFailed) {
            return static_cast<char>(0);
        }
        if (index == inString.size()) {
            return parseInFail("unexpected end of input", static_cast<char>(0));
        }
        return inString[index++];
    }
    void parseInEncodeUtf8(long pt, std::string& out) noexcept
    {
        if (pt < 0) {
            return;
        }
        if (pt < 0x80) {
            out += static_cast<char>(pt);
        } else if (pt < 0x800) {
            out += static_cast<char>((pt >> 6) | 0xC0);
            out += static_cast<char>((pt & 0x3F) | 0x80);
        } else if (pt < 0x10000) {
            out += static_cast<char>((pt >> 12) | 0xE0);
            out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
            out += static_cast<char>((pt & 0x3F) | 0x80);
        } else {
            out += static_cast<char>((pt >> 18) | 0xF0);
            out += static_cast<char>(((pt >> 12) & 0x3F) | 0x80);
            out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
            out += static_cast<char>((pt & 0x3F) | 0x80);
        }
    }
    std::string parseInString() noexcept
    {
        std::string out;
        long lastEscapedCodepoint = -1;
        while (true) {
            if (index == inString.size()) {
                return parseInFail("unexpected end of input in string", "");
            }
            char ch = inString[index++];
            if (ch == '"') {
                parseInEncodeUtf8(lastEscapedCodepoint, out);
                return out;
            }
            if (parseInRange(ch, 0, 0x1f)) {
                return parseInFail("unescaped " + parseInEsc(ch) + " in string", "");
            }
            if (ch != '\\') {
                parseInEncodeUtf8(lastEscapedCodepoint, out);
                lastEscapedCodepoint = -1;
                out += ch;
                continue;
            }
            if (index == inString.size()) {
                return parseInFail("unexpected end of input in string", "");
            }
            ch = inString[index++];
            if (ch == 'u') {
                std::string esc = inString.substr(index, 4);
                if (esc.length() < 4) {
                    return parseInFail("bad \\u escape: " + esc, "");
                }
                for (size_t j = 0; j < 4; j++) {
                    if (!parseInRange(esc[j], 'a', 'f') && !parseInRange(esc[j], 'A', 'F') && !parseInRange(esc[j], '0', '9')) {
                        return parseInFail("bad \\u escape: " + esc, "");
                    }
                }
                long codepoint = strtol(esc.data(), nullptr, 16);
                if (parseInRange(lastEscapedCodepoint, 0xD800, 0xDBFF) && parseInRange(codepoint, 0xDC00, 0xDFFF)) {
                    parseInEncodeUtf8((((lastEscapedCodepoint - 0xD800) << 10) | (codepoint - 0xDC00)) + 0x10000, out);
                    lastEscapedCodepoint = -1;
                } else {
                    parseInEncodeUtf8(lastEscapedCodepoint, out);
                    lastEscapedCodepoint = codepoint;
                }
                index += 4;
                continue;
            }
            parseInEncodeUtf8(lastEscapedCodepoint, out);
            lastEscapedCodepoint = -1;
            if (ch == 'b') {
                out += '\b';
            } else if (ch == 'f') {
                out += '\f';
            } else if (ch == 'n') {
                out += '\n';
            } else if (ch == 'r') {
                out += '\r';
            } else if (ch == 't') {
                out += '\t';
            } else if (ch == '"' || ch == '\\' || ch == '/') {
                out += ch;
            } else {
                return parseInFail("invalid escape character " + parseInEsc(ch), "");
            }
        }
    }
    Variant parseInNumber() noexcept
    {
        size_t startPos = index;
        if (inString[index] == '-') {
            index++;
        }
        if (inString[index] == '0') {
            index++;
            if (parseInRange(inString[index], '0', '9')) {
                return parseInFail("leading 0s not permitted in numbers");
            }
        } else if (parseInRange(inString[index], '1', '9')) {
            index++;
            while (parseInRange(inString[index], '0', '9')) {
                index++;
            }
        } else {
            return parseInFail("invalid " + parseInEsc(inString[index]) + " in number");
        }
        if (inString[index] != '.' && inString[index] != 'e' && inString[index] != 'E' && (index - startPos) <= static_cast<size_t>(std::numeric_limits<int>::digits10)) {
            try {
                return (int)std::atoi(inString.c_str() + startPos);
            } catch (...) {
                return parseInFail("invalid " + parseInEsc(inString[index]) + " in int");
            }
        }
        if (inString[index] == '.') {
            index++;
            if (!parseInRange(inString[index], '0', '9')) {
                return parseInFail("at least one digit required in fractional part");
            }
            while (parseInRange(inString[index], '0', '9')) {
                index++;
            }
        }
        if (inString[index] == 'e' || inString[index] == 'E') {
            index++;
            if (inString[index] == '+' || inString[index] == '-') {
                index++;
            }
            if (!parseInRange(inString[index], '0', '9')) {
                return parseInFail("at least one digit required in exponent");
            }
            while (parseInRange(inString[index], '0', '9')) {
                index++;
            }
        }
        try {
            return (double)std::strtod(inString.c_str() + startPos, nullptr);
        } catch (...) {
            return parseInFail("invalid " + parseInEsc(inString[index]) + " in double");
        }
    }
    Variant parseInExpect(const std::string& expected, Variant res) noexcept
    {
        if (index == 0) {
            return parseInFail("parse error: expected " + expected + ", index can not be zero");
        }
        index--;
        if (inString.compare(index, expected.length(), expected) == 0) {
            index += expected.length();
            return res;
        } else {
            return parseInFail("parse error: expected " + expected + ", got " + inString.substr(index, expected.length()));
        }
    }
    Variant parseIn(int depth) noexcept
    {
        if (depth > VAR_PARSE_MAX_DEPTH) {
            return parseInFail("exceeded maximum nesting depth");
        }
        char ch = parseInGetNextToken();
        if (isFailed) {
            return Variant();
        }
        if (ch == '-' || (ch >= '0' && ch <= '9')) {
            index--;
            return parseInNumber();
        }
        if (ch == 't') {
            return parseInExpect("true", true);
        }
        if (ch == 'f') {
            return parseInExpect("false", false);
        }
        if (ch == 'n') {
            return parseInExpect("null", Variant());
        }
        if (ch == '"') {
            return parseInString();
        }
        if (ch == '{') {
            std::map<std::string, Variant> data;
            ch = parseInGetNextToken();
            if (ch == '}') {
                return data;
            }
            while (1) {
                if (ch != '"') {
                    return parseInFail("expected '\"' in object, got " + parseInEsc(ch));
                }
                std::string key = parseInString();
                if (isFailed) {
                    return Variant();
                }
                ch = parseInGetNextToken();
                if (ch != ':') {
                    return parseInFail("expected ':' in object, got " + parseInEsc(ch));
                }
                data[std::move(key)] = parseIn(depth + 1);
                if (isFailed) {
                    return Variant();
                }
                ch = parseInGetNextToken();
                if (ch == '}') {
                    break;
                }
                if (ch != ',') {
                    return parseInFail("expected ',' in object, got " + parseInEsc(ch));
                }
                ch = parseInGetNextToken();
            }
            return data;
        }
        if (ch == '[') {
            std::vector<Variant> data;
            ch = parseInGetNextToken();
            if (ch == ']') {
                return data;
            }
            while (1) {
                index--;
                data.push_back(parseIn(depth + 1));
                if (isFailed) {
                    return Variant();
                }
                ch = parseInGetNextToken();
                if (ch == ']') {
                    break;
                }
                if (ch != ',') {
                    return parseInFail("expected ',' in list, got " + parseInEsc(ch));
                }
                ch = parseInGetNextToken();
                (void)ch;
            }
            return data;
        }
        return parseInFail("expected value, got " + parseInEsc(ch));
    }
};

template <Variant::Type _type, typename T>
class VariantValueProxy : public VariantValue {
public:
    explicit VariantValueProxy(const T& value) noexcept
        : m_value(value)
    {
    }
    explicit VariantValueProxy(T&& value) noexcept
        : m_value(std::move(value))
    {
    }

protected:
    inline Variant::Type type() const noexcept override
    {
        return _type;
    }
    inline virtual bool isEqual(const VariantValue* value) const noexcept override
    {
        return m_value == static_cast<const VariantValueProxy<_type, T>*>(value)->m_value;
    }
    inline virtual bool isLess(const VariantValue* value) const noexcept override
    {
        return m_value < static_cast<const VariantValueProxy<_type, T>*>(value)->m_value;
    }
    inline virtual void parseOut(std::string& json, int depth) const noexcept override
    {
        return VariantParser::parseOut(m_value, json, depth);
    }
    const T m_value;
};

class VariantNull final : public VariantValueProxy<Variant::TYPE_NULL, VariantNullPtr> {
public:
    VariantNull() noexcept
        : VariantValueProxy({})
    {
    }
};

class VariantBool final : public VariantValueProxy<Variant::TYPE_BOOL, bool> {
    inline virtual bool toBool() const noexcept override
    {
        return m_value;
    }

public:
    explicit VariantBool(bool value) noexcept
        : VariantValueProxy(value)
    {
    }
};

namespace VariantGlobal {
static const std::shared_ptr<VariantValue> nullValue()
{
    static const std::shared_ptr<VariantValue> value = std::make_shared<VariantNull>();
    return value;
}
static const std::shared_ptr<VariantValue> trueValue()
{
    static const std::shared_ptr<VariantValue> value = std::make_shared<VariantBool>(true);
    return value;
}
static const std::shared_ptr<VariantValue> falseValue()
{
    static const std::shared_ptr<VariantValue> value = std::make_shared<VariantBool>(false);
    return value;
}
static const std::string& emptyString()
{
    static const std::string value;
    return value;
}
static const VariantList& emptyList()
{
    static const VariantList value;
    return value;
}
static const VariantMap& emptyMap()
{
    static const VariantMap value;
    return value;
}
static const Variant& virtualData()
{
    static const Variant value;
    return value;
}
}

class VariantInt final : public VariantValueProxy<Variant::TYPE_INT, int> {
    inline virtual int toInt() const noexcept override
    {
        return m_value;
    }
    inline virtual double toDouble() const noexcept override
    {
        return m_value;
    }
    inline virtual bool isEqual(const VariantValue* value) const noexcept override
    {
        return m_value == value->toInt();
    }
    inline virtual bool isLess(const VariantValue* value) const noexcept override
    {
        return m_value < value->toInt();
    }

public:
    explicit VariantInt(int value) noexcept
        : VariantValueProxy(value)
    {
    }
};

class VariantDouble final : public VariantValueProxy<Variant::TYPE_DOUBLE, double> {
    inline virtual int toInt() const noexcept override
    {
        return static_cast<int>(m_value);
    }
    inline virtual double toDouble() const noexcept override
    {
        return m_value;
    }
    inline virtual bool isEqual(const VariantValue* value) const noexcept override
    {
        if (m_value - value->toDouble() > -VAR_DOUBLE_PRECISION && value->toDouble() - m_value < VAR_DOUBLE_PRECISION) {
            return true;
        }
        return false;
    }
    inline virtual bool isLess(const VariantValue* value) const noexcept override
    {
        return m_value < value->toDouble();
    }

public:
    explicit VariantDouble(double value) noexcept
        : VariantValueProxy(value)
    {
    }
};

class VariantString final : public VariantValueProxy<Variant::TYPE_STRING, std::string> {
    inline virtual const std::string& toString() const noexcept override
    {
        return m_value;
    }

public:
    explicit VariantString(const std::string& value) noexcept
        : VariantValueProxy(value)
    {
    }
    explicit VariantString(std::string&& value) noexcept
        : VariantValueProxy(std::move(value))
    {
    }
};

class VariantArray final : public VariantValueProxy<Variant::TYPE_LIST, VariantList> {
    inline virtual const VariantList& toList() const noexcept override
    {
        return m_value;
    }
    inline virtual const Variant& operator[](size_t i) const noexcept override
    {
        if (i < 0 || i >= m_value.size()) {
            return VariantGlobal::virtualData();
        } else {
            return m_value[i];
        }
    }
#ifdef VAR_USE_OPT_WRITER
    inline virtual Variant& operator[](size_t i) override
    {
        if (i < 0 || i >= m_value.size()) {
            throw std::runtime_error("Variant operator[] list index out of bounds");
        } else {
            return const_cast<Variant&>(m_value[i]);
        }
    }
#endif

public:
    explicit VariantArray(const VariantList& value) noexcept
        : VariantValueProxy(value)
    {
    }
    explicit VariantArray(VariantList&& value) noexcept
        : VariantValueProxy(std::move(value))
    {
    }
};

class VariantObject final : public VariantValueProxy<Variant::TYPE_MAP, VariantMap> {
    inline virtual const VariantMap& toMap() const noexcept override
    {
        return m_value;
    }
    inline virtual const Variant& operator[](const std::string& key) const noexcept override
    {
        auto it = m_value.find(key);
        if (it == m_value.end()) {
            return VariantGlobal::virtualData();
        } else {
            return it->second;
        }
    }
#ifdef VAR_USE_OPT_WRITER
    inline virtual Variant& operator[](const std::string& key) override
    {
        auto it = m_value.find(key);
        if (it == m_value.end()) {
            throw std::runtime_error("Variant operator[] map key not exits");
        } else {
            return const_cast<Variant&>(it->second);
        }
    }
#endif

public:
    explicit VariantObject(const VariantMap& value) noexcept
        : VariantValueProxy(value)
    {
    }
    explicit VariantObject(VariantMap&& value) noexcept
        : VariantValueProxy(std::move(value))
    {
    }
};

Variant::Variant() noexcept
    : m_ptr(VariantGlobal::nullValue())
{
}

Variant::Variant(bool value) noexcept
    : m_ptr(value ? VariantGlobal::trueValue() : VariantGlobal::falseValue())
{
}

Variant::Variant(int value) noexcept
    : m_ptr(std::make_shared<VariantInt>(value))
{
}

Variant::Variant(double value) noexcept
    : m_ptr(std::make_shared<VariantDouble>(value))
{
}

Variant::Variant(const std::string& value) noexcept
    : m_ptr(std::make_shared<VariantString>(value))
{
}

Variant::Variant(std::string&& value) noexcept
    : m_ptr(std::make_shared<VariantString>(std::move(value)))
{
}

Variant::Variant(const char* value) noexcept
    : m_ptr(std::make_shared<VariantString>(value))
{
}

Variant::Variant(const VariantList& values) noexcept
    : m_ptr(std::make_shared<VariantArray>(values))
{
}

Variant::Variant(VariantList&& values) noexcept
    : m_ptr(std::make_shared<VariantArray>(std::move(values)))
{
}

Variant::Variant(const VariantMap& values) noexcept
    : m_ptr(std::make_shared<VariantObject>(values))
{
}

Variant::Variant(VariantMap&& values) noexcept
    : m_ptr(std::make_shared<VariantObject>(std::move(values)))
{
}

Variant::Type Variant::type() const noexcept
{
    return m_ptr->type();
}

bool Variant::isValid() const noexcept
{
    return m_ptr->type() != TYPE_NULL;
}

bool Variant::isNull() const noexcept
{
    return m_ptr->type() == TYPE_NULL;
}

bool Variant::isInt() const noexcept
{
    return m_ptr->type() == TYPE_INT;
}

bool Variant::isDouble() const noexcept
{
    return m_ptr->type() == TYPE_DOUBLE;
}

bool Variant::isNumber() const noexcept
{
    return m_ptr->type() == TYPE_INT || m_ptr->type() == TYPE_DOUBLE;
}

bool Variant::isBool() const noexcept
{
    return m_ptr->type() == TYPE_BOOL;
}

bool Variant::isString() const noexcept
{
    return m_ptr->type() == TYPE_STRING;
}

bool Variant::isList() const noexcept
{
    return m_ptr->type() == TYPE_LIST;
}

bool Variant::isMap() const noexcept
{
    return m_ptr->type() == TYPE_MAP;
}

bool Variant::toBool(bool defaultValue) const noexcept
{
    if (isBool()) {
        return m_ptr->toBool();
    }
    return defaultValue;
}

int Variant::toInt(int defaultValue) const noexcept
{
    if (isNumber()) {
        return m_ptr->toInt();
    }
    return defaultValue;
}

double Variant::toDouble(double defaultValue) const noexcept
{
    if (isNumber()) {
        return m_ptr->toDouble();
    }
    return defaultValue;
}

const std::string& Variant::toString(const std::string& defaultValue) const noexcept
{
    if (isString()) {
        return m_ptr->toString();
    }
    return defaultValue;
}

const char* Variant::toCString(const char* defaultValue) const noexcept
{
    if (isString()) {
        return m_ptr->toString().c_str();
    }
    return defaultValue;
}

std::vector<int> Variant::toIntList() const noexcept
{
    std::vector<int> out;
    for (const auto& sub : m_ptr->toList()) {
        if (!sub.isString()) {
            out.clear();
            return out;
        }
        out.push_back(sub.toInt());
    }
    return out;
}

std::vector<double> Variant::toDoubleList() const noexcept
{
    std::vector<double> out;
    for (const auto& sub : m_ptr->toList()) {
        if (!sub.isString()) {
            out.clear();
            return out;
        }
        out.push_back(sub.toDouble());
    }
    return out;
}

std::vector<std::string> Variant::toStringList() const noexcept
{
    std::vector<std::string> out;
    for (const auto& sub : m_ptr->toList()) {
        if (!sub.isString()) {
            out.clear();
            return out;
        }
        out.push_back(sub.toString());
    }
    return out;
}

const VariantList& Variant::toList() const noexcept
{
    return m_ptr->toList();
}

const VariantMap& Variant::toMap() const noexcept
{
    return m_ptr->toMap();
}

const Variant& Variant::listValue(int index, const Variant& defaultValue) noexcept
{
    const auto& var = (*m_ptr)[index];
    if (var.isNull()) {
        return defaultValue;
    }
    return var;
}

const Variant& Variant::mapValue(const std::string& key, const Variant& defaultValue) noexcept
{
    const auto& var = (*m_ptr)[key];
    if (var.isNull()) {
        return defaultValue;
    }
    return var;
}

const Variant& Variant::operator[](size_t i) const noexcept
{
    return m_ptr->operator[](i);
}

const Variant& Variant::operator[](const std::string& key) const noexcept
{
    return m_ptr->operator[](key);
}

#ifdef VAR_USE_OPT_WRITER
Variant& Variant::operator[](size_t i)
{
    return (*m_ptr)[i];
}
Variant& Variant::operator[](const std::string& key)
{
    return (*m_ptr)[key];
}
#endif

bool Variant::operator==(const Variant& rhs) const noexcept
{
    if (m_ptr == rhs.m_ptr) {
        return true;
    }
    if (m_ptr->type() != rhs.m_ptr->type()) {
        return false;
    }
    return m_ptr->isEqual(rhs.m_ptr.get());
}

bool Variant::operator<(const Variant& rhs) const noexcept
{
    if (m_ptr == rhs.m_ptr) {
        return false;
    }
    if (m_ptr->type() != rhs.m_ptr->type()) {
        return m_ptr->type() < rhs.m_ptr->type();
    }
    return m_ptr->isLess(rhs.m_ptr.get());
}

bool Variant::operator!=(const Variant& rhs) const noexcept
{
    return !(*this == rhs);
}

bool Variant::operator<=(const Variant& rhs) const noexcept
{
    return !(rhs < *this);
}

bool Variant::operator>(const Variant& rhs) const noexcept
{
    return (rhs < *this);
}

bool Variant::operator>=(const Variant& rhs) const noexcept
{
    return !(*this < rhs);
}

std::string Variant::toJson(ParseType parseType) const noexcept
{
    std::string json;
    m_ptr->parseOut(json, parseType == PARSE_OUT_FORMAT ? 0 : -1);
    return json;
}

template <class T>
static inline bool saveJsonTemplate(const T& data, const std::string& filePath, Variant::ParseType parseType)
{
    std::string jsonStr = data.toJson(parseType);
    if (jsonStr.empty()) {
        return false;
    }
    std::ofstream wfile;
    wfile.open(filePath, std::ios::out | std::ios::trunc);
    if (!wfile.is_open()) {
        return false;
    }
    wfile << jsonStr;
    wfile.close();
    return true;
}

bool Variant::saveJson(const std::string& filePath, ParseType parseType) const noexcept
{
    return saveJsonTemplate<Variant>(*this, filePath, parseType);
}

Variant Variant::fromJson(const std::string& json, std::string* errorString, ParseType parseType) noexcept
{
    VariantParser parse(json, errorString, parseType);
    Variant result = parse.parseIn(0);
    parse.parseInConsumeGarbage();
    if (parse.isFailed) {
        return Variant();
    }
    if (parse.index != json.size()) {
        return parse.parseInFail("unexpected trailing " + VariantParser::parseInEsc(json[parse.index]));
    }
    return result;
}

Variant Variant::readJson(const std::string& filePath, std::string* errorString, ParseType parseType) noexcept
{
    std::ifstream rfile;
    rfile.open(filePath, std::ios::in);
    if (!rfile.is_open()) {
        return Variant();
    }
    std::stringstream buffer;
    buffer << rfile.rdbuf();
    rfile.close();
    return fromJson(buffer.str(), errorString, parseType);
}

std::ostream& operator<<(std::ostream& ostream, const Variant& data) noexcept
{
    ostream << data.toJson(Variant::PARSE_OUT_FORMAT);
    return ostream;
}

bool VariantValue::toBool() const noexcept
{
    return false;
}

int VariantValue::toInt() const noexcept
{
    return 0;
}

double VariantValue::toDouble() const noexcept
{
    return 0;
}

const std::string& VariantValue::toString() const noexcept
{
    return VariantGlobal::emptyString();
}

const VariantList& VariantValue::toList() const noexcept
{
    return VariantGlobal::emptyList();
}

const VariantMap& VariantValue::toMap() const noexcept
{
    return VariantGlobal::emptyMap();
}

const Variant& VariantValue::operator[](size_t i) const noexcept
{
    return VariantGlobal::virtualData();
}

const Variant& VariantValue::operator[](const std::string& key) const noexcept
{
    return VariantGlobal::virtualData();
}

#ifdef VAR_USE_OPT_WRITER
Variant& VariantValue::operator[](size_t i)
{
    throw std::runtime_error("Variant operator[] called on a non-list type");
}

Variant& VariantValue::operator[](const std::string& key)
{
    throw std::runtime_error("Variant operator[] called on a non-map type");
}
#endif

VariantList::VariantList(const Variant& values) noexcept
{
    *this = values.toList();
}

VariantList::VariantList(Variant&& values) noexcept
{
    *this = std::move(values.toList());
}

bool VariantList::contains(const Variant& data) const noexcept
{
    return std::any_of(begin(), end(), [&](const auto& sub) { return sub == data; });
}

const Variant& VariantList::value(int index, const Variant& defaultValue) const noexcept
{
    if (index < 0 || index >= this->size()) {
        return defaultValue;
    }
    return (*this)[index];
}

std::string VariantList::toJson(Variant::ParseType parseType) const noexcept
{
    std::string json;
    VariantParser::parseOut(*this, json, parseType == Variant::PARSE_OUT_FORMAT ? 0 : -1);
    return json;
}

bool VariantList::saveJson(const std::string& filePath, Variant::ParseType parseType) const noexcept
{
    return saveJsonTemplate<Variant>(*this, filePath, parseType);
}

std::ostream& operator<<(std::ostream& ostream, const VariantList& data) noexcept
{
    ostream << data.toJson(Variant::PARSE_OUT_FORMAT);
    return ostream;
}

VariantMap::VariantMap(const Variant& values) noexcept
{
    *this = values.toMap();
}

VariantMap::VariantMap(Variant&& values) noexcept
{
    *this = std::move(values.toMap());
}

bool VariantMap::remove(const std::string& key) noexcept
{
    auto it = find(key);
    if (it != end()) {
        erase(it);
        return true;
    }
    return false;
}

bool VariantMap::contains(const std::string& key) const noexcept
{
    return std::any_of(begin(), end(), [&](const auto& sub) { return sub.first == key; });
}

const Variant& VariantMap::value(const std::string& key, const Variant& defaultValue) const noexcept
{
    auto it = find(key);
    return (it == end()) ? defaultValue : it->second;
}

std::string VariantMap::toJson(Variant::ParseType parseType) const noexcept
{
    std::string json;
    VariantParser::parseOut(*this, json, parseType == Variant::PARSE_OUT_FORMAT ? 0 : -1);
    return json;
}

bool VariantMap::saveJson(const std::string& filePath, Variant::ParseType parseType) const noexcept
{
    return saveJsonTemplate<Variant>(*this, filePath, parseType);
}

std::ostream& operator<<(std::ostream& ostream, const VariantMap& data) noexcept
{
    ostream << data.toJson(Variant::PARSE_OUT_FORMAT);
    return ostream;
}

DCUS_NAMESPACE_END
