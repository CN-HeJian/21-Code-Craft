#ifndef __TOOLS_H
#define __TOOLS_H
#include <chrono>
#include <string>
#include <vector>

#define test
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

void clock_start();
float clock_end();

#endif //__TOOLS_H