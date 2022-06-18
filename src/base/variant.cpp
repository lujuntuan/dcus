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
#include <sstream>

#define VAR_PARSER_MAX_DEPTH 500
#define VAR_PARSER_FORMAT_SPACE "    "
#define VAR_DOUBLE_PRECISION (double)0.0000001

DCUS_NAMESPACE_BEGIN

struct NullStruct {
    bool operator==(NullStruct) const { return true; }
    bool operator<(NullStruct) const { return false; }
};

class VariantParser {
public:
    explicit VariantParser(const std::string& str, std::string* err, Variant::ParserType type) noexcept
        : _string(str)
        , _errorString(err)
        , _parserType(type)
    {
    }

protected:
    size_t _index = 0;
    bool _isFailed = false;
    const std::string& _string;
    std::string* _errorString = nullptr;
    const Variant::ParserType _parserType = Variant::PARSER_UNKNOWN;

private:
    friend class Variant;

public:
    static void formatEnter(std::string& out, int depth) noexcept
    {
        if (depth < 0) {
            return;
        }
        out += "\n";
        for (size_t i = 0; i < depth; i++) {
            out += VAR_PARSER_FORMAT_SPACE;
        }
    }
    static void dump(NullStruct, std::string& out, int depth) noexcept
    {
        out += "null";
    }
    static void dump(double value, std::string& out, int depth) noexcept
    {
        if (std::isfinite(value)) {
            char buf[32];
            snprintf(buf, sizeof buf, "%.17g", value);
            out += buf;
        } else {
            out += "null";
        }
    }
    static void dump(int value, std::string& out, int depth) noexcept
    {
        char buf[32];
        snprintf(buf, sizeof buf, "%d", value);
        out += buf;
    }
    static void dump(bool value, std::string& out, int depth) noexcept
    {
        out += value ? "true" : "false";
    }
    static void dump(const std::string& value, std::string& out, int depth) noexcept
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
    static void dump(const VariantList& values, std::string& out, int depth) noexcept
    {
        if (depth >= 0) {
            if (depth > VAR_PARSER_MAX_DEPTH) {
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
                formatEnter(out, depth);
                value.m_ptr->dump(out, depth);
                first = false;
            }
            formatEnter(out, depth - 1);
        }
        out += "]";
    }
    static void dump(const VariantMap& values, std::string& out, int depth) noexcept
    {
        if (depth >= 0) {
            if (depth > VAR_PARSER_MAX_DEPTH) {
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
                formatEnter(out, depth);
                dump(kv.first, out, depth);
                out += ": ";
                kv.second.m_ptr->dump(out, depth);
                first = false;
            }
            formatEnter(out, depth - 1);
        }
        out += "}";
    }

public:
    static inline std::string esc(char c) noexcept
    {
        char buf[12];
        if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
            snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
        } else {
            snprintf(buf, sizeof buf, "(%d)", c);
        }
        return std::string(buf);
    }
    static inline bool inRange(long x, long lower, long upper) noexcept
    {
        return (x >= lower && x <= upper);
    }
    Variant fail(std::string&& msg) noexcept
    {
        return fail(move(msg), Variant());
    }
    template <typename T>
    T fail(std::string&& msg, const T err_ret) noexcept
    {
        if (!_isFailed) {
            if (_errorString) {
                *_errorString = std::move(msg);
            }
        }
        _isFailed = true;
        return err_ret;
    }
    void consumeWhitespace() noexcept
    {
        while (_string[_index] == ' ' || _string[_index] == '\r' || _string[_index] == '\n' || _string[_index] == '\t')
            _index++;
    }
    bool consumeComment() noexcept
    {
        bool commentFound = false;
        if (_string[_index] == '/') {
            _index++;
            if (_index == _string.size()) {
                return fail("unexpected end of input after start of comment", false);
            }
            if (_string[_index] == '/') {
                _index++;
                while (_index < _string.size() && _string[_index] != '\n') {
                    _index++;
                }
                commentFound = true;
            } else if (_string[_index] == '*') {
                _index++;
                if (_index > _string.size() - 2) {
                    return fail("unexpected end of input inside multi-line comment", false);
                }
                while (!(_string[_index] == '*' && _string[_index + 1] == '/')) {
                    _index++;
                    if (_index > _string.size() - 2) {
                        return fail("unexpected end of input inside multi-line comment", false);
                    }
                }
                _index += 2;
                commentFound = true;
            } else {
                return fail("malformed comment", false);
            }
        }
        return commentFound;
    }
    void consumeGarbage() noexcept
    {
        consumeWhitespace();
        if (_parserType == Variant::PARSER_IN_COMMENTS) {
            bool comment_found = false;
            do {
                comment_found = consumeComment();
                if (_isFailed) {
                    return;
                }
                consumeWhitespace();
            } while (comment_found);
        }
    }
    char getNextToken() noexcept
    {
        consumeGarbage();
        if (_isFailed) {
            return static_cast<char>(0);
        }
        if (_index == _string.size()) {
            return fail("unexpected end of input", static_cast<char>(0));
        }
        return _string[_index++];
    }
    void encodeUtf8(long pt, std::string& out) noexcept
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
    std::string parseString() noexcept
    {
        std::string out;
        long lastEscapedCodepoint = -1;
        while (true) {
            if (_index == _string.size()) {
                return fail("unexpected end of input in string", "");
            }
            char ch = _string[_index++];
            if (ch == '"') {
                encodeUtf8(lastEscapedCodepoint, out);
                return out;
            }
            if (inRange(ch, 0, 0x1f)) {
                return fail("unescaped " + esc(ch) + " in string", "");
            }
            if (ch != '\\') {
                encodeUtf8(lastEscapedCodepoint, out);
                lastEscapedCodepoint = -1;
                out += ch;
                continue;
            }
            if (_index == _string.size()) {
                return fail("unexpected end of input in string", "");
            }
            ch = _string[_index++];
            if (ch == 'u') {
                std::string esc = _string.substr(_index, 4);
                if (esc.length() < 4) {
                    return fail("bad \\u escape: " + esc, "");
                }
                for (size_t j = 0; j < 4; j++) {
                    if (!inRange(esc[j], 'a', 'f') && !inRange(esc[j], 'A', 'F') && !inRange(esc[j], '0', '9')) {
                        return fail("bad \\u escape: " + esc, "");
                    }
                }
                long codepoint = strtol(esc.data(), nullptr, 16);
                if (inRange(lastEscapedCodepoint, 0xD800, 0xDBFF) && inRange(codepoint, 0xDC00, 0xDFFF)) {
                    encodeUtf8((((lastEscapedCodepoint - 0xD800) << 10) | (codepoint - 0xDC00)) + 0x10000, out);
                    lastEscapedCodepoint = -1;
                } else {
                    encodeUtf8(lastEscapedCodepoint, out);
                    lastEscapedCodepoint = codepoint;
                }
                _index += 4;
                continue;
            }
            encodeUtf8(lastEscapedCodepoint, out);
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
                return fail("invalid escape character " + esc(ch), "");
            }
        }
    }
    Variant parseNumber() noexcept
    {
        size_t startPos = _index;
        if (_string[_index] == '-') {
            _index++;
        }
        if (_string[_index] == '0') {
            _index++;
            if (inRange(_string[_index], '0', '9')) {
                return fail("leading 0s not permitted in numbers");
            }
        } else if (inRange(_string[_index], '1', '9')) {
            _index++;
            while (inRange(_string[_index], '0', '9')) {
                _index++;
            }
        } else {
            return fail("invalid " + esc(_string[_index]) + " in number");
        }
        if (_string[_index] != '.' && _string[_index] != 'e' && _string[_index] != 'E' && (_index - startPos) <= static_cast<size_t>(std::numeric_limits<int>::digits10)) {
            try {
                return (int)std::atoi(_string.c_str() + startPos);
            } catch (...) {
                return fail("invalid " + esc(_string[_index]) + " in int");
            }
        }
        if (_string[_index] == '.') {
            _index++;
            if (!inRange(_string[_index], '0', '9')) {
                return fail("at least one digit required in fractional part");
            }
            while (inRange(_string[_index], '0', '9')) {
                _index++;
            }
        }
        if (_string[_index] == 'e' || _string[_index] == 'E') {
            _index++;
            if (_string[_index] == '+' || _string[_index] == '-') {
                _index++;
            }
            if (!inRange(_string[_index], '0', '9')) {
                return fail("at least one digit required in exponent");
            }
            while (inRange(_string[_index], '0', '9')) {
                _index++;
            }
        }
        try {
            return (double)std::strtod(_string.c_str() + startPos, nullptr);
        } catch (...) {
            return fail("invalid " + esc(_string[_index]) + " in double");
        }
    }
    Variant expect(const std::string& expected, Variant res) noexcept
    {
        if (_index == 0) {
            return fail("parse error: expected " + expected + ", index can not be zero");
        }
        _index--;
        if (_string.compare(_index, expected.length(), expected) == 0) {
            _index += expected.length();
            return res;
        } else {
            return fail("parse error: expected " + expected + ", got " + _string.substr(_index, expected.length()));
        }
    }
    Variant formJson(int depth) noexcept
    {
        if (depth > VAR_PARSER_MAX_DEPTH) {
            return fail("exceeded maximum nesting depth");
        }
        char ch = getNextToken();
        if (_isFailed) {
            return Variant();
        }
        if (ch == '-' || (ch >= '0' && ch <= '9')) {
            _index--;
            return parseNumber();
        }
        if (ch == 't') {
            return expect("true", true);
        }
        if (ch == 'f') {
            return expect("false", false);
        }
        if (ch == 'n') {
            return expect("null", Variant());
        }
        if (ch == '"') {
            return parseString();
        }
        if (ch == '{') {
            std::map<std::string, Variant> data;
            ch = getNextToken();
            if (ch == '}') {
                return data;
            }
            while (1) {
                if (ch != '"') {
                    return fail("expected '\"' in object, got " + esc(ch));
                }
                std::string key = parseString();
                if (_isFailed) {
                    return Variant();
                }
                ch = getNextToken();
                if (ch != ':') {
                    return fail("expected ':' in object, got " + esc(ch));
                }
                data[std::move(key)] = formJson(depth + 1);
                if (_isFailed) {
                    return Variant();
                }
                ch = getNextToken();
                if (ch == '}') {
                    break;
                }
                if (ch != ',') {
                    return fail("expected ',' in object, got " + esc(ch));
                }
                ch = getNextToken();
            }
            return data;
        }
        if (ch == '[') {
            std::vector<Variant> data;
            ch = getNextToken();
            if (ch == ']') {
                return data;
            }
            while (1) {
                _index--;
                data.push_back(formJson(depth + 1));
                if (_isFailed) {
                    return Variant();
                }
                ch = getNextToken();
                if (ch == ']') {
                    break;
                }
                if (ch != ',') {
                    return fail("expected ',' in list, got " + esc(ch));
                }
                ch = getNextToken();
                (void)ch;
            }
            return data;
        }
        return fail("expected value, got " + esc(ch));
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
    inline virtual void dump(std::string& json, int depth) const noexcept override
    {
        return VariantParser::dump(m_value, json, depth);
    }
    const T m_value;
};

class VariantNull final : public VariantValueProxy<Variant::TYPE_NULL, NullStruct> {
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

// Static globals - static-init-safe

struct VariantStatics {
    const std::shared_ptr<VariantValue> nullValue = std::make_shared<VariantNull>();
    const std::shared_ptr<VariantValue> trueValue = std::make_shared<VariantBool>(true);
    const std::shared_ptr<VariantValue> falseValue = std::make_shared<VariantBool>(false);
    const std::string emptyString;
    const VariantList emptyList;
    const VariantMap emptyMap;
    VariantStatics() { }
};

static const VariantStatics& variantStatics()
{
    static const VariantStatics statics {};
    return statics;
}

static const Variant& variantStaticNull()
{
    static const Variant data;
    return data;
}

//

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
            return variantStaticNull();
        } else {
            return m_value[i];
        }
    }

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
        return (it == m_value.end()) ? variantStaticNull() : it->second;
    }

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
    : m_ptr(variantStatics().nullValue)
{
}

Variant::Variant(std::nullptr_t value) noexcept
    : m_ptr(variantStatics().nullValue)
{
}

Variant::Variant(bool value) noexcept
    : m_ptr(value ? variantStatics().trueValue : variantStatics().falseValue)
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
    return (*m_ptr)[i];
}

const Variant& Variant::operator[](const std::string& key) const noexcept
{
    return (*m_ptr)[key];
}

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

std::string Variant::toJson(ParserType parserType) const noexcept
{
    std::string json;
    m_ptr->dump(json, parserType == PARSER_OUT_FORMAT ? 0 : -1);
    return json;
}

template <class T>
static bool saveJsonTemplate(const T& data, const std::string& filePath, Variant::ParserType parserType)
{
    std::string jsonStr = data.toJson(parserType);
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

bool Variant::saveJson(const std::string& filePath, ParserType parserType) const noexcept
{
    return saveJsonTemplate<Variant>(*this, filePath, parserType);
}

Variant Variant::fromJson(const std::string& json, std::string* errorString, ParserType parserType) noexcept
{
    VariantParser parser(json, errorString, parserType);
    Variant result = parser.formJson(0);
    parser.consumeGarbage();
    if (parser._isFailed) {
        return Variant();
    }
    if (parser._index != json.size()) {
        return parser.fail("unexpected trailing " + VariantParser::esc(json[parser._index]));
    }
    return result;
}

Variant Variant::readJson(const std::string& filePath, std::string* errorString, ParserType parserType) noexcept
{
    std::ifstream rfile;
    rfile.open(filePath, std::ios::in);
    if (!rfile.is_open()) {
        return Variant();
    }
    std::stringstream buffer;
    buffer << rfile.rdbuf();
    rfile.close();
    return fromJson(buffer.str(), errorString, parserType);
}

std::ostream& operator<<(std::ostream& ostream, const Variant& data) noexcept
{
    ostream << data.toJson(Variant::PARSER_OUT_FORMAT);
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
    return variantStatics().emptyString;
}

const VariantList& VariantValue::toList() const noexcept
{
    return variantStatics().emptyList;
}

const VariantMap& VariantValue::toMap() const noexcept
{
    return variantStatics().emptyMap;
}

const Variant& VariantValue::operator[](size_t i) const noexcept
{
    return variantStaticNull();
}

const Variant& VariantValue::operator[](const std::string& key) const noexcept
{
    return variantStaticNull();
}

VariantList::VariantList(const Variant& values) noexcept
{
    *this = values.toList();
}

VariantList::VariantList(Variant&& values) noexcept
{
    *this = std::move(values.toList());
}

void VariantList::add(const Variant& data) noexcept
{
    push_back(data);
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

Variant VariantList::toVariant() const noexcept
{
    return Variant(*this);
}

std::string VariantList::toJson(Variant::ParserType parserType) const noexcept
{
    std::string json;
    VariantParser::dump(*this, json, parserType == Variant::PARSER_OUT_FORMAT ? 0 : -1);
    return json;
}

bool VariantList::saveJson(const std::string& filePath, Variant::ParserType parserType) const noexcept
{
    return saveJsonTemplate<Variant>(*this, filePath, parserType);
}

std::ostream& operator<<(std::ostream& ostream, const VariantList& data) noexcept
{
    ostream << data.toJson(Variant::PARSER_OUT_FORMAT);
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

void VariantMap::add(const std::string& key, const Variant& data) noexcept
{
    auto it = find(key);
    if (it != end()) {
        erase(key);
    }
    emplace(key, data);
}

void VariantMap::add(int key, const Variant& data) noexcept
{
    add(std::to_string(key), data);
}

void VariantMap::sub(const std::string& key) noexcept
{
    erase(key);
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

Variant VariantMap::toVariant() const noexcept
{
    return Variant(*this);
}

std::string VariantMap::toJson(Variant::ParserType parserType) const noexcept
{
    std::string json;
    VariantParser::dump(*this, json, parserType == Variant::PARSER_OUT_FORMAT ? 0 : -1);
    return json;
}

bool VariantMap::saveJson(const std::string& filePath, Variant::ParserType parserType) const noexcept
{
    return saveJsonTemplate<Variant>(*this, filePath, parserType);
}

std::ostream& operator<<(std::ostream& ostream, const VariantMap& data) noexcept
{
    ostream << data.toJson(Variant::PARSER_OUT_FORMAT);
    return ostream;
}

DCUS_NAMESPACE_END
