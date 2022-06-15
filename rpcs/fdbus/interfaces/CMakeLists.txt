#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2021
#  *Author:  Juntuan.Lu
#  *Version: 1.0
#  *Date:  2022/04/01
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************
project(${RPC_INTERFACE_NAME} VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES C CXX)

proto_generate_src(
    PROTO_IDL_SRC
    ${CMAKE_CURRENT_LIST_DIR}/*.proto
    )
if(MSVC)
    add_library(
        ${PROJECT_NAME}
        STATIC
        ${PROTO_IDL_SRC}
        )
else()
    add_library(
        ${PROJECT_NAME}
        SHARED
        ${PROTO_IDL_SRC}
        )
endif()
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${PROTOBUF_INCLUDE_DIR}
    ${PROJECT_BINARY_DIR}
    )
target_link_libraries(
    ${PROJECT_NAME}
    ${PROTOBUF_LIBRARY}
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
