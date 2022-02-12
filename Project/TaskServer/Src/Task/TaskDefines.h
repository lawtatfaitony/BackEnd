#pragma once
#include <string>
#include <vector>
#include <map>

struct TaskInfo
{
    std::string strIdetityId;  // 客户端连接id
    int nTaskId = 0;
    std::string strTaskName;
    std::vector<int> veclib;
    std::map<int, std::string> mapCamera1;
    std::map<int, std::string> mapCamera2;
    int nInterval = 3;  // s
    int nVideoId = 0;
    float nThreshold;
    int nTop = 1;
};