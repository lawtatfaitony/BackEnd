######################################################
#
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


## Retrieve add source files.
recursive_add_source_files(SOURCE_LIST
    "src"
    "cmake"
)

file(GLOB HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
