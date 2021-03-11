#include "operation.hpp"

operations::operations(int day) : m_total_days(day), m_current_day(0)
{
    m_operations.resize(day);
}
void operations::set_days(int day)
{
    m_total_days = day;
    m_operations.resize(day);
}
// 购买type类型的服务器
void operations::purchase_server(std::string type)
{
    auto iter = m_operations.at(m_current_day).m_purchases.find(type);
    if (iter == m_operations.at(m_current_day).m_purchases.end())
    { // 没有信息
        m_operations.at(m_current_day).m_purchases.insert(std::make_pair(type, 1));
    }
    else
    {
        iter->second++;
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
    m_current_day++;
}

void operations::re_begin()
{
    m_current_day = 0;
    m_operations = std::vector<opetation>();
}