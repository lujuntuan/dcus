#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2021
#  *Author:  Lu Juntuan
#  *Version: 1.0
#  *Date:  2021/04/22
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************
project(${RPC_INTERFACE_NAME} VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES C CXX)

commonapi_core_generate_src(
    COMMONAPI_CORE_IDL_SRC
    "v1/commonapi/DCusInterfaces"
    ${CMAKE_CURRENT_LIST_DIR}/*.fidl
    )
commonapi_someip_generate_src(
    COMMONAPI_SOMEIP_IDL_SRC
    "v1/commonapi/DCusInterfaces"
    ${CMAKE_CURRENT_LIST_DIR}/*.fdepl
    )
add_library(
    ${PROJECT_NAME}
    ${COMMONAPI_CORE_IDL_SRC}
    ${COMMONAPI_SOMEIP_IDL_SRC}
    )
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${PROJECT_BINARY_DIR}
    )
if(CommonAPI-SomeIP_FOUND)
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${COMMONAPI_CORE_INCLUDE_DIR}
        ${COMMONAPI_SOMEIP_INCLUDE_DIR}
        )
    target_link_libraries(
        ${PROJECT_NAME}
        CommonAPI
        CommonAPI-SomeIP
        )
else()
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${COMMONAPI_CORE_INCLUDE_DIR}
        ${COMMONAPI_SOMEIP_INCLUDE_DIR}
        )
    target_link_libraries(
        ${PROJECT_NAME}
        ${COMMONAPI_CORE_LIBRARY}
        ${COMMONAPI_SOMEIP_LIBRARY}
        )
endif()
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    "-DCOMMONAPI_DLL_COMPILATION"
    )
install(
    TARGETS
    ${PROJECT_NAME}
    RUNTIME
    DESTINATION
    bin
    ARCHIVE
    DESTINATION
    lib
    LIBRARY
    DESTINATION
    lib
    )
install_etc(${PROJECT_NAME})
