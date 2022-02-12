#pragma once
#include <string>
#include <vector>
#include "CrossPlat.h"
#include "Basic.h"

NAMESPACE_BASIC_BEGIN


static const char kWindowSplash = '\\';
static const char kLinuxSplash = '/';
#ifdef _WIN32
static const char kSeprator = kWindowSplash;
#else
static const char kSeprator = kLinuxSplash;
#endif // _WIN32

class File
{
    static const int kSuccess = 0;
    static const int kError = -1;

public:
    /*
    * @brief: check the end of directory
    * @data:  20200216
    * @update:
    * @para[in]:    strDir, the directory for checking
    * @return:      bool, end with splash to return true, or false
    */
    static bool CheckEndWithSplash(const std::string& strDir)
    {
        if ((strDir.back() != kWindowSplash)
            && (strDir.back() != kLinuxSplash))
            return false;
        return true;
    }

    /*
    * @brief: create single directory
    * @data:  20200207
    * @update:
    * @para[in]:    strDir, the directory for creating
    * @return:      bool, success to return true, or false
    */
    static bool CreateSingleDirectory(const std::string& strDir)
    {
        if (-1 == CG_access(strDir.c_str(), 0))
        {
            if (kSuccess == CG_mkdir(strDir.c_str()))
                return true;
            printf("Create directory:%s failed:", strDir.c_str());
            return false;
        }
        return true;
    }

    /*
    * @brief: create multi-directories
    * @data:  20200207
    * @update:
    * @para[in]:    strDir, the directory for creating
    * @return:      bool, success to return true, or false
    */
    static bool CreateMultiDirectory(const std::string& strDir)
    {
        std::string strTmpPath(strDir);
        if (!CheckEndWithSplash(strTmpPath))
            strTmpPath += kSeprator;

        int nPrePos = 0;
        int nCurrPos = 0;
        while (nCurrPos = strTmpPath.find_first_of(kSeprator, nCurrPos), nCurrPos != -1)
        {
            std::string strTmpDir = strTmpPath.substr(0, ++nCurrPos);
            if (kSuccess != CG_access(strTmpDir.c_str(), 0))
            {
                if (kSuccess != CG_mkdir(strTmpDir.c_str()))
                {
                    printf("Create directory:%s failed:", strDir.c_str());
                    return false;
                }
            }
            nPrePos = nCurrPos;
        }
        return true;
    }

#ifdef _WIN32
    /*
    * @brief: recursive files of directory
    * @data:  20200207
    * @update:
    * @para[in]:    strDir, the directory for searching
    * @para[out]:   vecFile, files of directory
    * @return:      bool, success to return true, or false
    */
    static void GetFilesOfDir(const std::string& strDir,
        std::vector<std::string>& vecFile,
        const std::string& strPatten = "*.*",
        bool bName = true)
    {
        if (strDir.empty()) return;
        std::string strSearchDir(strDir);
        if (!CheckEndWithSplash(strSearchDir))
            strSearchDir += kLinuxSplash;
        std::string strRootDir(strSearchDir);
        strSearchDir += strPatten;
        _finddata_t file;
        long long lf;
        if (kError == (lf = _findfirst(strSearchDir.c_str(), &file)))
            printf("Not found directory %s\n", strSearchDir.c_str());
        else
        {
            while (kSuccess == _findnext(lf, &file))
            {
                if (kSuccess == strcmp(file.name, ".")
                    || kSuccess == strcmp(file.name, ".."))
                    continue;
                if (file.attrib&_A_SUBDIR)
                {
                    std::string strSubDir(strRootDir + file.name);
                    GetFilesOfDir(strSubDir, vecFile);
                }
                else
                {
                    if(bName)
                        vecFile.push_back(file.name);
                    else
                    {
                        std::string strFilename(strRootDir + file.name);
                        vecFile.push_back(strFilename);
                    }
                }
            }
        }
        _findclose(lf);
    }

    /*
    * @brief: recursive directories of directory
    * @data:  20200215
    * @update:
    * @para[in]:    strDir, the directory for searching
    * @para[out]:   vecDir, directoriess of directory
    * @return:      bool, success to return true, or false
    */
    static void GetDirsOfDir(const std::string& strDir,
        std::vector<std::string>& vecDir,
        const std::string& strPatten = "*.*")
    {
        if (strDir.empty()) return;
        std::string strSearchDir(strDir);
        if (!CheckEndWithSplash(strSearchDir))
            strSearchDir += kLinuxSplash;
        strSearchDir += kLinuxSplash;
        std::string strRootDir(strSearchDir);
        strSearchDir += strPatten;
        _finddata_t file;
        long long lf;
        if (kError == (lf = _findfirst(strSearchDir.c_str(), &file)))
            printf("Not found directory %s\n", strSearchDir.c_str());
        else
        {
            while (kSuccess == _findnext(lf, &file))
            {
                if (kSuccess == strcmp(file.name, ".")
                    || kSuccess == strcmp(file.name, ".."))
                    continue;
                if (file.attrib&_A_SUBDIR)
                {
                    std::string strSubDir(strRootDir + file.name);
                    vecDir.push_back(strSubDir);
                    GetDirsOfDir(strSubDir, vecDir);
                }
            }
        }
        _findclose(lf);
    }

#else
    /*
    * @brief: recursive files of directory
    * @data:  20200207
    * @update:
    * @para[in]:    strDir, the directory for creating
    * @para[out]:   vecFile, files of directory
    * @return:      bool, success to return true, or false
    */
    static bool GetFilesOfDir(const std::string& strDir, std::vector<std::string>& vecFile)
    {
        if (strDir.empty()) return false;

        struct stat sateDir;
        stat(strDir.c_str(), &sateDir);
        if (!S_ISDIR(sateDir.st_mode)) return false;

        DIR* pOpenDir = opendir(strDir.c_str());
        if (nullptr == pOpenDir) return false;
        dirent* pDirent = nullptr;
        while ((pDirent = readdir(pOpenDir)) != nullptr)
        {
            if (pDirent->d_name[0] != '.')
            {
                std::string strTmpFile = dir_in + kSeprator + std::string(pDirent->d_name);
                struct stat st;
                stat(strTmpFile.c_str(), &st);
                if (S_ISDIR(st.st_mode))
                    GetFilesOfDir(strTmpFile, vecFile);
                else if (S_ISREG(st.st_mode))
                    vecFile.push_back(strTmpFile);
            }
        }
        closedir(pOpenDir);
    }

#endif // _WIN32


};
NAMESPACE_BASIC_END
