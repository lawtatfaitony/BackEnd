######################################################
#
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2021/01/05
# 
#######################################################


#add source files
recursive_add_source_files(SOURCE_LIST
    "cmake"
    "Src"
)

set(HEADER_LIST 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Hanz2Piny.h
)

