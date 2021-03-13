#ifndef __MIGRATE_H
#define __MIGRATE_H 
// 迁移类，实现迁移操作 
#include <vector>
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

std::pair<int,server_data> server_data_;

class migrate
{
public:
    migrate(/* args */);
    ~migrate();
    // 操作接口，尝试对虚拟机进行迁移 
    std::vector<migrate_operation> try_migrate(
        std::vector<std::pair<int,server_data>> &servers,
        std::vector<std::vector<std::pair<int,virtual_machine_data>>> &VMs,
        vector<int> &remain_CPU_A,
        vector<int> &remain_RAM_A,
        vector<int> &remain_CPU_B,
        vector<int> &remain_RAM_B);
private:
    vector<bool> servers_isOld;
    vector<vector<bool>> vm_isOld;
    vector<bool> servers_isEmpty;
    int VmNums;
    int max_migrateCnt;
    constexpr const static  float Defined_highUsed  = 0.75;
    vector<float> A_CPU_used_rate;
    vector<float> A_RAM_used_rate;
    vector<float> B_CPU_used_rate;
    vector<float> B_RAM_used_rate;
    vector<bool> is_highUsed;

};





#endif //__MIGRATE_H
