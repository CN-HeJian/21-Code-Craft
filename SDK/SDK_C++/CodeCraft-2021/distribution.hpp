#ifndef __DISTRIBUTION_H
#define __DISTRIBUTION_H 

#include <vector>
#include "server.hpp"
#include "virtualMachine.hpp"
#include "tools.hpp"

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
};


class distribution
{
public:
    distribution(std::vector<server_data> servers,std::vector<virtual_machine_data> VMs);
    ~distribution();
    // 数据接口，尝试根据task的任务将虚拟机往服务器上分配
    std::vector<distribution_operation> try_distribution(
        std::vector<int> servers_type_id,
        std::vector<std::vector<int>> VMs_type_id,
        std::vector<task> tasks
    );
private:
    /* data */    
    // 保存所有虚拟机和服务器的实例
    std::vector<server_data> m_servers;
    std::vector<virtual_machine_data> m_VMs;
};

#endif // __DISTRIBUTION_H
