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

#ifndef DCUS_LOG_H
#define DCUS_LOG_H

#include "dcus/base/define.h"
#include <iostream>

#define DCUS_DEBUG_TYPE_NUM 0
#define DCUS_WARNING_TYPE_NUM 1
#define DCUS_CRITICAL_TYPE_NUM 2
#define DCUS_PROPERTY_TYPE_NUM 3

extern DCUS_EXPORT void dcus_print_initialize(const std::string& name);
extern DCUS_EXPORT void dcus_print_uninitialize();
extern DCUS_EXPORT std::ostream& _dcus_print_start(uint8_t type);
extern DCUS_EXPORT void _dcus_print_final(std::ostream& stream);
inline void _dcus_print_expand(std::ostream& stream)
{
    _dcus_print_final(stream);
}
template <typename VALUE, typename... ARGS>
inline constexpr void _dcus_print_expand(std::ostream& stream, VALUE&& value, ARGS&&... args)
{
    stream << value;
    _dcus_print_expand(stream, std::forward<ARGS>(args)...);
}
template <typename T, size_t S>
inline constexpr size_t _dcus_get_file_name(const T (&str)[S], size_t i = S - 1)
{
    return (str[i] == '/' || str[i] == '\\') ? i + 1 : (i > 0 ? _dcus_get_file_name(str, i - 1) : 0);
}
template <typename T>
inline constexpr size_t _dcus_get_file_name(T (&)[1])
{
    return 0;
}

#define LOG_DEBUG(...) \
    _dcus_print_expand(_dcus_print_start(DCUS_DEBUG_TYPE_NUM), __VA_ARGS__)
#define LOG_WARNING(...) \
    _dcus_print_expand(_dcus_print_start(DCUS_WARNING_TYPE_NUM), "[", &__FILE__[_dcus_get_file_name(__FILE__)], "|", __LINE__, "|", __FUNCTION__, "]: ", __VA_ARGS__)
#define LOG_CRITICAL(...)                                                                                                                                                         \
    _dcus_print_expand(_dcus_print_start(DCUS_CRITICAL_TYPE_NUM), "[*CRITICAL*", &__FILE__[_dcus_get_file_name(__FILE__)], "|", __LINE__, "|", __FUNCTION__, "]: ", __VA_ARGS__); \
    std::terminate()
#define LOG_PROPERTY(p, ...) \
    _dcus_print_expand(_dcus_print_start(DCUS_PROPERTY_TYPE_NUM), "[" + std::string(p) + "]: ", __VA_ARGS__)

#endif // DCUS_LOG_H
