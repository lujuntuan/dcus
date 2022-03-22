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

macro(start_sub_project)
    if(${ARGV0} MATCHES "lib" OR ${ARGV0} MATCHES "LIB")
        set(SUB_PROJECT_TYPE "lib")
    else()
        set(SUB_PROJECT_TYPE "app")
    endif()
    string(REGEX REPLACE ".*/(.*)" "\\1" PROJECT_DIR_NAME ${CMAKE_CURRENT_LIST_DIR})
    if("${ARGV1}" STREQUAL "")
        set(SUB_PROJECT_NAME ${PROJECT_DIR_NAME})
    else()
        set(SUB_PROJECT_NAME ${PROJECT_DIR_NAME}_${ARGV1})
    endif()
    project(${CMAKE_PROJECT_NAME}_${SUB_PROJECT_NAME} VERSION ${CMAKE_PROJECT_VERSION} LANGUAGES C CXX)
endmacro(start_sub_project)

macro(end_sub_project)
    add_dependencies(${PROJECT_NAME} ${CMAKE_PROJECT_NAME})
    target_link_libraries(
        ${PROJECT_NAME}
        ${CMAKE_PROJECT_NAME}
        )
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
    if(${SUB_PROJECT_TYPE} MATCHES "lib")
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
                PRIVATE
                ${PROJECT_SOURCE_DIR}/include
                )
            install(
                DIRECTORY
                ${PROJECT_SOURCE_DIR}/include/
                DESTINATION
                include
                )
        endif()
    else()
        target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            ${DCUS_LIBRARY_APPDEF}
            )
    endif()
    unset(PROJECT_DIR_NAME)
    unset(SUB_PROJECT_TYPE)
    unset(SUB_PROJECT_NAME)
endmacro(end_sub_project)
