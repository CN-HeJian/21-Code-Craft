#ifndef __MIGRATE_A_H
#define __MIGRATE_A_H 
// 迁移类，实现迁移操作 
#include <vector>
#include <unordered_map>
#include "tools.hpp"
#include "server.hpp"
#include "manager.hpp"

using namespace std;



// 结构体，保存一次迁移的结果
struct migrate_operation
{
    bool is_new;// 迁移的是否是新的节点
    int vm_id;// 需要迁移的虚拟机id,用户自定义的id
    int to_server_id;// 迁移到id对应的服务器
    // 见tools.hpp 中的枚举类型
    int node_type;//节点的类型，放置在A节点上还是B节点上，或者双节点AB，node_type = TYPE.A
};

struct node{
    bool A;
    bool B;
};

struct change_twoVm{
    float jude_point;  //评价指标
    //一个在原来中的位置
    int one_sever_id;
    int one_vm_id;
    //另一个在原来中的位置
    int another_server_id;
    int another_vm_id;
};

class migrate
{
public:
    migrate(std::vector<server_data>& servers, std::vector<virtual_machine_data>& VMs);
    //migrate(/* args */);
    ~migrate();
    // 操作接口，尝试对虚拟机进行迁移 
    std::vector<migrate_operation> try_migrate(
            std::unordered_map<int,virtual_machine> &m_deployed_vm,
            std::unordered_map<int,int> &vmId_2_id,
            std::vector<int>& servers_type_id,
            std::vector<std::vector<int>>& VMs_type_id,
            const vector<bool>& service_is_new,
            vector<vector<bool>> vm_is_new,
            std::vector<int>& remain_CPU_A,
            std::vector<int>& remain_RAM_A,
            std::vector<int>& remain_CPU_B,
            std::vector<int>& remain_RAM_B);
private:
    vector<bool> servers_isOld;//判断第i台服务器是否为旧的服务器
    vector<vector<bool>> vm_isOld;//二维数组判断第i台服务器上的第j台虚拟机是否为旧的
    vector<bool> servers_isEmpty;//判断某台服务器是否为空
    int VmNums{};//虚拟机总数
    int max_migrateCnt{};//最大迁移次数
    constexpr const static  float Defined_highUsed  = 0.75;//高占用
    vector<float> A_CPU_used_rate;//资源占用率
    vector<float> A_RAM_used_rate;
    vector<float> B_CPU_used_rate;
    vector<float> B_RAM_used_rate;

    vector<bool> is_highUsed;//每台服务器是否被高占用
    vector<node> cpu_busy; //单独判断每台服务器上单个节点CPU是否繁忙
    vector<node> ram_busy; //单独判断每台服务器上单个节点RAM是否繁忙
    //节点
    vector<bool> is_VMs_double; //判断虚拟机为单节点还是双节点
    vector<bool> is_VMs_newAndDouble; //判断每一台虚拟机是否为新加的双节点虚拟机
    vector<char> VMs__serverNode; //每一台虚拟机挂在在哪个服务器上的哪个节点上
    //用来保存移动的操作
    vector<int> server_id;
    vector<int> vM_id;
    //随机生成数
    int rand_one(int min, int max);
    //评判交换策略
    float judge_operate(std::unordered_map<int,virtual_machine> &m_deplyed_vm,
                        std::unordered_map<int,int> &vmId_2_id,
                        std::vector<int>& servers_type_id,
                        std::vector<std::vector<int>>& VMs_type_id,
                        std::vector<int>& remain_CPU_A,
                        std::vector<int>& remain_RAM_A,
                        std::vector<int>& remain_CPU_B,
                        std::vector<int>& remain_RAM_B,
                        vector<pair<int,int>> &recordSelectedPos,
                        int &oneVM,
                        int &anotherVM);
    //判断能否进行交换
    char migrate::canChange(
                    std::unordered_map<int,virtual_machine> &m_deplyed_vm,
                    std::unordered_map<int,int> &vmId_2_id,
                    std::vector<int>& servers_type_id,
                    std::vector<std::vector<int>>& VMs_type_id,
                    std::vector<int>& remain_CPU_A,
                    std::vector<int>& remain_RAM_A,
                    std::vector<int>& remain_CPU_B,
                    std::vector<int>& remain_RAM_B,
                    int vm_server_id,
                    int vm_vms_id,
                    int togo_serverid);
    std::vector<server_data> m_servers;// 目前是按照读入顺序
    std::vector<virtual_machine_data> m_VMs;// 目前按照读入顺序排序
};

#endif //__MIGRATE_H
