#ifndef __MANAGER_H
#define __MANAGER_H
#include <vector>
#include <unordered_map>
#include "migrate.hpp"
#include "server.hpp"
#include "virtualMachine.hpp"
#include "operation.hpp"
#include "iostream"
#include "tools.hpp"
#include <string>
#include <stdio.h>
#include <stdlib.h>

#ifdef test
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif


#include "Integer_program.hpp"
#include "distribution.hpp"
class migrate;
struct migrate_operation;

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
    bool try_purchase_server(int id,int server_typeId,bool is_try = false);// 买一个id对应的服务器
    bool try_delet_server(int server_id);// 删除服务器，仅在尝试阶段使用
    // 往server_id 对应的服务器上部署vm_id对应的一个虚拟机
    // type选择 A B 或者 AB
    bool try_deploy_VM(int vm_id,int vm_type,
        int server_id,int type,bool is_log=true,
        bool is_try = false);
    // 注销掉虚拟机 
    bool try_de_deploy_VM(int vm_id,bool is_try = false);
    // 迁移虚拟机
    bool try_migrate_VM(int vm_id,int server_to,int type,bool is_try = false);
    float try_cal_cost(bool is_try = false);
    // 处理所有天的任务
    void processing();
    void try_distribution();
    void try_migrate();
    void assign_by_try();// 通过尝试的结果，按照实际的流程来赋值

    // 读取数据
#ifdef test
    void readTxt(const std::string &inputFile);// 测试读取txt文件使用
#endif

    void readTxtbyStream();
    void output();
    // cout 
    void finish_oneday();
    void re_begin();

    void result();

    // 没有考虑迁移的情况下 
    // 在当前状态下对下一天的任务进行尝试,返回尝试后的成本
    float try_oneday(std::vector<int> distribution,std::vector<int> node_type);
    float assign_oneday(int day,std::vector<int> distribution,std::vector<int> node_type);
    float assign_current_day(std::vector<int> distribution,std::vector<int> node_type){return assign_oneday(m_current_day,distribution,node_type);}
    // 初始时快速计算当前大概需要买多少服务器
    std::vector<int> coarse_init();
    //统计每天每个服务器每个节点CPU、RAM的占用率
    void statistic_busy_rate(int m_current_day);
    //将计算结果输出为txt文件
    void writetotxt();
private:
    // 当前的成本
    double m_purchase_cost;
    double m_power_cost = 0;
    double m_try_cost;// 尝试操作的成本

    int m_current_day = 0;
    int m_server_id = -1;// 服务器id，每一次购买时 ++ 
    // 分配算法类
    distribution *m_distribution;
    std::vector<distribution_operation> m_distribution_op;
    // 交换算法类
    migrate *m_migrate;
    std::vector<migrate_operation> m_migrate_op;
    //  初始服务器估计
    Integer_program * m_coarse_init;
    // 实际上购买的服务器
    std::unordered_map<int,server> m_purchase_servers;
    std::vector<int> m_serverss_ids;
    // 尝试购买的服务器 
    std::unordered_map<int,server> m_try_purchase_servers;
    // 尝试购买的服务器id
    std::vector<int> m_try_serverss_ids;
    // 实际上部署的虚拟机
    std::unordered_map<int,virtual_machine> m_deploy_VMs;
    // 实际上部署的虚拟机id
    std::vector<int> m_VMs_id;
    // 尝试部署的虚拟机 
    std::unordered_map<int,virtual_machine> m_try_deploy_VMs;
    // 所有天的操作数据 
    operations m_operators;
    // 保存所有虚拟机和服务器的实例
    std::vector<server_data> m_servers;
    std::vector<virtual_machine_data> m_VMs;
    std::vector<task> m_tasks;
    // 映射关系 
    std::unordered_map<std::string,int> m_server_map;
    std::unordered_map<std::string,int> m_VM_map;
public:
    //每天每台服务器的各个节点的CPU以及RAM的占用率
    std::vector<vector<vector<float>>>  used_rate;
    //昨天购买的服务器总量
    int lastdayCnt = 0;

    vector<float> sum_cost;
    vector<float> hard_cost;
    vector<float> ele_cost;
};

#endif // __MANAGER_H
