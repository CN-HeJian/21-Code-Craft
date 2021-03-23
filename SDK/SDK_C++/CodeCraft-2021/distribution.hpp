#ifndef __DISTRIBUTION_H
#define __DISTRIBUTION_H 

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
const double eps = 1e-20;
enum{ mxn = 400, mxm = 200 };//mxn是未知数的个数（包括松弛变量）mxm 约束个数


//#define  test

class virtual_machine_data;
class virtual_machine;
class server;
class server_data;
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

// 单纯性法求解
class Simplex
{
public:
    int n, m, t;
    double c[mxn] ;
    double a[mxm][mxn] ;
    int idx[mxn] , idy[mxn] ;
    int st[mxn] , top = 0;

    Simplex(int _m, int _n);
    void set_objective(double ci[mxn]);
    void set_co_matrix(double co_matrix[mxm][mxn]);
    void set_bi_matrix(double b_matrix[mxm]);
    int init_simplex();
    void Pivot(int x, int y);
    int run();
    std::pair<std::vector<double>, double> getans();
};

// 分支定界法求解整数规划问题
class Integer_program
{
public:
    Integer_program(int server_num);
    ~Integer_program();
    void set_all_servers(std::vector<server_data> server_data,int min_cpu,int min_ram);
    void get_min_CPU(int cpu);
    void get_min_RAM(int ram);
    std::vector<int> solve(bool fast);
private:
    Simplex *m_solver;// 求解器
    std::vector<server_data> m_server_data;// 服务器数据
    int m_server_num;
    // 单纯性法求解约束
    double b_matrix[mxm] = { 0 };
    double co_matrix[mxm][mxn] = { {0} };
    double c_matrix[mxn] = { 0 };
};


/////////////////////////////////////////////////////////////////////////////////////////////
using namespace std;


// 返回不同的操作
enum distribution_type{
    norm = 0,// 正常分配操作
    erase = 1,// 删除服务器操作
    add = 2// 添加服务器操作 
};

// 正常分配的信息
struct  distribution_operation
{
    int server_id;// 分配到id对应的服务器上
    int server_type;//只有当额外添加服务器时才用
    int node_type;//节点的类型，放置在A节点上还是B节点上，或者双节点AB
    // norm 正常操作 
    // erase 将 server_id 对应的服务器删除 
    // add 则是再添加一个server_type类型的服务器，其id为 server_id 
    int distribution_type;
    distribution_operation()
    {
    }
    distribution_operation(const distribution_operation & op)
    {
        server_id = op.server_id;
        server_type = op.server_type;
        node_type = op.node_type;
        distribution_type = op.distribution_type;
    }
};


class distribution
{
public:
    distribution(std::vector<server_data>& servers, std::vector<virtual_machine_data>& VMs);
    ~distribution();
    // 数据接口，尝试根据task的任务将虚拟机往服务器上分配
    std::vector<distribution_operation> try_distribution(
        std::vector<int>& servers_type_id,
        const task& task_today,
        std::vector<int>& remain_CPU_A,
        std::vector<int>& remain_RAM_A,
        std::vector<int>& remain_CPU_B,
        std::vector<int>& remain_RAM_B,
        std::vector<std::pair<int, int>>& delete_server_id,
        std::unordered_map<int, int>& vmId_2_vmTypeId
    );
    std::vector<distribution_operation> try_distribution2(
            std::vector<int>& servers_type_id,
            const task& task_today,
            std::vector<int>& remain_CPU_A,
            std::vector<int>& remain_RAM_A,
            std::vector<int>& remain_CPU_B,
            std::vector<int>& remain_RAM_B,
            std::vector<std::pair<int, int>>& delete_server_id,
            std::unordered_map<int, int>& vmId_2_vmTypeId
    );
std::vector<distribution_operation> try_violence_distribution(
        std::vector<int> servers_type_id,
        std::vector<std::vector<int> > VMs_type_id,
        task tasks,
        std::vector<int> left_CPU_A,
        std::vector<int> left_RAM_A,
        std::vector<int> left_CPU_B,
        std::vector<int> left_RAM_B);
private:    
    /************ used for violent distribution ***************/
    void sort_Server_Index();// 对服务器表进行排序
    /********************************************************/
private:
    /* data */    
    // 保存所有已有的虚拟机和服务器的实例表：相当于产品表
    std::vector<server_data> m_servers;// 目前是按照读入顺序
    std::vector<virtual_machine_data> m_VMs;// 目前按照读入顺序排序
    std::vector<distribution_operation> distribution_result_queue;//当天操作完成后返回的操作数据队列
    std::vector<distribution_operation> server_buy_queue;//当天操作完成后返回的操作数据队列

    std::vector<int> sorted_server_table;// 按照约定排序方式排序好的服务器表，存的元素是对应型号的索引
    std::vector<int> sorted_vm_table;// 按照约定排序方式排序好的虚拟机表，存的元素是对应型号的索引

    std::vector<int> split_pos;//存放del命令在今日命令中的索引
    const int num_of_server_type = 105;// 服务器的最大种类数

    float weight_cpu = 0.5;
    float weight_ram = 0.5;
};


/////////////////////////////////////////////////////////////////////////


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
    float lower_factor_A = 0.f;
    float lower_factor_B = 0.f;
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
        lower_factor_A = d.occupancy_factor_A;
        lower_factor_B = d.occupancy_factor_B;
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
        lower_factor_A = d.lower_factor_A;
        lower_factor_B = d.lower_factor_B;
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

#endif // __DISTRIBUTION_H
