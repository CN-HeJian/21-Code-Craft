#ifndef __OPERATION_H
#define __OPERATION_H
#include <vector>

// 购买的服务器 
struct purchase_log
{
    int type;
};
// 部署虚拟机
struct deploy_log
{
    int id;
};
// 删除虚拟机
struct delete_log
{
    int id;
};
// 迁移虚拟机
struct migrate_log
{
    int id;
};
enum state{
    PURCHASE = 0,
    DEPLOY = 1,
    DELETE = 2,
    MIGRATE = 3
};
// 一天的所有操作 
struct opetation
{
    std::vector<purchase_log> m_purchases;
    std::vector<deploy_log> m_deploys;
    std::vector<delete_log> m_deletes;
    std::vector<migrate_log> m_migrates;
    // 记录当前一天的操作状态
    // 避免误操作，不按顺序来
    int state = PURCHASE;
};
// 记录每一天的操作 
class operations
{
public:
    operations(int day);
    // 购买type类型的服务器
    void purchase_server(int type);
    // 迁移虚拟机
    void migrate_VM();
    // 部署虚拟机
    void deploy_VM();
    // 删除虚拟机
    void delete_VM(int id); 
    void finish_oneday();
private:
    int m_total_days = -1;
    int m_current_day = -1;
    std::vector<opetation> m_operations;
};

#endif // __OPERATION_H
