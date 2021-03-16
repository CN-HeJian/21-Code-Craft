#ifndef __TOOLS_H
#define __TOOLS_H
#include <chrono>
#include <string>
#include <vector>

//#define test
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

#endif //__TOOLS_H