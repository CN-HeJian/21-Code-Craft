#include "manager.hpp"
#include <iostream>
using namespace std;

manager::manager() : m_purchase_cost(0)
{
}
// 买一个id对应的服务器
// 设置服务器 id
// 服务器类型
bool manager::try_purchase_server(int id, int server_typeId, bool is_try)
{
    if ((size_t)server_typeId > m_servers.size() - 1)
    {
        std::cerr << "can not find the server!!!" << std::endl;
        return false;
    }
    if (is_try)
    { // 只是在当前天的条件下进行的尝试
        m_try_purchase_servers.insert(std::pair<int, server>(id, server(id, m_servers.at(server_typeId))));
        m_try_serverss_ids.emplace_back(id);
        m_try_cost += m_servers.at(server_typeId).m_price;
    }
    else
    { // 实际上确定要执行的操作
        auto data = m_servers.at(server_typeId);
        data.is_old = true;
        m_purchase_servers.insert(std::pair<int, server>(id, server(id, data)));
        m_serverss_ids.emplace_back(id);
        m_purchase_cost += data.m_price;
        // 记录操作
        m_operators.purchase_server(data.m_name,id);
    }
    return true;
}
// 往servver_id 对应的服务器上部署vm_id对应的一个虚拟机
// type选择 A B 或者 AB
bool manager::try_deploy_VM(int vm_id, int vm_typeId, int server_id, int type, bool is_log, bool is_try)
{
    // 构造一个虚拟机
    if ((size_t)vm_typeId > m_VMs.size() - 1)
    {
        std::cerr << "can not find the virtual machine !!!" << std::endl;
        return false;
    }
    virtual_machine VM(vm_id, m_VMs.at(vm_typeId));
    VM.set_server_type(type);
    if (is_try)
    { // 尝试部署虚拟机
        auto iter = m_try_purchase_servers.find(server_id);
        if (iter == m_try_purchase_servers.end())
        {
            std::cerr << "can not find the server !!!" << std::endl;
            return false;
        }
        //
        if (!VM.deploy(server_id,type))
        {
            return false;
        }
        if (!iter->second.add_virtual_machine(vm_id, VM.get_data(), type))
        {
            return false;
        }
        m_try_deploy_VMs.insert(std::pair<int, virtual_machine>(vm_id, VM));
    }
    else
    {
        // 插入到服务器
        // 查找指定的服务器
        VM.set_old();
        auto iter = m_purchase_servers.find(server_id);
        if (iter == m_purchase_servers.end())
        {
            std::cerr << "can not find the server !!!" << std::endl;
            std::vector<int> i;
            i.at(2) = 0;
            return false;
        }
        if (!VM.deploy(server_id,type))
        {
            return false;
        }
        if (!iter->second.add_virtual_machine(vm_id, VM.get_data(), type))
        {
            return false;
        }
        m_purchase_servers[server_id].set_old();

        m_deploy_VMs.insert(std::pair<int, virtual_machine>(vm_id, VM));
        m_VMs_id.emplace_back(vm_id);
        // 记录操作
        if (is_log)
            m_operators.deploy_VM(server_id, (type == AB), type == A ? "A" : "B");
    }
    return true;
}
// 注销掉虚拟机
bool manager::try_de_deploy_VM(int vm_id, bool is_try)
{
    if (is_try)
    {
        auto iter_vm = m_try_deploy_VMs.find(vm_id);
        if (iter_vm == m_try_deploy_VMs.end())
        { // 没有找到
            std::cerr << "can not delet unexit VM" << std::endl;
            return false;
        }
        auto server_id = iter_vm->second.get_server_id();
        iter_vm->second.de_deploy();
        auto iter_server = m_try_purchase_servers.find(server_id);
        if (iter_server == m_purchase_servers.end())
        {
            std::cerr << "can not find corresponde server " << std::endl;
            return false;
        }
        if (!iter_server->second.remove_virtual_machine(vm_id))
        {
            return false;
        }
        m_try_deploy_VMs.erase(iter_vm);
    }
    else
    {
        auto iter_vm = m_deploy_VMs.find(vm_id);
        if (iter_vm == m_deploy_VMs.end())
        {
            std::cerr << "can not delet unexit VM" << std::endl;
            return false;
        }
        auto server_id = iter_vm->second.get_server_id();
        iter_vm->second.de_deploy();
        auto iter_server = m_purchase_servers.find(server_id);
        if (iter_server == m_purchase_servers.end())
        {
            std::cerr << "can not find corresponde server " << std::endl;
            return false;
        }
        if (!iter_server->second.remove_virtual_machine(vm_id))
        {
            return false;
        }
        m_deploy_VMs.erase(iter_vm);
        auto it = find(m_VMs_id.begin(), m_VMs_id.end(), vm_id);
        m_VMs_id.erase(it);
    }
    return true;
}
/*功能：迁移虚拟机，先删除然后再添加
* 输入参数：
* vm_id : 迁移虚拟机id
* server_to : 迁移到这个id的服务器上
* type : 在这个服务器上部署节点的类型
*/
bool manager::try_migrate_VM(int vm_id, int server_to, int type, bool is_try)
{
    //
    if (is_try)
    {
        auto iter_vm = m_try_deploy_VMs.find(vm_id);
        if(iter_vm == m_try_deploy_VMs.end())
        {
            std::cerr << "can not migrate not exit VM!!!" << std::endl;
            return false;
        }
        int vm_type = iter_vm->second.get_VM_id();
        if (!try_de_deploy_VM(vm_id,true))
        {
            return false;
        }
        if (!try_deploy_VM(vm_id, vm_type, server_to, type, false,true))
        {
            return false;
        }
        // record
        m_mig_ops.emplace_back(mig_op(vm_id, vm_type, server_to, type));
    }
    else
    {
        auto iter_vm = m_deploy_VMs.find(vm_id);
        if (iter_vm == m_deploy_VMs.end())
        {
            std::cerr << "can not migrate not exit VM!!!" << std::endl;
            return false;
        }
        int vm_type = iter_vm->second.get_VM_id();
        if (!try_de_deploy_VM(vm_id))
        {
            return false;
        }
        if (!try_deploy_VM(vm_id, vm_type, server_to, type, false))
        {
            return false;
        }
        // 记录操作
        m_operators.migrate_VM(vm_id, server_to, (type == AB), type == A ? "A" : "B");
    }
    return true;
}

float manager::try_cal_cost(bool is_try)
{
    if (is_try)
    {
        for (auto id : m_try_serverss_ids)
        {
            m_try_cost += m_try_purchase_servers[id].get_daily_cost();
        }
    }
    else
    {
        for (auto id : m_serverss_ids)
        {
            if (m_purchase_servers[id].is_power_on())
            {
                m_power_cost += m_purchase_servers[id].get_daily_cost();
            }
        }
    }
    return m_power_cost;
}

void manager::finish_oneday()
{
    m_operators.finish_oneday();
    m_mig_ops.clear();
    m_current_day++;
}
void manager::re_begin()
{
    m_operators.re_begin();
    m_current_day = 0;
}

std::vector<int> manager::fine_init(std::queue<int> task_ids)
{
    auto task = m_tasks.at(m_current_day).cmd;
    float sum_cpu = 0, sum_ram = 0;
    for (int i = 0;i < task_ids.size();i ++)
    { // 遍历当前所有任务
        int id = task_ids.front();
        task_ids.pop();
        int sign = task.at(id).first == "add"?1:-1;
        {
            int vm_type_id = task.at(id).second.second;
            sum_cpu += (float)(sign * m_VMs.at(vm_type_id).m_CPU_num);
            sum_ram += (float)(sign * m_VMs.at(vm_type_id).m_RAM);
        }
    }
    for(auto s:m_try_serverss_ids)
    {
        float rate_A =  m_try_purchase_servers[s].get_occupancy_factor_A()<0.5?1:0;
        float rate_B =  m_try_purchase_servers[s].get_occupancy_factor_B()<0.5?1:0;

        sum_cpu -= (rate_A * (float)m_try_purchase_servers[s].get_CPU_left_A() + rate_B * (float)m_purchase_servers[s].get_CPU_left_B());
        sum_ram -= (rate_A * (float)m_try_purchase_servers[s].get_RAM_left_A() + rate_B * (float)m_purchase_servers[s].get_RAM_left_B());
    }
    sum_cpu = sum_cpu < 0 ? 0 : sum_cpu;
    sum_ram = sum_ram < 0 ? 0 : sum_ram;
    m_coarse_init->set_all_servers(m_servers, (int)(sum_cpu+0.99), (int)(sum_ram+0.99));

    return m_coarse_init->solve(true);
}

std::vector<int> manager::coarse_init()
{
    auto task = m_tasks.at(m_current_day).cmd;
    float sum_cpu = 0, sum_ram = 0;
    for (const auto& t : task)
    { // 遍历当前所有任务
        if (t.first == "add")
        {
            int vm_type_id = t.second.second;
            sum_cpu += m_VMs.at(vm_type_id).m_CPU_num;
            sum_ram += m_VMs.at(vm_type_id).m_RAM;
        }
    }
    for(auto s:m_serverss_ids)
    {
        //  sum_cpu -= 0.05 * (m_purchase_servers[s].get_CPU_left_A() + m_purchase_servers[s].get_CPU_left_B());
        // sum_ram -= 0.05 * (m_purchase_servers[s].get_RAM_left_A() + m_purchase_servers[s].get_RAM_left_B());
        float rate_A =  m_purchase_servers[s].get_occupancy_factor_A()<0.5?1:0;
        float rate_B =  m_purchase_servers[s].get_occupancy_factor_B()<0.5?1:0;

        sum_cpu -= (rate_A * m_purchase_servers[s].get_CPU_left_A() + rate_B * m_purchase_servers[s].get_CPU_left_B());
        sum_ram -= (rate_A * m_purchase_servers[s].get_RAM_left_A() + rate_B * m_purchase_servers[s].get_RAM_left_B());
    }
    sum_cpu = sum_cpu < 0 ? 0 : sum_cpu;
    sum_ram = sum_ram < 0 ? 0 : sum_ram;
    m_coarse_init->set_all_servers(m_servers, sum_cpu, sum_ram);

    return m_coarse_init->solve(true);
}

// 通过尝试的结果，按照实际的流程来赋值
void manager::assign_by_try()
{
    // try和实际的服务器id不一致，给一个映射表
    std::unordered_map<int,int> servers_map;
    std::vector<int> add_try_servers;
    // 购买服务器
    int exist_servers_num = m_serverss_ids.size();
    for(size_t i = 0;i < (m_try_serverss_ids.size() - exist_servers_num);i ++)
    {
        // 拿到了服务器id
        int try_server_id = m_try_serverss_ids.at(exist_servers_num + i);
        // 对应服务器的类型
        int server_type = m_try_purchase_servers[try_server_id].get_type();
        // 实际去购买
        try_purchase_server(++m_server_id,server_type);
        // 建立表
        servers_map.insert(make_pair(try_server_id,m_server_id));
        add_try_servers.emplace_back(try_server_id);
    }
    // 交换操作
     for(auto op:m_mig_ops)
     {// 遍历所有旧的虚拟机

         auto iter = servers_map.find(op.m_server_to);
         if(iter == servers_map.end())
         {// 没有找到说明是在两个旧服务器上进行的迁移
             try_migrate_VM(op.m_vm_id,op.m_server_to,op.m_type);
         }
         else
         {
             // 真正的id
             int current_server_id = servers_map[op.m_server_to];
             // 进行迁移操作
             try_migrate_VM(op.m_vm_id,current_server_id,op.m_type);
         }

     }
    // 按照命令执行部分
    auto daily_task = m_tasks.at(m_current_day).cmd;
    for(const auto& task:daily_task)
    {// 遍历所有任务
        if(task.first == "add")
        {// 添加
            // 找到try中的虚拟机
            auto vm = m_try_deploy_VMs[task.second.first];
            // try中的服务器id
            int try_server_id = vm.get_server_id();
            int node_type = vm.get_type();
            // // 得到实际的服务器id
            auto iter = servers_map.find(try_server_id);
            int server_id = 0;
            if(iter == servers_map.end())
            {
                server_id = try_server_id;
            }
            else
            {
                server_id = servers_map[try_server_id];
            }
            try_deploy_VM(task.second.first,task.second.second,server_id,node_type);
        }
        else
        {// 删除我就直接删掉了
            try_de_deploy_VM(task.second.first);
        }
    }
    // 确保try部分和实际的是一样的
    m_try_cost = m_purchase_cost;
    m_try_serverss_ids = m_serverss_ids;
    // 由于服务器的id会错乱因此需要重新分配id
    for(const auto& task:daily_task)
    {// task 中保存了所有新虚拟机的id值
        if(task.first == "add")
        {
            // 尝试的服务器id
            int try_server_id = m_try_deploy_VMs[task.second.first].get_server_id();
            // 设置为old
            m_try_deploy_VMs[task.second.first].set_old();
            // 查找真正的
            auto iter = servers_map.find(try_server_id);
            if(iter == servers_map.end())
            {// 没有找到，不需要进行操作
                continue;
            }
            else
            {
                m_try_deploy_VMs[task.second.first].set_server_id(servers_map[try_server_id]);
            }
        }
    }

    std::vector<int> add_server_ids;
    for (int & add_try_server : add_try_servers)
    { // 遍历所有新买的服务器
        if (add_try_server != servers_map[add_try_server])
        {
            int server_id = servers_map[add_try_server];
            m_try_purchase_servers.erase(add_try_server);
            add_server_ids.emplace_back(server_id);
        }
        else
        {
            m_try_purchase_servers[add_try_server].set_old();
        }
    }
    for (int & add_server_id : add_server_ids)
    {
        auto s = m_purchase_servers[add_server_id];
        m_try_purchase_servers.insert(make_pair(add_server_id, s));
        m_try_purchase_servers[add_server_id].set_old();
    }
    try_cal_cost();
}
// 迁移操作
void manager::try_migrate()
{
    std::vector<std::pair<int,server_data>> servers;
    std::vector<std::vector<std::pair<int,virtual_machine_data>>> VMs;
    for(int & m_try_serverss_id : m_try_serverss_ids)
    {//
        std::vector<std::pair<int,virtual_machine_data>> temp;
        int server_id = m_try_serverss_id;
        servers.emplace_back(
            make_pair(m_try_serverss_id,
             m_try_purchase_servers[server_id].get_data()));
        auto vm_id = m_try_purchase_servers[server_id].get_VM_ids();
        for(int id : vm_id)
        {
            temp.emplace_back(make_pair(id,
                m_try_purchase_servers[server_id].get_VM()[id]));
        }
        VMs.emplace_back(temp);
    }
    //m_migrate_op = m_migrate->try_igrate(servers,VMs);
}
void manager::my_try_migrate()
{
    int max_iter = 1000;
    int last_num = 0;
    int max_migrate_num = (int)((float)m_VMs_id.size() * 0.005f);
    while (max_migrate_num != 0)
    {// try migrate
        for(int i = 0;i < m_serverss_ids.size() - 1;i ++)
        {// for all servers
            auto server_id = m_serverss_ids.at(i);
            auto next_id = (rand() % (m_serverss_ids.size() - 1-i))+i+1;
            auto server2_id = m_serverss_ids.at(next_id);
            // migrate between server && server2
            int vm_id = 0,node_type = 0;// need to migrate
            if(m_purchase_servers[server_id].get_left() > m_purchase_servers[server_id].get_left())
            {
                if(migrate_two_server(server_id,server2_id,vm_id,node_type))
                {
                    max_migrate_num --;
                    if(max_migrate_num == 0)
                    {
                        break;
                    }
                }
                // try migrate
                if(vm_id != -1 && node_type != -1)
                {
                    try_migrate_VM(vm_id,server2_id,node_type,true);
                }

            }
            else
            {
                if(migrate_two_server(server2_id,server_id,vm_id,node_type))
                {
                    max_migrate_num --;
                    if(max_migrate_num == 0)
                    {
                        break;
                    }
                }
                // try migrate
                if(vm_id != -1 && node_type != -1)
                {
                    try_migrate_VM(vm_id,server_id,node_type,true);
                }
            }
            max_iter --;
        }
        if(last_num == max_migrate_num || max_iter < 0)
        {
            break;
        }
        last_num = max_migrate_num;

    }
}

bool manager::migrate_two_server(int from_server,int to_server,int &vm_id,int &node_type)
{
    bool is_old =  false;
    auto server1 = m_try_purchase_servers[from_server];
    auto server2 = m_try_purchase_servers[to_server];
    vm_id = -1;
    node_type = -1;

    int max = 0;
    auto VMs = server1.get_VM_ids();
    for(auto id:VMs)
    {
        auto vm = m_try_deploy_VMs[id];
        if(vm.get_server_type() == A || vm.get_server_type() == B)
        {// single
            int tmp = 0;
            int node = -1;
            if(server2.get_CPU_left_A() > vm.get_CPU() &&
                server2.get_RAM_left_A() > vm.get_RAM())
            {
                tmp = vm.get_CPU() + vm.get_RAM();
                node = A;
            }
            else if(server2.get_CPU_left_B() > vm.get_CPU() &&
                server2.get_RAM_left_B() > vm.get_RAM())
            {
                tmp = vm.get_CPU() + vm.get_RAM();
                node = B;
            }
            if(tmp > max)
            {
                max = tmp;
                vm_id = id;
                node_type = node;
            }
        }
        else
        {// double
            int tmp = 0;
            if(server2.get_CPU_left_A() > vm.get_CPU()/2 &&
               server2.get_RAM_left_A() > vm.get_RAM()/2 &&
               server2.get_CPU_left_B() > vm.get_CPU()/2 &&
               server2.get_RAM_left_B() > vm.get_RAM()/2)
            {
                tmp = vm.get_CPU() + vm.get_RAM();
            }
            if(tmp > max)
            {
                max = tmp;
                vm_id = id;
                node_type = AB;
            }
        }
    }
    is_old = m_try_deploy_VMs[vm_id].get_data().is_old;
    return is_old;
}

// 分配操作
void manager::try_distribution()
{
    std::vector<int> servers_type_id;
    std::vector<std::vector<int>> VMs_type_id;
    std::vector<int> left_CPU_A;
    std::vector<int> left_RAM_A;
    std::vector<int> left_CPU_B;
    std::vector<int> left_RAM_B;
    for(int server_id : m_try_serverss_ids)
    {
        servers_type_id.emplace_back(m_try_purchase_servers[server_id].get_type());
        VMs_type_id.emplace_back(m_try_purchase_servers[server_id].get_VM_ids());
        left_CPU_A.emplace_back(m_try_purchase_servers[server_id].get_CPU_left_A());
        left_RAM_A.emplace_back(m_try_purchase_servers[server_id].get_RAM_left_A());
        left_CPU_B.emplace_back(m_try_purchase_servers[server_id].get_CPU_left_B());
        left_RAM_B.emplace_back(m_try_purchase_servers[server_id].get_RAM_left_B());
    }
    m_distribution_op = m_distribution->try_violence_distribution(servers_type_id,
    VMs_type_id,m_tasks.at(m_current_day),left_CPU_A,left_RAM_A,left_CPU_B,left_RAM_B);
}

std::vector<distribution_data> manager::try_fine_distribution(std::queue<int> task_ids)
{

}
// 尝试删除掉新购买的服务器
//  new_server_ids 所有新购买的服务器id
void manager::try_delet_unused(std::vector<int> new_server_ids)
{
    int delet_num = 0;
    for(int i = 0;i < new_server_ids.size();i ++)
    {
        if(!m_try_purchase_servers[new_server_ids[i]].is_power_on())
        {// 如果当前服务器没有开机，处于空载状态
            // 删除掉服务器 
             try_delet_server(new_server_ids[i]);
             delet_num ++;
        }
    }
    //std::cerr<<"delet num"<<delet_num<<std::endl;
}
// 尝试删除掉服务器，这在实际中是不存在的，仅在尝试的时候使用
bool manager::try_delet_server(int server_id)
{
    auto iter = m_try_purchase_servers.find(server_id);
    if(iter == m_try_purchase_servers.end())
    {// 没有找到
        std::cerr<<"can not delet server unexits!!"<<std::endl;
        return false;
    }
    if(iter->second.get_VM_num() != 0)
    {// 服务器里面还有剩余的
        std::cerr<<"can not delet servers with VMs!!!"<<std::endl;
        return false;
    }
    m_try_purchase_servers.erase(server_id);
    return true;
}

void manager::try_fine_purchase()
{
    int remain_task = m_tasks.at(m_current_day).cmd.size();
    int server_id_cnt = m_purchase_servers.size();
    std::unordered_map<int,distribution_data> tasks;
    int current_task = 0;
    // get all tasks
    tasks.reserve(remain_task);
    for(int i = 0;i < remain_task;i ++)
    {
        tasks.insert(make_pair(i,distribution_data()));
    }
    std::queue<int> task_ids_30;
    int last_task_num = 0;
    while (remain_task != 0)
    {
        int remain_distribution = 0;
        while (remain_distribution == 0)
        {
            //1. get 30 tasks
            int tmp = current_task;
            for(int i = tmp;i < tmp + 30 - last_task_num;i ++)
            {
                task_ids_30.push(i);
                current_task ++;
            }
            //2. cal distribution
            auto result = try_fine_distribution(task_ids_30);
            //3. try distribution
            auto temp_id = task_ids_30;
            for(int i = 0;i < task_ids_30.size();i ++)
            {
                auto id = temp_id.front();
                temp_id.pop();
                if(result.at(i).is_distribution)
                {
                    tasks.at(id) = result.at(i);
                    remain_task --;
                }
                else
                {
                    remain_distribution ++;
                }
            }
        }
        //4. get 30 - remain_distribution task
        int tmp = current_task;
        for(int i = tmp;i < tmp + 30 - remain_distribution;i ++)
        {
            task_ids_30.push(i);
            current_task ++;
        }
        last_task_num = 30;

        //5. buy some server
        auto init = fine_init(task_ids_30);
        // 尝试购买
        for (int i = 0; i < init.size(); i++)
        {
            for (int j = 0; j < init.at(i); j++)
            {
                try_purchase_server(++server_id_cnt, i, true);
            }
        }
    }
}
float manager::cal_score(int node_type,int day,int need_CPU,int need_RAM,
                         int CPU,int RAM,int left_CPU,int left_RAM,
                         int left_CPU_B,int left_RAM_B)
{
    float score = 0;
    int power_cost = 0;
    int all_day = m_tasks.size();
    float match_rate;
    float balance_rate;
    // cal score
    if(node_type == 0)
    {// double
        if(need_CPU/2 > left_CPU || need_CPU/2 > left_CPU_B ||
           need_RAM/2 > left_RAM || need_RAM/2 > left_RAM_B)
        {// RAM or CPU is not enough
            return 0;
        }
        if(CPU == (left_CPU + left_CPU_B) && RAM == (left_RAM + left_RAM_B))
        {
            power_cost =  day;
        }
        float rate_A = powf(2.f * (float)left_CPU / (float)CPU - 2.f * (float)left_RAM / (float)RAM,2);
        float rate_B = powf(2.f * (float)left_CPU_B / (float)CPU - 2.f * (float)left_RAM_B / (float)RAM,2);
        left_CPU -= need_CPU/2;
        left_RAM -= need_RAM/2;
        left_CPU_B -= need_CPU/2;
        left_RAM_B -= need_RAM/2;
        float after_rate_A = powf(2.f * (float)left_CPU / (float)CPU - 2.f * (float)left_RAM / (float)RAM,2);
        float after_rate_B = powf(2.f * (float)left_CPU_B / (float)CPU - 2.f * (float)left_RAM_B / (float)RAM,2);
        balance_rate = (rate_A + rate_B) - (after_rate_A + after_rate_B);
    }
    else
    {// single
        if(need_CPU > left_CPU || need_RAM > left_RAM)
        {// RAM or CPU is not enough
            return 0;
        }
        if(CPU == left_CPU*2 && RAM == left_RAM*2)
        {
            power_cost = day;
        }
        float rate_A = powf(2.f * (float)left_CPU / (float)CPU - 2.f * (float)left_RAM / (float)RAM,2);
        left_CPU -= need_CPU;
        left_RAM -= need_RAM;
        float after_rate_A = powf(2.f * (float)left_CPU / (float)CPU - 2.f * (float)left_RAM / (float)RAM,2);
        balance_rate = rate_A - after_rate_A;
    }
    balance_rate = balance_rate<0?0:balance_rate;
    // match rate
    match_rate = powf((((float)need_CPU/(float)need_RAM) - ((float)CPU/(float)RAM)),2);
    score = 1.f / (0.001f * (float)power_cost + 1.f) + 1/(match_rate + 1) + 4 * balance_rate;

    return score;
}
void manager::cal_MAX_CPU_RAM(int &MAX_CPU,int &MAX_RAM)
{
    int CPU = 0,RAM = 0;

    for(const auto& task:m_tasks)
    {
        int del_CPU = 0;
        int del_RAM = 0;
        for(const auto& cmd:task.cmd)
        {
            int vm_type = cmd.second.second;
            auto vm = m_VMs[vm_type];
            if(cmd.first == "add")
            {

                CPU += vm.m_CPU_num;
                RAM += vm.m_RAM;
            }
            else
            {
                del_CPU += vm.m_CPU_num;
                del_RAM += vm.m_RAM;
                CPU -= vm.m_CPU_num;
                RAM -= vm.m_RAM;
            }
            if(MAX_CPU < CPU)
            {
                MAX_CPU = CPU;
            }
            if(MAX_RAM < RAM)
            {
                MAX_RAM = RAM;
            }
        }
        //std::cerr<<"del CPU: "<<del_CPU<<"del RAM: "<<del_RAM<<std::endl;
    }
}

// 主要调用的函数，处理所有天的数据
void manager::processing()
{
    // 初始化的时候进行一些统计数据
    int server_num  = -1;
    int MAX_CPU = 0;
    int MAX_RAM = 0;
    cal_MAX_CPU_RAM(MAX_CPU,MAX_RAM);

    // 初始化一些变量
    //clock_start();
    m_coarse_init = new Integer_program(m_serverss_ids.size());
    m_distribution = new distribution(m_servers,m_VMs);
    m_migrate = new migrate();
    m_coarse_init->set_all_servers(m_servers, 1.02 * MAX_CPU, 1.02 * MAX_RAM);
    auto init = m_coarse_init->solve(true);
    // 尝试购买
    for (int i = 0; i < init.size(); i++)
    {
        for (int j = 0; j < init.at(i); j++)
        {
            try_purchase_server(++server_num, i, true);
        }
    }
    // 开始遍历所有天的操作
    for(int day = 0;day < get_days();day ++)
    {
        // 初步计算需要多少
//        auto init = coarse_init();
//        // 尝试购买
//        for (int i = 0; i < init.size(); i++)
//        {
//            for (int j = 0; j < init.at(i); j++)
//            {
//                try_purchase_server(++server_num, i, true);
//            }
//        }
        // 进行分配操作
        //my_try_migrate();
        auto task = m_tasks.at(m_current_day);
        for(const auto& cmd:task.cmd)
        {
            int vm_id = cmd.second.second;
            if(cmd.first == "add")
            {
                int server_id = 0;
                int node_type = 0;
                float score = 0;
                float max_score = 0;
                for(auto& s:m_try_purchase_servers)
                {// for all servers
                    int node = -1;
                    if(m_VMs[vm_id].m_is_double_node)
                    {// double node
                        score = cal_score(0,s.second.get_daily_cost(),
                                          m_VMs[vm_id].m_CPU_num,m_VMs[vm_id].m_RAM,
                                          s.second.get_CPU(),s.second.get_RAM(),
                                          s.second.get_CPU_left_A(),s.second.get_RAM_left_A(),
                                          s.second.get_CPU_left_B(),s.second.get_RAM_left_B());
                        node = 2;
                    }
                    else
                    {// single node
                        float score_A = cal_score(1,s.second.get_daily_cost(),
                                                  m_VMs[vm_id].m_CPU_num,m_VMs[vm_id].m_RAM
                                                  ,s.second.get_CPU(),s.second.get_RAM(),
                                                  s.second.get_CPU_left_A(),s.second.get_RAM_left_A());
                        float score_B = cal_score(1,s.second.get_daily_cost(),
                                                  m_VMs[vm_id].m_CPU_num,m_VMs[vm_id].m_RAM
                                                  ,s.second.get_CPU(),s.second.get_RAM(),
                                                  s.second.get_CPU_left_B(),s.second.get_RAM_left_B());
                        if(score_A > score_B)
                        {
                            score = score_A;
                            node = 0;
                        }
                        else
                        {
                            score = score_B;
                            node = 1;
                        }
                    }
                    if(max_score < score)
                    {
                        max_score = score;
                        server_id = s.second.get_id();
                        node_type = node;
                    }
                }
                if(max_score == 0)
                {
                    int id = 0;
                    for(const auto& s:m_servers)
                    {
                        if(m_VMs[cmd.second.second].m_is_double_node)
                        {
                            int lc = s.m_CPU_num - m_VMs[cmd.second.second].m_CPU_num*10;
                            int lr = s.m_RAM - m_VMs[cmd.second.second].m_RAM*10;
                            if(lc > 0 && lr > 0)
                            {
                                id = s.m_type;
                            }
                        }
                        else
                        {
                            int lc = s.m_CPU_num/2 - m_VMs[cmd.second.second].m_CPU_num*10;
                            int lr = s.m_RAM/2 - m_VMs[cmd.second.second].m_RAM*10;
                            if(lc > 0 && lr > 0)
                            {
                                id = s.m_type;
                            }
                        }
                    }
                    try_purchase_server(++server_num,id, true);
                    server_id = server_num;
                    //std::cerr<<"buy"<<std::endl;
                }
                try_deploy_VM(cmd.second.first,cmd.second.second,server_id,node_type,false,true);
            }
            else
            {
                try_de_deploy_VM(cmd.second.first,true);
            }
        }
//        try_distribution();
//        int task_num = 0;
//        // 尝试进行分配
//        for(const auto& op:m_distribution_op)
//        {
//            if(op.distribution_type == add)
//            {// 添加服务器
//                try_purchase_server(op.server_id,op.server_type,true);
//                server_num ++;
//                // temp
//                auto c = m_tasks.at(day).cmd.at(task_num);
//                auto vm = m_VMs[c.second.second];
//                try_deploy_VM(c.second.first,c.second.second,op.server_id,vm.m_is_double_node?AB:A,false,true);
//                task_num ++;
//            }
//            else if(op.distribution_type == norm)
//            {// 正常部署或者删除虚拟机
//                auto c = m_tasks.at(day).cmd.at(task_num);
//                if(c.first == "add")
//                {// 部署虚拟机
//                    try_deploy_VM(c.second.first,c.second.second,op.server_id,op.node_type,false,true);
//                }
//                else
//                {// 删除虚拟机
//                    try_de_deploy_VM(c.second.first,true);
//                }
//                 task_num ++;
//            }
//            else if(op.distribution_type == erase)
//            {// 删除服务器
//                try_delet_server(op.server_id);
//            }
//        }
        //std::cerr<<"cost cost time in ms:"<<clock_end()<<std::endl;
        //clock_start();
        // 迁移操作

        // 尝试进行迁移
        // for(auto op:m_migrate_op)
        // {
        //     try_purchase_server(op.vm_id,op.node_type,true);
        // }
        //std::cerr<<"mig cost time in ms:"<<clock_end()<<std::endl;
        //clock_start();
        // 计算当天的电费
		try_cal_cost(true);// 更新尝试结果的电费
        // 根据迁移结果来确定最终当天的结果
        assign_by_try();
        // 一天结束后处理的操作
        finish_oneday();// 一天结束的标志
        //std::cerr<<"assign cost time in ms:"<<clock_end()<<std::endl;
        //std::cerr<<"finish day"<<m_current_day<<std::endl;

#ifdef test
        float rate = 0;
        int num = 0;
        for(int & m_serverss_id : m_serverss_ids)
        {
            if(m_purchase_servers[m_serverss_id].is_power_on())
            {
                rate += m_purchase_servers[m_serverss_id].get_occupancy_factor_A();
                rate += m_purchase_servers[m_serverss_id].get_occupancy_factor_B();
                num ++;
            }
        }
        rate /= (2*num);
        std::cerr<<"day: "<<m_current_day<<"rate: "<<rate<<"cost:"<<m_purchase_cost + m_power_cost<<std::endl;
        statistic_busy_rate(m_current_day);
        sum_cost.push_back(m_purchase_cost+m_power_cost);
        hard_cost.push_back(m_purchase_cost);
        ele_cost.push_back(m_power_cost);
#endif
    }
#ifdef test
    writetotxt();
#endif
}

float manager::try_oneday(std::vector<int> distribution, std::vector<int> node_type)
{
    auto current_day_task = m_tasks.at(m_current_day).cmd; // 当前天的任务
    auto current_serve = m_purchase_servers;

    // 在当前状态下各种服务器存在多少CPU和RAM的剩余
    std::vector<int> left_CPU_A;
    std::vector<int> left_RAM_A;
    std::vector<int> left_CPU_B;
    std::vector<int> left_RAM_B;

    left_CPU_A.resize(m_servers.size());
    left_RAM_A = left_CPU_B = left_RAM_B = left_CPU_A;

    for (auto id : m_serverss_ids)
    { // 遍历当前买的所有服务器id
        auto server = m_purchase_servers[id];
        left_CPU_A.at(server.get_type()) += server.get_CPU_left_A();
        left_RAM_A.at(server.get_type()) += server.get_RAM_left_A();
        left_CPU_B.at(server.get_type()) += server.get_CPU_left_B();
        left_RAM_B.at(server.get_type()) += server.get_RAM_left_B();
    }
    // 遍历所有的命令
    for (auto & i : current_day_task)
    {
        if (i.first == "add")
        { // 添加操作
        }
        else
        { // 删除操作
            // 找到需要删除的虚拟机
            auto iter_vm = m_deploy_VMs.find(i.second.first);
            if (iter_vm == m_deploy_VMs.end())
            {
                std::cerr << "can not delet unexit VM" << std::endl;
                return 1e50;
            }
            auto server_id = iter_vm->second.get_server_id(); // 找到对应的服务器id
            auto iter_server = m_purchase_servers.find(server_id);
            if (iter_server == m_purchase_servers.end())
            {
                std::cerr << "can not find corresponde server " << std::endl;
                return 1e50;
            }
        }
    }
    return 0;
}

// 直接赋值一天的数据
float manager::assign_oneday(int day, std::vector<int> distribution, std::vector<int> node_type)
{
    return 0;
}

void manager::result()
{
    int day = m_tasks.size();
    for (int i = 0; i < m_operators.days(); i++)
    { // 遍历所有天
        auto op = m_operators.get_operator(i);
        // 当前购买服务器
        std::cout << "(purchase, " << op.m_purchases.size() << ")" << std::endl;

        auto names = m_operators.get_servers_name(i);
        for(auto & name : names)
        {
            std::cout << "(" << name << ", " << op.m_purchases[name].num << ")" << std::endl;
        }
        // 当前迁移服务器
//        std::cout << "(migration, " << 0 << ")" << std::endl;
        std::cout << "(migration, " << op.m_migrates.size() << ")" << std::endl;
        for (auto m : op.m_migrates)
        {
            if (m.is_double)
            { // 双节点
                std::cout << "(" << m.server_from_id << ", " << m.server_to_id << ")" << std::endl;
            }
            else
            { // 单节点
                std::cout << "(" << m.server_from_id << ", " << m.server_to_id << ", " << m.node << ")" << std::endl;
            }
        }
        // 当前部署
        for (const auto& d : op.m_deploys)
        {
            int map_id = m_operators.m_server_id_map[d.server_id];
            if (d.is_double)
            { // 双节点
                std::cout << "(" << map_id << ")" << std::endl;
            }
            else
            {
                std::cout << "(" << map_id << ", " << d.node << ")" << std::endl;
            }
        }
        if(i == day - 1)
        {
            return;
        }
    }
}

#ifdef test

void manager::readTxt(const string &inputFile)
{
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1)
        std::cerr << "fail to open files" << std::endl;

    struct stat sb;
    fstat(fd, &sb);
    char *buffer = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0); // 返回的是指向映射区域首地址的指针
    if (buffer == nullptr || buffer == (void *)-1)
    {
        close(fd);
        exit(-1);
    }
    close(fd);

    int choice = 1; //1：读服务器 2：读虚拟机 3：读操作数
    int val = 0;
    while (*buffer)
    {
        // 开始进行数据读取的分析
        if (*buffer >= '0' && *buffer <= '9')
        {
            val = val * 10 + (*buffer - '0');
        }

        else if (*buffer == '\n')
        {
            if (choice == 1)
            {
                choice++;
                m_servers.reserve(val);
                m_server_map.reserve(val);
                int server_nums = val; // 服务器总数
                val = 0;
                //开始输入接下来的server_nums行
                int id = 0;
                for (int row = 0; row < server_nums; row++)
                {
                    int coreNum = 0, memory = 0, firstCost = 0, onCost = 0;
                    string type;
                    buffer++; // 进入新的一行
                    string temp;
                    while (*buffer != '\n')
                    {
                        temp.push_back(*buffer);
                        buffer++;
                    }
                    // 分割字符串temp，temp的样式：(host0Y6DP, 300, 830, 141730, 176)
                    int len = temp.size();
                    string t;
                    int elemNum = 0;
                    for (int n = 1; n < len; n++)
                    {

                        if (temp[n] == ',' || temp[n] == ')')
                        {
                            if (elemNum == 0)
                            {
                                //server[row].type = t;
                                type = t;
                                t.clear();
                            }
                            else if (elemNum == 1)
                            {
                                //server[row].coreNum = stoi(t);
                                coreNum = stoi(t);
                                t.clear();
                            }
                            else if (elemNum == 2)
                            {
                                //server[row].
                                memory = stoi(t);
                                t.clear();
                            }
                            else if (elemNum == 3)
                            {
                                //server[row].
                                firstCost = stoi(t);
                                t.clear();
                            }
                            else
                            {
                                //server[row].
                                onCost = stoi(t);
                                t.clear();
                            }
                            elemNum++;
                            continue;
                        }
                        t.push_back(temp[n]);
                    }
                    m_servers.emplace_back(server_data(id, coreNum, memory, firstCost, onCost, type));
                    m_server_map.insert(make_pair(type, id));
                    id++;
                }
            }
            else if (choice == 2)
            {
                choice++;
                int vm_nums = val; // 虚拟机类型数目
                m_VMs.reserve(vm_nums);
                m_VM_map.reserve(vm_nums);
                val = 0;
                //开始输入接下来的vm_nums行
                int id = 0;
                for (int row = 0; row < vm_nums; row++)
                {
                    buffer++; // 进入新的一行
                    string temp;
                    while (*buffer != '\n')
                    {
                        temp.push_back(*buffer);
                        buffer++;
                    }
                    // 分割字符串temp，temp的样式：(vm38TGB, 124, 2, 1)
                    int len = temp.size();
                    string t;
                    int elemNum = 0;
                    int needcoreNum = 0, needMemory = 0, isdoubleNode = 0;
                    string type;
                    for (int n = 1; n < len; n++)
                    {
                        if (temp[n] == ',' || temp[n] == ')')
                        {
                            if (elemNum == 0)
                            {
                                //vm[row].type = t;
                                type = t;
                                t.clear();
                            }
                            else if (elemNum == 1)
                            {
                                //vm[row].
                                needcoreNum = stoi(t);
                                t.clear();
                            }
                            else if (elemNum == 2)
                            {
                                //vm[row].
                                needMemory = stoi(t);
                                t.clear();
                            }
                            else
                            {
                                //vm[row].
                                isdoubleNode = stoi(t);
                                t.clear();
                            }
                            elemNum++;
                            continue;
                        }
                        t.push_back(temp[n]);
                    }
                    m_VMs.emplace_back(virtual_machine_data(id, needcoreNum, needMemory, isdoubleNode));
                    m_VM_map.insert(make_pair(type, id));
                    id++;
                }
            }
            else if (choice == 3)
            {
                int days = val; // 总的运行天数:1～days
                val = 0;
                choice++;
                int count = 0; //操作数的索引下标
                //开始输入接下来的days天的操作
                m_tasks.reserve(days);
                m_operators.set_days(days);
                for (int day = 0; day < days; day++)
                {
                    task T;
                    // 读取每一天的操作数
                    buffer++;    // 进入下一行
                    string temp; // 存放一行
                    while (*buffer != '\n')
                    {
                        temp.push_back(*buffer);
                        buffer++;
                    }
                    int operations = stoi(temp);
                    temp.clear();

                    //operation_nums_perday[day] = operations; // 每一天的操作数存入数组中
                    T.cmd.reserve(operations);
                    // 读取当天所有操作的具体指令
                    string type;
                    int index, vm_id;
                    for (int i = 0; i < operations; i++)
                    {
                        buffer++; // 进入当天操作指令的第一行
                        string t;
                        while (*buffer != '\n')
                        {
                            t.push_back(*buffer);
                            buffer++;
                        }
                        // 分割字符串t,t的样式：(add, vmVDAZV, 381492167)或(del, 264022204)
                        int len = t.size();
                        string t_t;
                        int elemNum = 0;
                        for (int n = 1; n < len; n++)
                        {
                            if (t[n] == ',' || t[n] == ')')
                            {
                                if (elemNum == 0)
                                {
                                    //operation[count].type = t_t;
                                    type = t_t;
                                    t_t.clear();
                                }
                                else if (elemNum == 1 && type == "add")
                                {
                                    t_t.erase(0, 1); //删除头部空格
                                    string vm_type = t_t;
                                    vm_id = m_VM_map[vm_type];
                                    t_t.clear();
                                }
                                else
                                {
                                    index = stoi(t_t);
                                    t_t.clear();
                                }
                                elemNum++;
                                continue;
                            }
                            t_t.push_back(t[n]);
                        }
                        T.cmd.emplace_back(make_pair(type, make_pair(index, vm_id)));
                        count++;
                    }
                    m_tasks.emplace_back(T);
                }
            }
        }
        buffer++;
    }
    munmap(buffer, sb.st_size); // 解除内存映射
}
#endif

bool manager::vertify_result()
{
    std::vector<int> left_CPU_A;
    std::vector<int> left_RAM_A;
    std::vector<int> left_CPU_B;
    std::vector<int> left_RAM_B;
    std::vector<int> server_type_id;
    std::unordered_map<int,int> vms;
    int day = m_tasks.size();
    // for every day
    for(int i = 0;i < day; i++)
    {
        auto op = m_operators.get_operator(i);
        // purchase

        // migrate

        // task
        auto daily_task = m_tasks.at(day).cmd;
        for(auto t:daily_task)
        {
            if(t.first == "add")
          {

            }
            else
            {// del

            }
        }
    }

    return true;
}


void manager::readTxtbyStream()
{
    //std::string test = "/home/lyc/21-Code-Craft/training-data/training-2.txt";
    //std::freopen(test.c_str(), "rb", stdin);// 文件重定向
    // 标准输入流读取服务器相关信息
    int serverNum = 0;
    cin >> serverNum;
    m_servers.reserve(serverNum); // 服务器队列初始化
    m_server_map.reserve(serverNum);
    string serverType, cpuCores, memorySize, serverCost, powerCost;
    for (int i = 0; i < serverNum; i++)
    {
        cin >> serverType >> cpuCores >> memorySize >> serverCost >> powerCost;
        // 分割字符串操作读取数据
        int coreNum = 0, memory = 0, firstCost = 0, onCost = 0;
        string type;
        type = serverType.substr(1, serverType.size() - 2);
        coreNum = stoi(cpuCores.substr(0, cpuCores.size() - 1));
        memory = stoi(memorySize.substr(0, memorySize.size() - 1));
        firstCost = stoi(serverCost.substr(0, serverCost.size() - 1));
        onCost = stoi(powerCost.substr(0, powerCost.size() - 1));
        m_servers.emplace_back(server_data(i, coreNum, memory, firstCost, onCost, type));
        m_server_map.insert(make_pair(type, i));
    }
    // 标准输入流读取虚拟机相关信息
    int vmNum = 0;
    cin >> vmNum;
    string vmType, needCpuCores, needMemorySize, isTwoNode;
    for (int i = 0; i < vmNum; i++)
    {
        cin >> vmType >> needCpuCores >> needMemorySize >> isTwoNode;
        // 分割字符串操作读取数据
        int needcoreNum = 0, needMemory = 0, isdoubleNode = 0;
        string type;
        type = vmType.substr(1, vmType.size() - 2);
        needcoreNum = stoi(needCpuCores.substr(0, needCpuCores.size() - 1));
        needMemory = stoi(needMemorySize.substr(0, needMemorySize.size() - 1));
        isdoubleNode = stoi(isTwoNode.substr(0, isTwoNode.size() - 1));
        m_VMs.emplace_back(virtual_machine_data(i, needcoreNum, needMemory, isdoubleNode));
        m_VM_map.insert(make_pair(type, i));
    }
    // 标准输入流读取用户请求相关信息
    int requestdays = 0, reqNumPerDay = 0;
    cin >> requestdays;
    m_tasks.reserve(requestdays);
    m_operators.set_days(requestdays);
    string op, req_VmType, reqId;
    for (int day = 0; day < requestdays; day++)
    {
        task T;
        cin >> reqNumPerDay;
        T.cmd.reserve(reqNumPerDay);
        for (int i = 0; i < reqNumPerDay; i++)
        {
            cin >> op;
            if (op[1] == 'a')
            {
                cin >> req_VmType >> reqId;
                // 分割字符串操作读取数据
                string opType, temp_vmType;
                int vmId, vm_index;
                opType = op.substr(1, op.size() - 2);
                temp_vmType = req_VmType.substr(0, req_VmType.size() - 1);
                vmId = stoi(reqId.substr(0, reqId.size() - 1));
                vm_index = m_VM_map[temp_vmType];
                T.cmd.emplace_back(make_pair(opType, make_pair(vmId, vm_index)));
            }
            else
            {
                cin >> reqId;
                // 分割字符串操作读取数据
                string opType;
                int vmId;
                opType = op.substr(1, op.size() - 2);
                vmId = stoi(reqId.substr(0, reqId.size() - 1));
                T.cmd.emplace_back(make_pair(opType, make_pair(vmId, -1)));
            }
        }
        m_tasks.emplace_back(T);
    }
}
// 输出调试问题
void manager::output()
{
    for (auto & m_server : m_servers)
    {
        cerr << "CPU nums:" << m_server.m_CPU_num << "RAM size:" << m_server.m_RAM << "price:" << m_server.m_price << "daily cost:" << m_server.m_daily_cost << endl;
    }
    for (auto & m_VM : m_VMs)
    {
        cerr << "need CPU :" << m_VM.m_CPU_num << "need RAM :" << m_VM.m_RAM << "is double node :" << m_VM.m_is_double_node << endl;
    }
    for (size_t i = 0; i < m_tasks.size(); i++)
    {
        cerr << "day " << i << endl;
        auto C = m_tasks.at(i).cmd;
        for (auto & j : C)
        {
            cerr << j.first << "  "
                 << j.second.first << "  " << j.second.second << endl;
        }
    }
}
//计算占用率
void manager::statistic_busy_rate(int m_current_day) {
    for(int i=0;i<m_serverss_ids.size();++i){
        int seq = m_serverss_ids[i];
        auto service = m_purchase_servers[seq];
        float a_cpu = 1.f - float(service.get_CPU_left_A())/float((service.get_CPU()>>1));
        float a_ram = 1.f - float(service.get_RAM_left_A())/float((service.get_RAM()>>1));
        float b_cpu = 1.f - float(service.get_CPU_left_B())/float((service.get_CPU()>>1));
        float b_ram = 1.f - float(service.get_RAM_left_B())/float((service.get_RAM()>>1));
        vector<float> oneday_used_rate{a_cpu,a_ram,b_cpu,b_ram};
        //第一次加的服务器
        if(i >= lastdayCnt ) {
            vector<float> day = {m_current_day*1.f};
            used_rate.push_back({{day}});
            used_rate[i].push_back(oneday_used_rate);
        }else {
            //已经加过了的服务器
            used_rate[i].push_back(oneday_used_rate);
        }
    }
    lastdayCnt = m_serverss_ids.size();
}

#include <fstream>
#include <utility>
//将保存的数据写进文件
void manager::writetotxt(){
    //准备输出为txt文件
    ofstream outfile;
    ofstream ofs("world.txt");
    ofstream cost("cost.txt");
    for(int i =0;i<used_rate.size();++i){ //每一个服务器
        ofs<<used_rate[i][0][0]<<" ";
        vector<float> onedaysUsedData;
        for(int j = 1;j<used_rate[i].size();j++){  //每一个服务器每天的占用情况
            float a_cpu = used_rate[i][j][0];
            float a_ram = used_rate[i][j][1];
            float b_cpu = used_rate[i][j][2];
            float b_ram = used_rate[i][j][3];
            onedaysUsedData.push_back(a_cpu);
            onedaysUsedData.push_back(a_ram);
            onedaysUsedData.push_back(b_cpu);
            onedaysUsedData.push_back(b_ram);
        }
        for (int j = 0; j <onedaysUsedData.size(); ++j) {
            ofs<<onedaysUsedData[j]<< " ";
        }
        ofs<<"\n";
    }
    for(int i=0;i<sum_cost.size();++i){
        cost<<sum_cost[i]<<" ";
    }
    cost<<"\n";
    for(int i =0;i<hard_cost.size();++i){
        cost<<hard_cost[i]<<" ";
    }
    cost<<"\n";
    for(int i =0;i<ele_cost.size();++i){
        cost<<ele_cost[i]<<" ";
    }
    ofs.close();//关闭文件
    cost.close();
    lastdayCnt = m_serverss_ids.size();
}

