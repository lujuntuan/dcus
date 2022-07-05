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

#define DCUS_NAMESPACE_BEGIN namespace DCus {
#define DCUS_NAMESPACE_END }

#define CLASS_DISSABLE_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;          \
    TypeName& operator=(const TypeName&) = delete;

#define DCUS_HELPER_CREATE(helper) helper = new std::remove_reference<decltype(*helper)>::type
#define DCUS_HELPER_DESTROY(helper) delete helper

#define DCUS_UNUSED(Var) (void)(Var)
#define DCUS_STRNAME(Name) #Name
#define DCUS_GETNAME(Name) DCUS_STRNAME(Name)
#define DCUS_MEM_ALIGN alignas(8)

#endif // DCUS_DEFINE_H
