#ifndef __MANAGER_H
#define __MANAGER_H
#include <vector>
#include <unordered_map>

#include "server.hpp"
#include "virtualMachine.hpp"
#include "operation.hpp"
#include "iostream"
#include "tools.hpp"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "Integer_program.hpp"


class manager
{
public:
    manager();
    // 获取数据函数
    // 获取任务相关信息
    task get_tasks(int day){return m_tasks.at(day);}
    int get_days(){return m_tasks.size();}

    int get_severNums(){return m_serverss_ids.size();}
    server get_server(int id){return m_purchase_servers[id];}

    // 一些操作
    bool purchase_server(int id,int server_typeId);// 买一个id对应的服务器
    // 往server_id 对应的服务器上部署vm_id对应的一个虚拟机
    // type选择 A B 或者 AB
    bool deploy_VM(int vm_id,int vm_type,int server_id,int type,bool is_log=true);
    // 注销掉虚拟机 
    bool de_deploy_VM(int vm_id);
    // 迁移虚拟机
    bool migrate_VM(int vm_id,int server_to,int type);
    float cal_cost();
    // 读取数据 
    void readTxt(const std::string &inputFile);// 测试读取txt文件使用
    void readTxtbyStream(const std::string &inputFile);
    void output();
    // cout 
    void finish_oneday();
    void re_begin();
    void cout_result();
    // 没有考虑迁移的情况下 
    // 在当前状态下对下一天的任务进行尝试,返回尝试后的成本
    float try_oneday(std::vector<int> distribution,std::vector<int> node_type);
    float assign_oneday(int day,std::vector<int> distribution,std::vector<int> node_type);
    float assign_current_day(std::vector<int> distribution,std::vector<int> node_type){return assign_oneday(m_current_day,distribution,node_type);}
    // 初始时快速计算当前大概需要买多少服务器
    std::vector<int> coarse_init();
private:
    // 当前的成本
    float m_cost;
    int m_current_day = 0;
    //  初始服务器估计
    Integer_program * m_coarse_init;
    // 实际上购买的服务器
    std::unordered_map<int,server> m_purchase_servers;
    std::vector<int> m_serverss_ids;
    // 实际上部署的虚拟机
    std::unordered_map<int,virtual_machine> m_deploy_VMs;
    // 所有天的操作数据 
    operations m_operators;
    // 保存所有虚拟机和服务器的实例
    std::vector<server_data> m_servers;
    std::vector<virtual_machine_data> m_VMs;
    std::vector<task> m_tasks;
    // 映射关系 
    std::unordered_map<std::string,int> m_server_map;
    std::unordered_map<std::string,int> m_VM_map;
};

#endif // __MANAGER_H
