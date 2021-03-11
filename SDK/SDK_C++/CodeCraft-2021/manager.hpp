#ifndef __MANAGER_H
#define __MANAGER_H
#include <vector>
#include <unordered_map>

#include "server.hpp"
#include "virtualMachine.hpp"
#include "operation.hpp"
#include "iostream"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

struct task
{
    // ("add/del" ,(vm_id,vm_type))
    std::vector<std::pair<std::string,std::pair<int,int>>> cmd;
};

class manager
{
public:
    manager();
    // 获取数据函数
    task get_tasks(int day){return m_tasks.at(day);}
    int get_days(){return m_tasks.size();}
    server get_server(int id){return m_purchase_servers[id];}
    // 一些操作
    bool purchase_server(int id,int type);// 买一个id对应的服务器
    // 往servver_id 对应的服务器上部署vm_id对应的一个虚拟机
    // type选择 A B 或者 AB
    bool deploy_VM(int vm_id,int vm_type,int server_id,int type,bool is_log=true);
    // 注销掉虚拟机 
    bool de_deploy_VM(int id);
    // 迁移虚拟机
    bool migrate_VM(int vm_id,int server_to,int type);
    float cal_cost();
    // 读取数据 
    void readTxt(const std::string &inputFile);
    void output();
    // cout 
    void finish_oneday();
    void re_begin();
    void cout_result();
private:
    // 当前的成本
    float m_cost;
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
