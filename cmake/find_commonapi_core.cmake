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

find_package(CommonAPI QUIET)

start_3rdparty_search(CommonAPI)
if(CMAKE_HOST_WIN32)
    find_program(COMMONAPI_CORE_GENERATOR_EXE
        NAMES
        capicxx-core-gen
        commonapi-core-generator-windwos-x86
        commonapi-core-generator-windwos-x86_64
        PATHS
        ${3RD_EXE_SEARCH_PATH}
        ENV PATH
        NO_CMAKE_PATH
        )
elseif(CMAKE_HOST_UNIX)
    find_program(COMMONAPI_CORE_GENERATOR_EXE
        NAMES
        capicxx-core-gen
        commonapi-core-generator-linux-x86
        commonapi-core-generator-linux-x86_64
        PATHS
        ${3RD_EXE_SEARCH_PATH}
        ENV PATH
        NO_CMAKE_PATH
        )
endif()
find_path(COMMONAPI_CORE_INCLUDE_DIR
    NAMES
    CommonAPI/CommonAPI.h
    CommonAPI/CommonAPI.hpp
    PATHS
    ${3RD_INC_SEARCH_PATH}
    PATH_SUFFIXES
    ""
    CommonAPI-3.2
    )
find_library(COMMONAPI_CORE_LIBRARY
    NAMES
    CommonAPI
    libCommonAPI
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(CommonAPI_FOUND OR (COMMONAPI_CORE_INCLUDE_DIR AND COMMONAPI_CORE_LIBRARY))
    set(COMMONAPI_CORE_ENABLE ON)
    if(COMMONAPI_CORE_GENERATOR_EXE)
        message(STATUS "COMMONAPI_CORE_GENERATOR_EXE=${COMMONAPI_CORE_GENERATOR_EXE}")
    endif()
    if(COMMONAPI_CORE_INCLUDE_DIR AND COMMONAPI_CORE_LIBRARY)
        message(STATUS "COMMONAPI_CORE_INCLUDE_DIR=${COMMONAPI_CORE_INCLUDE_DIR}")
        message(STATUS "COMMONAPI_CORE_LIBRARY=${COMMONAPI_CORE_LIBRARY}")
    else()
        message(STATUS "")
    endif()
    message(STATUS "CommonAPI found")
else()
    set(COMMONAPI_CORE_ENABLE OFF)
    message(STATUS "CommonAPI not found")
endif()

function(commonapi_core_generate_src _out_var _opt_value)
    if(${ARGC} LESS 3)
        message(FATAL_ERROR "Args error")
    endif()
    set(_source ${ARGV})
    set(_file_sources)
    list(REMOVE_AT _source 0 1)
    foreach(it ${_source})
        file(GLOB_RECURSE
            _file
            ${it}
            )
        list(APPEND _file_sources ${_file})
    endforeach()
    if(NOT _opt_value)
        message(FATAL_ERROR "Opt value not set")
    endif()
    if(NOT _file_sources)
        message(FATAL_ERROR "Can not find any sources")
    endif()
    foreach(it ${_file_sources})
        set(outdir ${CMAKE_CURRENT_BINARY_DIR})
        get_filename_component(filedir ${it} DIRECTORY)
        get_filename_component(infile ${it} ABSOLUTE)
        get_filename_component(outfilename ${it} NAME_WE)
        list(APPEND outfile
            ${outdir}/${_opt_value}.hpp
            ${outdir}/${_opt_value}Proxy.hpp
            ${outdir}/${_opt_value}ProxyBase.hpp
            ${outdir}/${_opt_value}Stub.hpp
            ${outdir}/${_opt_value}StubDefault.hpp
            )
        add_custom_command(
            OUTPUT ${outfile}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir}
            COMMAND ${COMMONAPI_CORE_GENERATOR_EXE} ${it} -sk --dest=${outdir}
            DEPENDS ${infile}
            VERBATIM
            )
        list(APPEND outfiles ${outfile})
    endforeach()
    set_source_files_properties(${outfiles} PROPERTIES GENERATED TRUE)
    set(${_out_var} ${outfiles} PARENT_SCOPE)
endfunction(commonapi_core_generate_src)
