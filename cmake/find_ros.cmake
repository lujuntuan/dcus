#*********************************************************************************
#  *Copyright(C): Juntuan.Lu 2022
#  *Author:  Juntuan.Lu
#  *Version: 1.0
#  *Date:  2022/04/01
#  *Phone: 15397182986
#  *Description:
#  *Others:
#  *Function List:
#  *History:
#**********************************************************************************

find_package(ament_cmake QUIET)
find_package(rclcpp QUIET)

function(rosidl_target_interfaces_private target interface_target typesupport_name)
    if(ARGN)
        message(FATAL_ERROR
            "rosidl_target_interfaces() called with unused arguments: ${ARGN}")
    endif()
    if(NOT TARGET ${target})
        message(FATAL_ERROR "rosidl_target_interfaces() the first argument '${target}' must be a valid target name")
    endif()
    if(NOT TARGET ${interface_target})
        message(FATAL_ERROR "rosidl_target_interfaces() the second argument '${interface_target}' must be a valid target name")
    endif()
    set(typesupport_target "${interface_target}__${typesupport_name}")
    if(NOT TARGET ${typesupport_target})
        message(FATAL_ERROR
            "rosidl_target_interfaces() the second argument '${interface_target}' "
            "concatenated with the third argument '${typesupport_name}' "
            "using double underscores must be a valid target name")
    endif()

    add_dependencies(${target} ${interface_target})
    get_target_property(include_directories ${typesupport_target} INTERFACE_INCLUDE_DIRECTORIES)
    #  target_include_directories(${target} PUBLIC ${include_directories})
    #  target_link_libraries(${target} ${typesupport_target})
    target_include_directories(${target} PRIVATE ${include_directories})
    target_link_libraries(${target} $<BUILD_INTERFACE:${typesupport_target}>)
endfunction()

if(ament_cmake_FOUND AND rclcpp_FOUND)
    set(ROS_ENABLE ON)
    message(STATUS "rclcpp found")
    message(STATUS "")
else()
    set(ROS_ENABLE OFF)
    message(STATUS "rclcpp not found")
endif()
