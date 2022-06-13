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

if(ROS_ENABLE)
    set(DCUS_EXAMPLES_LNK_PLUGIN "ros" CACHE STRING "Examples link plugin name")
elseif(VSOMEIP_ENABLE)
    set(DCUS_EXAMPLES_LNK_PLUGIN "vsomeip" CACHE STRING "Examples link plugin name")
elseif(FDBUS_ENABLE)
    set(DCUS_EXAMPLES_LNK_PLUGIN "fdbus" CACHE STRING "Examples link plugin name")
endif()

string(TOLOWER ${DCUS_EXAMPLES_LNK_PLUGIN} DCUS_EXAMPLES_LNK_PLUGIN)

if(${DCUS_EXAMPLES_LNK_PLUGIN} MATCHES "ros")
    message("** Build examples for ${DCUS_EXAMPLES_LNK_PLUGIN}.")
elseif(${DCUS_EXAMPLES_LNK_PLUGIN} MATCHES "vsomeip")
    message("** Build examples for ${DCUS_EXAMPLES_LNK_PLUGIN}.")
elseif(${DCUS_EXAMPLES_LNK_PLUGIN} MATCHES "fdbus")
    message("** Build examples for ${DCUS_EXAMPLES_LNK_PLUGIN}.")
else()
    message(FATAL_ERROR "** Can not find plugin library(name:${DCUS_EXAMPLES_LNK_PLUGIN}) !")
endif()

macro(add_example_library)
    target_link_libraries(
        ${PROJECT_NAME}
        dcus_${DCUS_EXAMPLES_LNK_PLUGIN}_client
        )
endmacro(add_example_library)

add_subdirectory(client_simple)
if(QTWIDGETS_ENABLE)
    add_subdirectory(client_viewer)
endif()
