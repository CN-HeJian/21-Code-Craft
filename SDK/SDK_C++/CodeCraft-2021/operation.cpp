#include "operation.hpp"

operations::operations(int day):m_total_days(day),m_current_day(1)
{
    m_operations.reserve(day);
}
// 购买type类型的服务器
void operations::purchase_server(int type)
{

}
// 迁移虚拟机
void operations::migrate_VM()
{

}
// 部署虚拟机
void operations::deploy_VM()
{

}
// 删除虚拟机
void operations::delete_VM(int id)
{

}
void operations::finish_oneday()
{

}
