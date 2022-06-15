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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(COMMON_ETC_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/output/etc)
set(COMMON_SHARE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/output/share)
set(COMMON_CMAKE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/output/cmake)
set(COMMON_INCLUDE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/output/include)

macro(get_git_commit _git_hash)
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%H
            OUTPUT_VARIABLE ${_git_hash}
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
            )
    endif()
endmacro()

macro(install_etc _target)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/etc)
        return()
    endif()
    if("${_target}" STREQUAL "")
        message(FATAL_ERROR "Target not set")
    endif()
    add_custom_command(
        TARGET
        ${_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/etc/ ${COMMON_ETC_OUTPUT_PATH}
        )
    get_filename_component(_PREFIX_RELATIVE_PATH ${CMAKE_INSTALL_PREFIX} REALPATH)
    if(${_PREFIX_RELATIVE_PATH} MATCHES "^/usr")
        #use root
        install(
            DIRECTORY
            ${PROJECT_SOURCE_DIR}/etc/
            DESTINATION
            /etc
            )
    else()
        install(
            DIRECTORY
            ${PROJECT_SOURCE_DIR}/etc/
            DESTINATION
            etc
            )
    endif()
    unset(_PREFIX_RELATIVE_PATH)
endmacro()

macro(install_share _target)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/share)
        return()
    endif()
    if("${_target}" STREQUAL "")
        message(FATAL_ERROR "Target not set")
    endif()
    add_custom_command(
        TARGET
        ${_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/share/ ${COMMON_SHARE_OUTPUT_PATH}
        )
    install(
        DIRECTORY
        ${PROJECT_SOURCE_DIR}/share/
        DESTINATION
        share/${CMAKE_PROJECT_NAME}
        )
    unset(_PREFIX_RELATIVE_PATH)
endmacro()
