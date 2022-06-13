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

#ifndef DCUS_DATA_H
#define DCUS_DATA_H

#include "dcus/base/define.h"
#include "dcus/base/value.h"
#include <map>
#include <string>
#include <vector>

DCUS_NAMESPACE_BEGIN

struct DCUS_EXPORT Data final {
public:
    Data() noexcept = default;
    Data(const std::initializer_list<std::pair<std::string, Value>>& data) noexcept;
    Data(Data&& data) noexcept;
    Data(const Data& data) noexcept;
    void add(const std::string& key, const Value& value) noexcept;
    void add(int key, const Value& value) noexcept;
    void sub(const std::string& key) noexcept;
    bool contains(const std::string& key) noexcept;
    void clear() noexcept;
    void swap(Data&& data) noexcept;
    inline size_t size() const noexcept
    {
        return m_map.size();
    }
    inline std::map<std::string, Value>::const_iterator begin() const noexcept
    {
        return m_map.begin();
    }
    inline std::map<std::string, Value>::const_iterator end() const noexcept
    {
        return m_map.end();
    }
    inline bool empty() const noexcept
    {
        return m_map.empty();
    }
    const Value value(const std::string& key, const Value& defaultValue = Value()) const noexcept;
    const Value value(int key, const Value& defaultValue = Value()) const noexcept;
    Value& operator[](const std::string& key) noexcept;
    const Value operator[](const std::string& key) const noexcept;
    Data& operator<<(const std::pair<std::string, Value>& pair) noexcept;
    Data& operator=(const Data& data) noexcept;
    Data& operator=(Data&& data) noexcept;
    bool operator==(const Data& data) const noexcept;
    bool operator!=(const Data& data) const noexcept;
    static Data readStream(const std::string& jsonStr, bool printErrorLog = false) noexcept;
    static Data read(const std::string& filePath) noexcept;
    static bool saveStream(std::string& jsonStr, const Data& data) noexcept;
    static bool save(const std::string& filePath, const Data& data) noexcept;
    bool save(const std::string& filePath) const noexcept;
    std::string toStream() const noexcept;
    DCUS_EXPORT friend std::ostream& operator<<(std::ostream& ostream, const Data& data) noexcept;

private:
    std::map<std::string, Value> m_map;
};

DCUS_NAMESPACE_END

#endif // DCUS_DATA_H
