#pragma once
#ifdef _WIN32
    #include <io.h>
    #include <direct.h>
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <sys/types.h>

#endif // _WIN32




#ifdef _WIN32
#define CG_access(file,mode) _access(file,mode)
#define CG_local_time(time,tm) localtime_s(&tm, &time)
#define CG_mkdir(dir) _mkdir(dir)
#define CG_remove(file) remove(file)


#else
#define CG_access(file,mode) access(file,mode)
#define CG_local_time(time,tm) localtime_r(&time, &tm)
#define CG_mkdir(dir) mkdir(dir, 775)


#endif // _WIN32

