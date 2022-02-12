######################################################
#
# @brief    common seting of cmake
# @author   Wite_Chen
# @date     2020/01/11
# 
#######################################################


#add source files
recursive_add_source_files(SOURCE_LIST
    "cmake"
    "Src"
)

# set pre-compile options
set(PROJECT_DEFINITIONS
    ${PROJECT_DEFINITIONS}
    -DMG_ENABLE_HTTP_STREAMING_MULTIPART
)

set(HEADER_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/mongoose.h)

