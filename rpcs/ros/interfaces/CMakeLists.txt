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

project(${RPC_INTERFACE_NAME} VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES C CXX)

find_package(${PROJECT_NAME} QUIET)
if(NOT ${PROJECT_NAME}_FOUND)
    find_package(rosidl_default_generators REQUIRED)
    file(GLOB_RECURSE
        DCUS_ROS_IDL_SOURCE
        RELATIVE
        ${CMAKE_CURRENT_LIST_DIR}
        msg/*
        )
    rosidl_generate_interfaces(${PROJECT_NAME}
        ${DCUS_ROS_IDL_SOURCE}
        )
    ament_package()
    unset(DCUS_ROS_IDL_SOURCE)
endif()
