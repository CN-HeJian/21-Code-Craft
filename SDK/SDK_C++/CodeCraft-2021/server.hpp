#ifndef __SERVER_H
#define __SERVER_H
#include <map>  
#include <unordered_map>
#include "virtualMachine.hpp"
#include "tools.hpp"
#include <vector>
#include <string>


//test

// 服务器数据结构体
class server_data
{
public:    
    int m_type;// 服务器的型号
    int m_CPU_num;// CPU 总数
    int m_RAM;// 内存大小 
    int m_price;// 购买的价格
    int m_daily_cost;// 每天的成本价格
    bool is_old;
    std::string m_name;
    // 构造函数 
    server_data(){}
    server_data(int id,int cpu,int ram,int price,int daily_cost,std::string name):
    m_type(id),m_CPU_num(cpu),
    m_RAM(ram),m_price(price),
    m_daily_cost(daily_cost),m_name(name){}
    // 复制构造函数
    server_data(const server_data & d){
        m_CPU_num = d.m_CPU_num;
        m_daily_cost = d.m_daily_cost;
        m_type = d.m_type;
        m_price = d.m_price;
        m_RAM = d.m_RAM;
        m_name = d.m_name;
    }
};


// 服务器类,购买的时候新建 
class server
{
public:
    // 新购买一个服务器
    server(){}
    server(int id,server_data data):m_index(id),m_data(data)
    {
        m_RAM_left_A = m_RAM_left_B = data.m_RAM/2;
        m_CPU_left_A = m_CPU_left_B = data.m_CPU_num/2;
    }
    server(const server & s){
        m_data = s.m_data;
        m_index = s.m_index;
        m_RAM_left_A = m_RAM_left_B = s.m_RAM_left_A;
        m_CPU_left_A = m_CPU_left_B = s.m_CPU_left_A;
        m_VM = s.m_VM;
        m_VM_ids = s.m_VM_ids;
    }
    // 数据接口
    int get_id(){return m_index;}
    int get_CPU(){return m_data.m_CPU_num;}
    int get_RAM(){return m_data.m_RAM;}
    int get_daily_cost(){return m_data.m_daily_cost;}
    int get_price(){return m_data.m_price;}
    int get_type(){return m_data.m_type;}
    int get_CPU_left_A(){return m_CPU_left_A;}
    int get_RAM_left_A(){return m_RAM_left_A;}
    int get_CPU_left_B(){return m_CPU_left_B;}
    int get_RAM_left_B(){return m_RAM_left_B;}
    int get_VM_num(){return m_VM.size();}
    std::vector<int> get_VM_ids(){return m_VM_ids;}
    std::unordered_map<int,virtual_machine_data> get_VM(){return m_VM;}
    // 提供一些操作
    bool is_power_on();// 判断是否当前开机
    bool add_virtual_machine(int id,virtual_machine_data VM,int type);// 添加一个虚拟机 
    bool remove_virtual_machine(int id);// 删除一个虚拟机 
private:
    /* data */   
    int m_index;// 购买id
    server_data m_data;// 服务器的型号数据
    int m_CPU_left_A;// A节点当前服务器剩余的CPU数目
    int m_RAM_left_A;// A节点当前服务器剩余的RAM数目
    int m_CPU_left_B;// B节点当前服务器剩余的CPU数目
    int m_RAM_left_B;// B节点当前服务器剩余的RAM数目
    // 含有的虚拟机 
    std::vector<int> m_VM_ids;//依次记录配置的每一个虚拟机的id，m_VM_ids_size即这个服务器上包含的虚拟机的总数
    std::unordered_map<int,virtual_machine_data> m_VM;// 包含的虚拟机数据，通过id范围

};

#endif //__SERVER_H
