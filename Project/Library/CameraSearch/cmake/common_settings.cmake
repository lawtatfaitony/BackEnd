######################################################
#
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2020/01/14
# 
#######################################################


#add source files
recursive_add_source_files(SOURCE_LIST
    "cmake"
    "Src"
)

set(HEADER_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CameraSearchDefins.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CameraSearch.h
)

