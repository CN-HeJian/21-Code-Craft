#include "operation.hpp"

operations::operations(int day) : m_total_days(day), m_current_day(0)
{
    m_operations.resize(day);
    server_names.resize(day);
}
void operations::set_days(int day)
{
    m_total_days = day;
    m_operations.resize(day);
    server_names.resize(day);
}
// 购买type类型的服务器
void operations::purchase_server(std::string type,int server_id)
{
    purchase p;
    auto iter = m_operations.at(m_current_day).m_purchases.find(type);
    if (iter == m_operations.at(m_current_day).m_purchases.end())
    { // 没有信息
        p.num = 1;
        p.server_id.emplace_back(server_id);
        m_operations.at(m_current_day).m_purchases.insert(std::make_pair(type, p));
        server_names.at(m_current_day).emplace_back(type);
    }
    else
    {
        iter->second.num ++;
        iter->second.server_id.emplace_back(server_id);
    }
}
// 迁移虚拟机
void operations::migrate_VM(int server_from_id, int server_to_id, bool is_double, std::string node)
{
    migrate_log log;
    log.server_from_id = server_from_id;
    log.server_to_id = server_to_id;
    log.is_double = is_double;
    log.node = node;
    m_operations.at(m_current_day).m_migrates.emplace_back(log);
}
// 部署虚拟机
void operations::deploy_VM(int server_id, bool is_double, std::string node)
{
    deploy_log log;
    log.server_id = server_id;
    log.is_double = is_double;
    log.node = node;
    m_operations.at(m_current_day).m_deploys.emplace_back(log);
}

void operations::finish_oneday()
{
    auto p = m_operations.at(m_current_day);
    // remap current day servers id
    for(const auto& name:server_names.at(m_current_day))
    {
        int num = p.m_purchases[name].num;
        for(int i = 0;i < num;i ++)
        {
            m_server_id_map.insert(std::make_pair(p.m_purchases[name].server_id.at(i),m_server_id));
            m_server_id ++;
        }
    }
    m_current_day++;
}

void operations::re_begin()
{
    m_current_day = 0;
    m_operations = std::vector<opetation>();
}