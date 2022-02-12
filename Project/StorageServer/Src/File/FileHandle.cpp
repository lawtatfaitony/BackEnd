#include "FileHandle.h"
#include <Macro.h>


FileHandle::FileHandle()
{

}

FileHandle::FileHandle(const std::string& strPath,
    const std::string& strFileName,
    bool bRead)
{
    // open file to read/write
    std::string strFile = strPath + strFileName;
    auto mode = std::ios::in | std::ios::binary;
    if (!bRead)
    {
        mode |= std::ios::out;
        mode |= std::ios::app;
    }
    m_hFile.open(strFile.c_str(), mode);
    if (m_hFile.is_open())
    {
        m_hFile.seekg(0, std::ios::end);
        int64_t nSize = m_hFile.tellg();
        if (nSize > 0)
        {
            // cache the file data
            m_strData.resize(nSize);
            m_hFile.seekg(0, std::ios::beg);
            m_hFile.read((char*)m_strData.c_str(), nSize);
        }
        if (bRead)
            m_hFile.close();
    }
}

FileHandle::~FileHandle()
{
    if (m_hFile.is_open())
    {
        m_hFile.flush();
        m_hFile.close();
    }
}

void FileHandle::AddFile(const FileInfo& infoFile)
{
    SCOPE_LOCK(m_mtLock);
    // save data into file
    m_hFile.write((char*)&infoFile.nType, sizeof(infoFile.nType));
    m_hFile.write((char*)&infoFile.nDataLen, sizeof(infoFile.nDataLen));
    m_hFile.write(infoFile.strData.c_str(), infoFile.strData.length());
    std::string strFill = std::string((infoFile.nSlice + 1) * kSlice - infoFile.strData.length(), '\0');
    m_hFile.write(strFill.c_str(), strFill.length());
    m_hFile.flush();
    // push data into cache
    m_mapFile.Insert(infoFile.nIndex, infoFile);
}

void FileHandle::ReadFile(FileInfo& infoFile)
{
    if (!m_mapFile.GetValue(infoFile.nIndex, infoFile))
    {
        int64_t nOffset = infoFile.nIndex * kSlice;
        if (!m_strData.empty() && m_strData.length() > nOffset)
        {
            SCOPE_LOCK(m_mtLock);
            int64_t nOffset = infoFile.nIndex * kSlice;
            memcpy(&infoFile.nType, m_strData.c_str() + nOffset, sizeof(infoFile.nType));
            nOffset += sizeof(infoFile.nType);
            memcpy(&infoFile.nDataLen, m_strData.c_str() + nOffset, sizeof(infoFile.nDataLen));
            nOffset += sizeof(infoFile.nDataLen);
            infoFile.strData.resize(infoFile.nDataLen);
            memcpy((void*)infoFile.strData.c_str(), m_strData.c_str() + nOffset, infoFile.nDataLen);
        }
    }
}
