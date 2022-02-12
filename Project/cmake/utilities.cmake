######################################################
# @brief    utilities of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


# recursive_add_source_files(<out-var> [<directory>...])
function(recursive_add_source_files out_var)
    set(source_list)
    foreach(dirname ${ARGN})
        file(GLOB_RECURSE filelist RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
            "${dirname}/*.h"
            "${dirname}/*.h2"
            "${dirname}/*.hpp"
            "${dirname}/*.c"
            "${dirname}/*.cpp"
            "${dirname}/*.cc"
            "${dirname}/*.cxx"
            "${dirname}/*.ice"
            "${dirname}/*.wsdl"
            "${dirname}/*.rc"
            "${dirname}/*.client"
            "${dirname}/*.server"
            "${dirname}/*.ini"
            "${dirname}/*.cfg"
            "${dirname}/*.conf"
            "${dirname}/*.config"
            "${dirname}/*.json"
            "${dirname}/*.cmake"
            "${dirname}/*.dat"
            "${dirname}/*.rc.in"
            "${dirname}/*.h.in"
        )
        foreach(filename ${filelist})
            list(APPEND source_list "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
            string(FIND ${filename} "/" ret)
            if (${ret} GREATER 0)
                string(REGEX REPLACE "(.+)/(.+\..+)$" "\\1" fpath ${filename})
                string(REPLACE "/" "\\\\" group ${fpath})
                source_group(${group} FILES "${CMAKE_CURRENT_SOURCE_DIR}/${filename}")
            endif()
        endforeach()
    endforeach()
    set(${out_var} ${source_list} PARENT_SCOPE)
endfunction()


# retrieve_files(<out-var> <directory> [<expressions>...])
function(retrieve_files out_var directory)
    set(expressions)
    foreach(exp ${ARGN})
        list(APPEND expressions "${directory}/${exp}")
    endforeach()
    set(file_list)
    file(GLOB file_list ${expressions})
    set(${out_var} ${file_list} PARENT_SCOPE)
endfunction()


# build_event_copy_file(<source> <destination>)
function(build_event_copy_file source destination)
    if(MSVC)
        string(REPLACE "/" "\\" source "${source}")
        string(REPLACE "/" "\\" destination "${destination}")
        add_custom_command(TARGET ${BUILD_PROJECT_NAME}
            POST_BUILD
            COMMAND copy /Y "\"${source}\"" "\"${destination}\""
        )
    elseif(UNIX)
        string(REPLACE "\\" "/" source "${source}")
        string(REPLACE "\\" "/" destination "${destination}")
        add_custom_command(TARGET ${BUILD_PROJECT_NAME}
            POST_BUILD
            COMMAND cp -f "\"${source}\"" "\"${destination}\""
        )
    endif()
endfunction()


# build_event_generate_version(<template_file> <version_file>)
function(build_event_generate_version template_file version_file)
    if(MSVC)
        string(REPLACE "/" "\\" template_file "${template_file}")
        string(REPLACE "/" "\\" version_file "${version_file}")
        add_custom_command(TARGET ${BUILD_PROJECT_NAME}
            PRE_BUILD
            COMMAND SubWCRev \"${CMAKE_CURRENT_SOURCE_DIR}\" \"${template_file}\" \"${version_file}\"
        )
    elseif(UNIX)
        string(REPLACE "\\" "/" template_file "${template_file}")
        string(REPLACE "\\" "/" version_file "${version_file}")
        add_custom_command(TARGET ${BUILD_PROJECT_NAME}
            PRE_BUILD
            COMMAND sed 's/\\$$WCREV\\$$/'\$$\(svn info \"${CMAKE_CURRENT_SOURCE_DIR}\" | grep \"Last Changed Rev:\"
             | awk '{print $$NF}'\)'/g\;s/\\$$WCNOW=.*\\$$/'`date \"+%Y%m%dT%H%M%S\"`'/g'
             \"${template_file}\" >\"${version_file}\"
        )
    endif()
endfunction()


# modify_default_compiler_flags([<compiler flags>...])
function(modify_default_compiler_flags)
    message("modify default compiler flags...")
    foreach(compiler_flags ${ARGN})
#        string(REPLACE "/MD" "/MT" ${compiler_flags} "${${compiler_flags}}")
#        string(REPLACE "/MDd" "/MTd" ${compiler_flags} "${${compiler_flags}}")
#        set(${compiler_flags} "${${compiler_flags}}" PARENT_SCOPE)
        message(STATUS "[FLAGS]  ${compiler_flags}")
        foreach(flag ${${compiler_flags}})
            message(STATUS "         ${flag}")
        endforeach()
    endforeach()
endfunction()


# create directory
function(create_directory dir)
    if(NOT EXISTS ${dir})
        file(MAKE_DIRECTORY ${dir})
        message("Create directory: ${dir}")
        message(STATUS "[DIRECTORY] ${dir}")
    endif()
endfunction()


# generate project version
function(modify_project_version)
    string(TIMESTAMP PROJECT_BUILD_YEAR "%Y")
    string(TIMESTAMP PROJECT_BUILD_DATE "%Y%m%d")
    string(TIMESTAMP PROJECT_BUILD_TIME "%H%M%S")
    SET(PROJECT_REVISION 0)
    # get svn/git commit reversion
    if(EXISTS "${PROJECT_STORE_PATH}/.git/")
        find_package(Git)
        if(GIT_FOUND)
            execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE res_code OUTPUT_VARIABLE GIT_COMMIT_ID)
            if(${res_code} EQUAL 0)
                message("-- Get git revision success")
                # -g: tag of git
                string(FIND  ${GIT_COMMIT_ID} "-g" pos)
                if(${pos} GREATER 0)
                    string(SUBSTRING ${GIT_COMMIT_ID} ${pos} -1 COMMIT_ID)
                    string(SUBSTRING ${COMMIT_ID} 2 -1 PROJECT_REVISION)
                    message("-- Git commit id: ${PROJECT_REVISION}")
                endif()
            else(${res_code} EQUAL 0)
                message( WARNING "-- Git failed (not a repo, or no tags). Build will not contain git revision info." )
            endif(${res_code} EQUAL 0)
        else(GIT_FOUND)
            message("-- Git not found!)")
        endif(GIT_FOUND)
    else(EXISTS "${PROJECT_STORE_PATH}/.git/")
        if(EXISTS "${PROJECT_STORE_PATH}/.svn/")
            FIND_PACKAGE(Subversion)
            if(SUBVERSION_FOUND)
                Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} Project)
                SET(PROJECT_REVISION ${Project_WC_REVISION})
                message("-- Svn revision:${PROJECT_REVISION}")
            else(SUBVERSION_FOUND)
                message("-- Can't find packet Subversion")
            endif(SUBVERSION_FOUND)
        else()
            message(ERROR "-- Svn directory not exists")
        endif(EXISTS "${PROJECT_STORE_PATH}/.svn/")
    endif(EXISTS "${PROJECT_STORE_PATH}/.git/")

    # generate the version file
    set(VERSION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Version/Version.h)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Version/Version.h.in"
                "${VERSION_FILE}"
    @ONLY)

endfunction()