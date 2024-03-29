#ifndef __SERVER_H
#define __SERVER_H
#include <map>  
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

//#define  test

class virtual_machine_data;
class virtual_machine;
/////////////////////////////////////////////////////////////////////////
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
        return *this;
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
    int get_server_type(){return m_data.m_type;}
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
            //std::cerr<<"you can deploy vm twice !!"<<std::endl;
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
    int m_server_id = -1;
};
// 服务器数据结构体
class server_data
{
public:    
    int m_type;// 服务器的型号
    int m_CPU_num;// CPU 总数
    int m_RAM;// 内存大小 
    int m_price;// 购买的价格
    int m_daily_cost;// 每天的成本价格
    std::string m_name;
    bool is_old = false;
        // 占用率 A 
    float occupancy_factor_A = 0.f;// A节点占用率
    // 占用率 B
    float occupancy_factor_B = 0.f;// B节点占用率
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
        occupancy_factor_A = d.occupancy_factor_A;// A节点占用率
        occupancy_factor_B = d.occupancy_factor_B;// B节点占用率
        is_old = d.is_old;
    }
    server_data &operator =(const server_data& d) //重载=
    {
        m_CPU_num = d.m_CPU_num;
        m_daily_cost = d.m_daily_cost;
        m_type = d.m_type;
        m_price = d.m_price;
        m_RAM = d.m_RAM;
        m_name = d.m_name;
        occupancy_factor_A = d.occupancy_factor_A;// A节点占用率
        occupancy_factor_B = d.occupancy_factor_B;// B节点占用率
        is_old = d.is_old;
        return *this;
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
        m_RAM_left_A = s.m_RAM_left_A;
        m_RAM_left_B = s.m_RAM_left_B;
        m_CPU_left_A = s.m_CPU_left_A;
        m_CPU_left_B = s.m_CPU_left_B;
        m_VM = s.m_VM;
        m_VM_ids = s.m_VM_ids;
    }
    void operator =(const server& s) //重载=
    {
        m_data = s.m_data;
        m_index = s.m_index;
        m_RAM_left_A = s.m_RAM_left_A;
        m_RAM_left_B = s.m_RAM_left_B;
        m_CPU_left_A = s.m_CPU_left_A;
        m_CPU_left_B = s.m_CPU_left_B;

        m_VM = s.m_VM;
        m_VM_ids = s.m_VM_ids;
    }
    // 数据接口
    server_data get_data(){return m_data;}
    int get_id(){return m_index;}
    int get_CPU(){return m_data.m_CPU_num;}
    int get_RAM(){return m_data.m_RAM;}
    int get_left(){return m_CPU_left_A + m_RAM_left_A + m_CPU_left_B + m_RAM_left_B;}
    int get_daily_cost(){return m_data.m_daily_cost;}
    int get_price(){return m_data.m_price;}
    int get_type(){return m_data.m_type;}
    int get_CPU_left_A(){return m_CPU_left_A;}
    int get_RAM_left_A(){return m_RAM_left_A;}
    int get_CPU_left_B(){return m_CPU_left_B;}
    int get_RAM_left_B(){return m_RAM_left_B;}
    int get_cost(){return m_data.m_price + m_power_on_day * m_data.m_daily_cost;}
    int get_power_on_day(){return m_power_on_day;}
    int get_VM_num(){return m_VM.size();}
    float get_occupancy_factor_A(){return m_data.occupancy_factor_A;}
    float get_occupancy_factor_B(){return m_data.occupancy_factor_B;}
    std::vector<int> get_VM_ids(){return m_VM_ids;}
    std::unordered_map<int,virtual_machine_data> get_VM(){return m_VM;}

    // 提供一些操作
    void set_old();
    bool is_power_on();// 判断是否当前开机
    bool add_virtual_machine(int id,virtual_machine_data VM,int type);// 添加一个虚拟机 
    bool remove_virtual_machine(int id);// 删除一个虚拟机
    void reset_type(server_data temp);//换掉服务器需要用到
    void update_one_day(){m_power_on_day ++;}
private:
    /* data */
    int m_power_on_day = 0;
    int m_index;// 购买id

    server_data m_data;// 服务器的型号数据
    int m_CPU_left_A;// A节点当前服务器剩余的CPU数目
    int m_RAM_left_A;// A节点当前服务器剩余的RAM数目
    int m_CPU_left_B;// B节点当前服务器剩余的CPU数目
    int m_RAM_left_B;// B节点当前服务器剩余的RAM数目

    // 含有的虚拟机 
    std::vector<int> m_VM_ids;//依次记录配置的每一个虚拟机的id，m_VM_ids_size即这个服务器上包含的虚拟机的总数
    std::unordered_map<int,virtual_machine_data> m_VM;// 包含的虚拟机数据，通过id索引
};




#endif //__SERVER_H
