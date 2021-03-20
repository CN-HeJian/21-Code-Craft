#ifndef __OPERATION_H
#define __OPERATION_H
#include <vector>
#include <string>
#include <unordered_map>

// 部署虚拟机操作
struct deploy_log
{
    int server_id;
    bool is_double;
    std::string node;
};

// 迁移虚拟机
struct migrate_log
{
    int server_from_id;
    int server_to_id;
    bool is_double;
    std::string node;
};
enum state{
    PURCHASE = 0,
    DEPLOY = 1,
    DELETE = 2,
    MIGRATE = 3
};
struct purchase{
    int num;
   std::vector<int> server_id;
};
// 一天的所有操作 
struct opetation
{
    std::unordered_map<std::string,purchase> m_purchases;
    std::vector<deploy_log> m_deploys;// 包括删除和部署
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
    operations():m_current_day(0){}
    // 获取数据
    int days(){return m_total_days;}
    opetation get_operator(int day){return m_operations.at(day);}
    std::vector<std::string> get_servers_name(int day){return server_names.at(day);}
    void set_days(int day);
    
    // 购买type类型的服务器
    void purchase_server(std::string type,int server_id);
    // 迁移虚拟机
    void migrate_VM(int server_from_id,int server_to_id,bool is_double,std::string node);
    // 部署虚拟机
    void deploy_VM(int server_id,bool is_double,std::string node);
    void finish_oneday();
    void re_begin();
    std::unordered_map<int,int> m_server_id_map;
private:
    int m_total_days = -1;
    int m_current_day = -1;
    std::vector<opetation> m_operations;
    std::vector<std::vector<std::string>> server_names;
    int m_server_id = 0;
};

#endif // __OPERATION_H
