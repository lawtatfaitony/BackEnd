#pragma once
#include <Task.h>


class SaveResultBase
{
public:
    SaveResultBase();
    ~SaveResultBase();
    void Save(Task::TaskResult& taskResul);
};