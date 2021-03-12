#include "distribution.hpp"

distribution::distribution(std::vector<server_data> servers, std::vector<virtual_machine_data> VMs) :
 m_VMs(VMs), m_servers(servers)
{
}

distribution::~distribution()
{
}

/**
 * @brief  数据接口，尝试根据task的任务将虚拟机往服务器上分配 
 * @param servers_type_id 当前所有服务器的类型id
 *                    例如，servers_type_id.at(10) 代表第11台服务器的类型，然后查表就知道对应的数据了
 * @param VMs_type_id 目前已经分配好的各服务器对应的虚拟机类型
 *                    例如，VMs_type_id.at(3).at(1) 代表第4台服务器内，第2台虚拟机的类型，然后查表得到具体的数据
 * @param tasks 当前天具体的任务,在tools中有定义
 * @return 需要进行的操作
 *                    填充数据  std::vector<distribution_operation>  
 *                    按照如下的顺序进行
 *                    1、添加服务器命令
 *                    2、正常任务，一定要按照task的顺序来，不然就错了
 *                    3、删除服务器命令
 *          
 * */
std::vector<distribution_operation> distribution::try_distribution(
        std::vector<int> servers_type_id,
        std::vector<std::vector<int>> VMs_type_id,
        std::vector<task> tasks)
{

}