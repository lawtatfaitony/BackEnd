######################################################
# @brief    
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


cmake_minimum_required(VERSION 3.14)

project(CameraGuard)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set the ice storage path
set(ICE_GENERATED_FOLDER "generated")

# set the basic include path
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Include")


# set the project group
set(PROJECT_FOLDER_APP "App")
set(PROJECT_FOLDER_STATIC "Static")
set(PROJECT_FOLDER_SHARE "Shared")
set(PROJECT_FOLDER_TEST "Test")


# include the other cmake files
include(cmake/global_setting.cmake)
include(cmake/utilities.cmake)

# set the path to store the target
set(OUTPUT_TARGET_DIR Output)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${OUTPUT_TARGET_DIR})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${OUTPUT_TARGET_DIR}) 

# cache the root path for project
message("${CMAKE_SOURCE_DIR}")
string(REGEX REPLACE "(.*)/(.*)/*" "\\1" PROJECT_STORE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# add the projects
add_subdirectory(Cmake)
add_subdirectory(CameraGuard)
add_subdirectory(StorageServer)
add_subdirectory(CompareServer)
add_subdirectory(TaskServer)
add_subdirectory(Basic)

# add libraries
add_subdirectory(Library/mongoose)
add_subdirectory(Library/easylogging)
add_subdirectory(Library/CameraSearch)
add_subdirectory(Library/Hanz2Piny)

# add tests
add_subdirectory(Test/OnvifTest)
add_subdirectory(Test/SociTest)
add_subdirectory(Test/CurlTest)