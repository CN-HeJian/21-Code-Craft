#ifndef __VIRTUAL_MACHINE_H
#define __VIRTUAL_MACHINE_H
#include <iostream>
#include "tools.hpp"
#include <vector>


class virtual_machine_data
{
public:
    virtual_machine_data(){}
    virtual_machine_data(int id,int cpu,int ram,int node):
    m_id(id),m_CPU_num(cpu),m_RAM(ram),m_is_double_node(node) {}
    // 复制构造函数
    virtual_machine_data(const virtual_machine_data & d){
        m_is_double_node = d.m_is_double_node;
        m_RAM = d.m_RAM;
        m_CPU_num = d.m_CPU_num;
        m_id = d.m_id;
        m_type = d.m_type;
        is_old = d.is_old;
    }
    virtual_machine_data &operator =(const virtual_machine_data& d) //重载=
    {
        m_is_double_node = d.m_is_double_node;
        m_RAM = d.m_RAM;
        m_CPU_num = d.m_CPU_num;
        m_id = d.m_id;
        m_type = d.m_type;
        is_old = d.is_old;
    }
    int m_id{};// 虚拟机的型号
    int m_CPU_num{};// CPU 总数
    int m_RAM{};// 内存大小
    bool m_is_double_node{};// 是否是双节点
    int m_type{};// 实际上的类型，虚拟机string映射后的id
    bool is_old = false; //第n天提供的服务器是否是新买的
    int node_type{};
};

// 虚拟机类 
class virtual_machine
{
public:
    // 创建一个新的虚拟机 
    virtual_machine(int id,virtual_machine_data data):
    m_index(id),m_data(data),m_server_id(-1){}
    virtual_machine(const virtual_machine & v)
    {
        m_index = v.m_index;
        m_data = v.m_data;
        m_server_id = v.m_server_id;
    }
    virtual_machine(){}
    // 数据接口
    int get_id(){return m_index;}
    int get_CPU(){return m_data.m_CPU_num;}
    int get_RAM(){return m_data.m_RAM;}
    int get_node_num(){return m_data.m_is_double_node;}
    int get_server_id(){return m_server_id;}
    int get_server_type(){return m_node_type;}
    int get_type(){return m_data.m_type;}
    int get_VM_id(){return m_data.m_id;}
    virtual_machine_data get_data(){return m_data;}
    // 提供一些基础操作
    void set_server_id(int server_id){m_server_id = server_id;}
    void set_server_type(int type){m_data.m_type = type;}
    void set_old(){m_data.is_old = true;}
    bool is_deploy(){return(m_server_id>0);}//是否实例化到服务器上
    // 将虚拟机添加到id服务器上
    bool deploy(int id,int node_type){
        if(m_server_id<0){
            m_server_id = id;
            m_node_type = node_type;
            return true;
        }else{
            std::cerr<<"you can deploy vm twice !!"<<std::endl;
            return false;
        }
    }
    // 将虚拟机从id服务器上删除
    bool de_deploy(){
        m_server_id = -1;
        m_node_type = -1;
        return true;
    }
private:

    int m_index;// 创建id m_index == m_data.m_type
    int m_node_type = -1;// 放在服务器节点上的方式
    virtual_machine_data m_data;
    int m_server_id;
};

#endif // __VIRTUAL_MACHINE_H
