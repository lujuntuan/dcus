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

start_rpcs_project()

if(MSVC)
    foreach(var
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
            )
        if(${var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
        endif()
    endforeach()
endif()

macro(add_dcus_interfaces)
    add_dependencies(${PROJECT_NAME} ${RPC_INTERFACE_NAME})
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${PROTOBUF_INCLUDE_DIR}
        ${FDBUS_INCLUDE_DIR}
        ${PROJECT_BINARY_DIR}/../interfaces
        )
    target_link_libraries(
        ${PROJECT_NAME}
        $<BUILD_INTERFACE:${RPC_INTERFACE_NAME}>
        $<BUILD_INTERFACE:${PROTOBUF_LIBRARY}>
        $<BUILD_INTERFACE:${FDBUS_LIBRARY}>
        )
    target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
        "-DFDB_CONNECT_ADDRESS=\"svc://${CMAKE_PROJECT_NAME}\""
        )
    if(MSVC)
        target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            "-D__WIN32__"
            )
    endif()
endmacro(add_dcus_interfaces)

add_subdirectory(interfaces)
if(DCUS_BUILD_MASTER)
    add_subdirectory(master)
endif()
if(DCUS_BUILD_WORKER)
    add_subdirectory(worker)
endif()

end_rpcs_project()
