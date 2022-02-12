######################################################
# @brief    ICE compile of cmake
# @author   Wite_Chen
# @date     2020/02/01
# 
#######################################################


## Retrieve ice files.
retrieve_files(ICE_FILES "Ice Files"
    "*.ice"
)

## Create ice generated directory
create_directory(${CMAKE_CURRENT_SOURCE_DIR}/${ICE_GENERATED_FOLDER})


## Compile ice files.
message("Compile ice files...")
foreach(ice_file ${ICE_FILES})
    execute_process(COMMAND "${ENV_ICE}/tools/slice2cpp.exe"
        --underscore
        "-I${ENV_ICE}/slice"
        --output-dir "${CMAKE_CURRENT_SOURCE_DIR}/${ICE_GENERATED_FOLDER}"
        "${ice_file}"
    )
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" fpath ${ice_file})
    string(REGEX REPLACE "(.+)/(.+)\.ice$" "\\2" fname ${ice_file})
    message(STATUS "[COMPILE ICE]  Compiling '${fpath}' Generating -> "
        "${ICE_GENERATED_FOLDER}/${fname}.h and ${ICE_GENERATED_FOLDER}/${fname}.cpp")
endforeach()
