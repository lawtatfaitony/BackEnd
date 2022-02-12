######################################################
# @brief
# @author   Wite_Chen
# @date     2020/01/23
# 
#######################################################



cmake_minimum_required(VERSION 3.14)

# Set project name
set(BUILD_PROJECT_NAME "StorageServer")

# include the setting cmakes
#include(cmake/ice_compile_settings.cmake)
include(cmake/common_settings.cmake)
include(cmake/${FILE_CMAKE_ENV})

# Begin generate
message("\n")
message("---- Begin generate ${BUILD_PROJECT_NAME} ----")

# Step 1: Add source file
message("Step 1: Add source file")
message("Print the source file")
foreach(filepath ${SOURCE_LIST})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" fpath ${filepath})
    message(STATUS "${fpath}")
endforeach()

# Step 2: Add include directories
message("Step 2: Add include directories")
include_directories($<IF:$<CONFIG:Debug>,${PROJECT_INCLUDE_DIRECTORIES_DEBUG},${PROJECT_INCLUDE_DIRECTORIES_RELEASE}>)
message("Print the include directores")
get_property(inc_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(inc_dir ${inc_dirs})
    message(STATUS "[INCLUDE DIRECTORYS]  ${inc_dir}")
endforeach()

# Step 3: Add library directories
message("Step 3: Add library directories")
link_directories($<IF:$<CONFIG:Debug>,${PROJECT_LINK_DIRECTORIES_DEBUG},${PROJECT_LINK_DIRECTORIES_RELEASE}>)
message("Print the link directores")
get_property(link_dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
foreach(link_dir ${link_dirs})
    message(STATUS "[LINK DIRECTORYS]  ${link_dir}")
endforeach()

# Step 4: Add definitions
message("Step 4: Add definitions")
add_definitions(${PROJECT_DEFINITIONS})
message("Print the definitions")
get_property(definitions DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
foreach(define ${definitions})
    message(STATUS "[DEFINITIONS]  ${define}")
endforeach()

# Step 5: Add executable or library target
message("Step 5: Add executable or library target")
add_executable(${BUILD_PROJECT_NAME} ${SOURCE_LIST})
message(STATUS "target name: ${BUILD_PROJECT_NAME}")

# Step 6: Set up the group
message("Step 6: Set up the group")
set_property(TARGET ${BUILD_PROJECT_NAME} PROPERTY FOLDER ${PROJECT_FOLDER_APP})
message(STATUS "target group: ${PROJECT_FOLDER_APP}")

# Step 7: Add share link dependent libraries
message("Step 7: Add share link dependent libraries")
set_property(TARGET ${BUILD_PROJECT_NAME}
    PROPERTY
    LINK_LIBRARIES $<IF:$<CONFIG:Debug>,${PROJECT_LINK_LIBRARIES_DEBUG},${PROJECT_LINK_LIBRARIES_RELEASE}>)
message("Print the static link dependent libraries")
get_property(sdepe_libs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_LIBRARIES)
foreach(sdepe ${sdepe_libs})
    message(STATUS "[LINK LIBRARIES]  ${sdepe}")
endforeach()

# Step 8: Add static dependent libraries
message("Step 8: Add static dependent libraries")
target_link_libraries(${BUILD_PROJECT_NAME} ${PROJECT_DEPENDENCY})
message("Print the target dependent libraries")
foreach(tdepe_lib ${PROJECT_DEPENDENCY})
    message(STATUS "[DEPENDENCY]  ${tdepe_lib}")
endforeach()

# Step 9: Set vs debug environment.
if(MSVC)
    set_property(TARGET ${BUILD_PROJECT_NAME}
        PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_property(TARGET ${BUILD_PROJECT_NAME}
        PROPERTY
        VS_DEBUGGER_ENVIRONMENT $<IF:$<CONFIG:Debug>,${DEBUGGER_ENVIRONMENT_DEBUG},${DEBUGGER_ENVIRONMENT_RELEASE}>
    )
    message("Step 9: Set vs debug environment")
    message("Print working directory")
    message(STATUS "[WORKING DIRECTORY]  ${CMAKE_CURRENT_SOURCE_DIR}")
    message("Print debug debugger environment")
    foreach(env ${DEBUGGER_ENVIRONMENT_DEBUG})
        message(STATUS "[ENVIRONMENT]  ${env}")
    endforeach()
    message("Print release debugger environment")
    foreach(env ${DEBUGGER_ENVIRONMENT_RELEASE})
        message(STATUS "[ENVIRONMENT]  ${env}")
    endforeach()
endif()

message("---- End generate ${BUILD_PROJECT_NAME} ----\n\n")
