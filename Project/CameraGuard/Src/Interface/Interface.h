#pragma once
#include <map>
#include <string>


namespace Interface
{
    std::map<std::string, std::string> g_mapFunc
    {
        // user
        { "/Login","handle_login" },
        { "/RegisterUser","handle_register_user" },
        { "/UpdateUser","handle_update_user" },
		{ "/UserDetails","handle_details_user" },
        { "/DeleteUser","handle_delete_user" },
        // library
        { "/AddLibrary","handle_add_library" },
        { "/DeleteLibrary","handle_delete_library" },
        { "/QueryLibraryList","handle_query_library_list" },
        // person
        { "/AddPerson","handle_add_person" },
        { "/DeletePerson","handle_delete_person" },
        { "/UpdatePerson","handle_update_person" },
        { "/QueryPersonList","handle_query_person_list" },
        // camera
        { "/AddCamera","handle_add_camera" },
        { "/DeleteCamera","handle_delete_camera" },
        { "/UpdateCamera","handle_update_camera" },
        { "/QueryCameraList","handle_query_camera_list" },
        { "/SearchCamera","handle_search_camera" },
        { "/QueryCameraRtsp","handle_query_camera_rtsp" },
        // task
        { "/AddTask","handle_add_task" },
        { "/DeleteTask","handle_delete_task" },
        { "/UpdateCamera","handle_update_task" },
        { "/QueryTaskList","handle_query_task_list" },
        { "/StartTask","handle_start_task" },
        { "/StopTask","handle_stop_task" },
        // config
        { "/GetSystemConfig","get_system_config" },
        { "/UpdateSystemConfig","update_system_config" }

    };
}
