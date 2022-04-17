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

#ifndef DCUS_DEFINE_H
#define DCUS_DEFINE_H

#ifdef DCUS_LIBRARY_STATIC
#define DCUS_EXPORT
#else
#if (defined _WIN32 || defined _WIN64)
#ifdef DCUS_LIBRARY
#define DCUS_EXPORT __declspec(dllexport)
#else
#define DCUS_EXPORT __declspec(dllimport)
#endif
#else
#define DCUS_EXPORT
#endif
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4251)
#pragma warning(disable : 4996)
#endif

#ifdef __cplusplus
#define C_INTERFACE_BEGIN extern "C" {
#define C_INTERFACE_END }
#else
#define C_INTERFACE_BEGIN
#define C_INTERFACE_END
#endif

#define DCUS_NAMESPACE_BEGIN \
    namespace DCus {
#define DCUS_NAMESPACE_END \
    }                      \
    ;

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;    \
    TypeName& operator=(const TypeName&) = delete;

#define CREATE_PRIVATE_PROPERTY(ValueType, name, value)                \
public:                                                                \
    inline const ValueType& name() const noexcept { return m_##name; } \
                                                                       \
private:                                                               \
    ValueType m_##name = value;
#define CREATE_PUBLIC_PROPERTY(ValueType, name, value)     \
public:                                                    \
    inline ValueType& name() noexcept { return m_##name; } \
    CREATE_PRIVATE_PROPERTY(ValueType, name, value)

#define DCUS_UNUSED(Var) (void)(Var)
#define DCUS_STRNAME(Name) #Name
#define DCUS_GETNAME(Name) DCUS_STRNAME(Name)

#endif // DCUS_DEFINE_H
