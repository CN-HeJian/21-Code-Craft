#ifndef __DISTRIBUTION_H
#define __DISTRIBUTION_H 

#include <vector>
#include <unordered_map>
#include <algorithm>
#include "server.hpp"
#include <cmath>
const double eps = 1e-20;
enum{ mxn = 400, mxm = 200 };//mxn是未知数的个数（包括松弛变量）mxm 约束个数

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

#endif // __DISTRIBUTION_H
