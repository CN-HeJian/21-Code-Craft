#ifndef __MIGRATE_A_H
#define __MIGRATE_A_H 
// 迁移类，实现迁移操作 
#include <vector>
#include "tools.hpp"
#include "server.hpp"
#include "manager.hpp"

// 结构体，保存一次迁移的结果
struct migrate_operation
{
    bool is_new;// 迁移的是否是新的节点
    int vm_id;// 需要迁移的虚拟机id
    int to_server_id;// 迁移到id对应的服务器
    // 见tools.hpp 中的枚举类型
    int node_type;//节点的类型，放置在A节点上还是B节点上，或者双节点AB
};

class migrate
{
public:
    migrate(/* args */);
    ~migrate();
    // 操作接口，尝试对虚拟机进行迁移 
    std::vector<migrate_operation> try_migrate(
        std::vector<std::pair<int,server_data>> servers,
        std::vector<std::vector<std::pair<int,virtual_machine_data>>> VMs);
private:
};





#endif //__MIGRATE_H
