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

begin_rpc_project()

macro(add_dcus_interfaces)
    #add_dependencies(${PROJECT_NAME} ${RPC_INTERFACE_NAME})
    find_package(${RPC_INTERFACE_NAME} QUIET)
    ament_target_dependencies(
        ${PROJECT_NAME}
        rclcpp
        )
    if(${RPC_INTERFACE_NAME}_FOUND)
        ament_target_dependencies(
            ${PROJECT_NAME}
            ${RPC_INTERFACE_NAME}
            )
    else()
        rosidl_get_typesupport_target(
            ROS_IDL_TYPE
            ${RPC_INTERFACE_NAME}
            "rosidl_typesupport_cpp"
        )
        target_link_libraries(
            ${PROJECT_NAME}
            "${ROS_IDL_TYPE}"
            )
    endif()
endmacro(add_dcus_interfaces)

add_subdirectory(interfaces)
if(DCUS_BUILD_SERVER)
    add_subdirectory(server)
endif()
if(DCUS_BUILD_CLIENT)
    add_subdirectory(client)
endif()

end_rpc_project()
