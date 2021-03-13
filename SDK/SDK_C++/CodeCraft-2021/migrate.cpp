#include "migrate.hpp"
using namespace std;

migrate::migrate(/* args */)
{
}

migrate::~migrate()
{
}

/**
 * @brief 尝试对给定服务器进行迁移
 * @param servers 当前所有的服务器 
 *                    std::vector<std::pair<int,server_data>> 数据结构
 *                    其中 第一个int代表当前服务器的id，server_data 中包含了该服务器的所有数据
 * @param VMs 每一个服务器对应的所有虚拟机 
 *                     std::vector<std::vector<std::pair<int,virtual_machine_data>>>
 *                     其中，std::vector<std::pair<int,virtual_machine_data>> 包含了一个服务器内的所有虚拟机，和上面是对应的
 *                     这其中的 int 代表当前虚拟机的id，virtual_machine_data 中包含了该虚拟机的所有数据 
 * @return 迁移结果 
 * */
vector<migrate_operation> migrate::try_migrate(
    vector<std::pair<int,server_data>> &servers,
    vector<vector<pair<int,virtual_machine_data>>> &VMs) {

    /*扫描*/


    //servers.at(0).second.is_old;
    //VMs.at(0).at(0).second.is_old; //VMs存了每一个服务器中的存的所有虚拟机

}





