/**************************************************************
*
* @brief:       file management
* @date:        20201217
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <memory>
#include <list>
#include <Basic/ThreadObject.h>
#include <Basic/Singleton.h>
#include <Basic/SafeMap.h>
#include "FileHandle.h"



class FileManagement : public Basic::Singleton<FileManagement>
{
private:
    // const value
    const int kStartPos = 0;
    const int kCheckDate = 1000;
    const std::string kRootDir = "Data/";
    const int kMaxFile = 1024;


public:
    ~FileManagement();
    bool Init();
    void Uninit();
    std::string SaveFile(const std::string& strData, FileType nType = kFileTypeNone);
    std::string SaveSingleFile(const FileInfo& infoFile);
    void ReadFile(const std::string& strPath, FileInfo& infoFile);


private:
    void run();
    void add_new_handle();
    std::string generate_path(std::string& strFile, int nSlice, int& nIndex);
    std::shared_ptr<FileHandle> get_file_handle(const std::string& strFile);
    void push_file(const std::string& strFile, const FileInfo& infoFile);
    bool parse_path(const std::string& strPath, std::string& strFile, int& nFile);

private:
    static std::map<FileType, std::string> g_mapFileType;
    Basic::SafeMap<std::string, std::shared_ptr<FileHandle>> m_mapHandle;
    Basic::ThreadObject m_objThread;

    // file paras
    std::string m_strCurrDate;
    std::string m_strFile;
    std::atomic<int> m_nFile;
    std::mutex m_mtLock;

};