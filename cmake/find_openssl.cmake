#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2021
#  *Author:  Juntuan.Lu
#  *Version: 1.0
#  *Date:  2021/04/22
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

find_package(OpenSSL QUIET)

if(OPENSSL_FOUND AND OPENSSL_INCLUDE_DIR AND OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
    set(OPENSSL_ENABLE ON)
    message(STATUS "openssl found")
    message(STATUS "OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OPENSSL_SSL_LIBRARY=${OPENSSL_SSL_LIBRARY}")
    message(STATUS "OPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}")
else()
    set(OPENSSL_ENABLE OFF)
    message(STATUS "openssl not found")
endif()
