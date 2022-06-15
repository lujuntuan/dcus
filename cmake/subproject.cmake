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

macro(begin_sub_project)
    if(${ARGV0} MATCHES "lib" OR ${ARGV0} MATCHES "LIB")
        set(SUB_PROJECT_TYPE "lib")
    elseif(${ARGV0} MATCHES "plugin" OR ${ARGV0} MATCHES "PLUGIN")
        set(SUB_PROJECT_TYPE "plugin")
    elseif(${ARGV0} MATCHES "app" OR ${ARGV0} MATCHES "APP")
        set(SUB_PROJECT_TYPE "app")
    else()
        message(FATAL_ERROR "Sub project not support")
    endif()
    string(REGEX REPLACE ".*/(.*)" "\\1" PROJECT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR})
    if("${ARGV1}" STREQUAL "")
        set(SUB_PROJECT_NAME ${PROJECT_DIR_NAME})
    else()
        set(SUB_PROJECT_NAME ${PROJECT_DIR_NAME}_${ARGV1})
    endif()
    set(SUB_PROJECT_PLUGIN_PREFIX ${ARGV2})
    project(${CMAKE_PROJECT_NAME}_${SUB_PROJECT_NAME} VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES C CXX)
endmacro(begin_sub_project)

macro(end_sub_project)
    add_dependencies(${PROJECT_NAME} ${CMAKE_PROJECT_NAME})
    target_link_libraries(
        ${PROJECT_NAME}
        ${CMAKE_PROJECT_NAME}
        )
    if(${SUB_PROJECT_TYPE} MATCHES "lib")
        install(
            TARGETS
            ${PROJECT_NAME}
            EXPORT
            ${PROJECT_NAME}Config
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
        install(
            EXPORT
            ${PROJECT_NAME}Config
            DESTINATION
            lib/cmake/${PROJECT_NAME}
            )
        target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            ${DCUS_LIBRARY_DEF}
            )
        if(EXISTS ${PROJECT_SOURCE_DIR}/include)
            target_include_directories(
                ${PROJECT_NAME}
                PUBLIC
                $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
                )
            install(
                DIRECTORY
                ${PROJECT_SOURCE_DIR}/include/
                DESTINATION
                include
                )
        endif()
    elseif(${SUB_PROJECT_TYPE} MATCHES "plugin")
        install(
            TARGETS
            ${PROJECT_NAME}
            EXPORT
            ${PROJECT_NAME}Config
            RUNTIME
            DESTINATION
            lib/${SUB_PROJECT_PLUGIN_PREFIX}
            ARCHIVE
            DESTINATION
            lib/${SUB_PROJECT_PLUGIN_PREFIX}
            LIBRARY
            DESTINATION
            lib/${SUB_PROJECT_PLUGIN_PREFIX}
            )
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SUB_PROJECT_PLUGIN_PREFIX}
        )
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY
            ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${SUB_PROJECT_PLUGIN_PREFIX}
        )
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${SUB_PROJECT_PLUGIN_PREFIX}
        )
        set_target_properties(
            ${PROJECT_NAME}
            PROPERTIES
            PREFIX
            ""
        )
        target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            ${DCUS_LIBRARY_DEF}
            )
    elseif(${SUB_PROJECT_TYPE} MATCHES "app")
        install(
            TARGETS
            ${PROJECT_NAME}
            EXPORT
            ${PROJECT_NAME}Config
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
        target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            ${DCUS_LIBRARY_APPDEF}
            )
    endif()
    install_etc(${PROJECT_NAME})
    install_share(${PROJECT_NAME})
    unset(PROJECT_DIR_NAME)
    unset(SUB_PROJECT_TYPE)
    unset(SUB_PROJECT_NAME)
    unset(SUB_PROJECT_PLUGIN_PREFIX)
endmacro(end_sub_project)
