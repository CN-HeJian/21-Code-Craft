#ifndef __TOOLS_H
#define __TOOLS_H
#include <chrono>

enum TYPE{
    A = 0,
    B = 1,
    AB = 2
};

struct task
{
    // ("add/del" ,(vm_id,vm_type))
    std::vector<std::pair<std::string,std::pair<int,int>>> cmd;
};

bool clock_start();
float clock_end();

#endif //__TOOLS_H