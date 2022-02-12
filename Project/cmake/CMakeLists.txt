######################################################
#
# @brief
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################
cmake_minimum_required(VERSION 3.14)


# cmake version required
cmake_minimum_required(VERSION 3.14)


# set project name
set(BUILD_PROJECT_NAME "cmake_setting")


file(GLOB_RECURSE CMAKE_FILE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.cmake)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
message("--GENERATED ${BUILD_PROJECT_NAME}")
foreach(file ${CMAKE_FILE_LIST})
    message("cmake file: "  ${file})
endforeach()

# source group
source_group(src FILES ${SOURCE_FILES})
source_group(cmake FILES ${CMAKE_FILE_LIST})

# add target
add_executable(${BUILD_PROJECT_NAME} ${CMAKE_FILE_LIST} ${SOURCE_FILES})

# set up the group
message("--Set up the group")
set_property(TARGET 
    ${BUILD_PROJECT_NAME}
    PROPERTY FOLDER
    ${PROJECT_FOLDER_APP}
)
message(STATUS "target group: ${PROJECT_FOLDER_APP}")