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

option(PROTOBUF_USE_C "" OFF)

begin_3rdparty_search(protobuf)
find_program(PROTOBUF_GENERATOR_EXE
    NAMES
    protoc
    PATHS
    ${3RD_EXE_SEARCH_PATH}
    ENV PATH
    NO_CMAKE_PATH
    )
find_path(PROTOBUF_INCLUDE_DIR
    NAMES
    google/protobuf/message.h
    PATHS
    ${3RD_INC_SEARCH_PATH}
    )
find_library(PROTOBUF_LIBRARY
    NAMES
    protobuf
    libprotobuf
    PATHS
    ${3RD_LIB_SEARCH_PATH}
    )
end_3rdparty_search()

if(PROTOBUF_GENERATOR_EXE AND PROTOBUF_INCLUDE_DIR AND PROTOBUF_LIBRARY)
    set(PROTOBUF_ENABLE ON)
    message(STATUS "protobuf found")
    message(STATUS "PROTOBUF_GENERATOR_EXE=${PROTOBUF_GENERATOR_EXE}")
    message(STATUS "PROTOBUF_INCLUDE_DIR=${PROTOBUF_INCLUDE_DIR}")
    message(STATUS "PROTOBUF_LIBRARY=${PROTOBUF_LIBRARY}")
else()
    set(PROTOBUF_ENABLE OFF)
    message(STATUS "protobuf not found")
endif()

if(PROTOBUF_USE_C)
    begin_3rdparty_search(protobuf_c)
    find_program(PROTOBUF_C_GENERATOR_EXE
        NAMES
        protoc-c
        PATHS
        ${3RD_EXE_SEARCH_PATH}
        ENV PATH
        NO_CMAKE_PATH
        )
    find_path(PROTOBUF_C_INCLUDE_DIR
        NAMES
        protobuf-c/protobuf-c.h
        PATHS
        ${3RD_INC_SEARCH_PATH}
        )
    find_library(PROTOBUF_C_LIBRARY
        NAMES
        protobuf-c
        libprotobuf-c
        PATHS
        ${3RD_LIB_SEARCH_PATH}
        )
    end_3rdparty_search()
    
    if(PROTOBUF_C_GENERATOR_EXE AND PROTOBUF_C_INCLUDE_DIR AND PROTOBUF_C_LIBRARY)
        set(PROTOBUF_C_ENABLE ON)
        message(STATUS "protobuf-c found")
        message(STATUS "PROTOBUF_C_GENERATOR_EXE=${PROTOBUF_C_GENERATOR_EXE}")
        message(STATUS "PROTOBUF_C_INCLUDE_DIR=${PROTOBUF_C_INCLUDE_DIR}")
        message(STATUS "PROTOBUF_C_LIBRARY=${PROTOBUF_C_LIBRARY}")
    else()
        set(PROTOBUF_C_ENABLE OFF)
        message(STATUS "protobuf-c not found")
    endif()
endif()

function(proto_generate_src _out_var)
    if(${ARGC} LESS 2)
        message(FATAL_ERROR "Args error")
    endif()
    set(_source ${ARGV})
    set(_file_sources)
    list(REMOVE_AT _source 0)
    foreach(it ${_source})
        file(GLOB_RECURSE
            _file
            ${it}
            )
        list(APPEND _file_sources ${_file})
    endforeach()
    if(NOT _file_sources)
        message(FATAL_ERROR "Can not find any sources")
    endif()
    foreach(it ${_file_sources})
        set(outdir ${CMAKE_CURRENT_BINARY_DIR})
        get_filename_component(filedir ${it} DIRECTORY)
        get_filename_component(infile ${it} ABSOLUTE)
        get_filename_component(outfilename ${it} NAME_WE)
        list(APPEND outfile ${outdir}/${outfilename}.pb.h ${outdir}/${outfilename}.pb.cc)
        if(PROTOBUF_GENERATOR_EXE)
            add_custom_command(
                OUTPUT ${outfile}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir}
                COMMAND ${PROTOBUF_GENERATOR_EXE} -I${filedir} --cpp_out=${outdir} ${it}
                DEPENDS ${infile}
                VERBATIM
                )
        endif()
        if(PROTOBUF_C_GENERATOR_EXE AND PROTOBUF_USE_C)
            add_custom_command(
                OUTPUT ${outfile}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${outdir}
                COMMAND ${PROTOBUF_C_GENERATOR_EXE} -I${filedir} --c_out=${outdir} ${it}
                DEPENDS ${infile}
                VERBATIM
                )
        endif()
        list(APPEND outfiles ${outfile})
    endforeach()
    set_source_files_properties(${outfiles} PROPERTIES GENERATED TRUE)
    set(${_out_var} ${outfiles} PARENT_SCOPE)
endfunction(proto_generate_src)
