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

start_rpcs_project()

macro(add_dcus_interfaces)
    add_dependencies(${PROJECT_NAME} ${RPC_INTERFACE_NAME})
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${PROJECT_BINARY_DIR}/../interfaces
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
            $<BUILD_INTERFACE:${RPC_INTERFACE_NAME}>
            $<BUILD_INTERFACE:CommonAPI>
            $<BUILD_INTERFACE:CommonAPI-SomeIP>
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
            $<BUILD_INTERFACE:${RPC_INTERFACE_NAME}>
            $<BUILD_INTERFACE:${COMMONAPI_CORE_LIBRARY}>
            $<BUILD_INTERFACE:${COMMONAPI_SOMEIP_LIBRARY}>
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
