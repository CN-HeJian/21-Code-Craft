#include "server.hpp"
#include <iostream>

// 判断是否当前开机
bool server::is_power_on()
{
    return ((m_data.m_CPU_num - m_CPU_left_B - m_CPU_left_A) > 0.5);
}
// 添加一个虚拟机
bool server::add_virtual_machine(int id, virtual_machine_data VM, int type)
{
    int require_CPU_A = 0;
    int require_CPU_B = 0;
    int require_RAM_A = 0;
    int require_RAM_B = 0;
    switch (type)
    { // 根据不同的情况来进行分配资源
    case A:
        require_CPU_A = VM.m_CPU_num;
        require_RAM_A = VM.m_RAM;
        break;
    case B:
        require_RAM_B = VM.m_RAM;
        require_CPU_B = VM.m_CPU_num;
        break;
    case AB:
        require_CPU_A = VM.m_CPU_num / 2;
        require_RAM_A = VM.m_RAM / 2;
        require_RAM_B = VM.m_RAM / 2;
        require_CPU_B = VM.m_CPU_num / 2;
        break;
    default:
        std::cerr<<"get a node type not in A B or AB"<<std::endl;
        std::vector<int> i;
        i.at(0) = 1;
        return false;
    }
    // 判断是否超过当前服务器的资源
    if (require_RAM_A - m_RAM_left_A > 0 ||
        require_RAM_B - m_RAM_left_B > 0 ||
        require_CPU_A - m_CPU_left_A > 0 ||
        require_CPU_B - m_CPU_left_B > 0)
    {
        std::cerr<<"CPU or RAM is not enough !!!"<<std::endl;
        std::vector<int> i;
        i.at(1) = 0;
        return false;
    }
    else
    {
        VM.m_type = type;
        m_VM.insert(std::pair<int, virtual_machine_data>(id, VM));
        m_VM_ids.emplace_back(id);

        m_RAM_left_A -= require_RAM_A;
        m_RAM_left_B -= require_RAM_B;
        m_CPU_left_A -= require_CPU_A;
        m_CPU_left_B -= require_CPU_B;
        // 更新占用率
        float ram_rate_A = 1.f - 2.f * (float)m_RAM_left_A / (float)m_data.m_RAM;
        float ram_rate_B = 1.f - 2.f * (float)m_RAM_left_B / (float)m_data.m_RAM;
        float cpu_rate_A = 1.f - 2.f * (float)m_CPU_left_A / (float)m_data.m_CPU_num;
        float cpu_rate_B = 1.f - 2.f * (float)m_CPU_left_B / (float)m_data.m_CPU_num;
        //取比较大的
        //m_data.occupancy_factor_A = ram_rate_A>cpu_rate_A?ram_rate_A:cpu_rate_A;
        //m_data.occupancy_factor_B = ram_rate_B>cpu_rate_B?ram_rate_B:cpu_rate_B;
        //取较小的
        m_data.occupancy_factor_A = ram_rate_A<cpu_rate_A?ram_rate_A:cpu_rate_A;
        m_data.occupancy_factor_B = ram_rate_B<cpu_rate_B?ram_rate_B:cpu_rate_B;
        return true;
    }
}
// 删除一个虚拟机
bool server::remove_virtual_machine(int id)
{
    // 根据id 得到对应的虚拟机
    auto iter = m_VM.find(id);
    // 释放相关资源
    if (iter != m_VM.end())
    {
        switch (iter->second.m_type)
        {
        case A:
            m_RAM_left_A += iter->second.m_RAM;
            m_CPU_left_A += iter->second.m_CPU_num;
            break;
        case B:
            m_RAM_left_B += iter->second.m_RAM;
            m_CPU_left_B += iter->second.m_CPU_num;
            break;
        case AB:
            m_RAM_left_A += iter->second.m_RAM/2;
            m_CPU_left_A += iter->second.m_CPU_num/2;
            m_RAM_left_B += iter->second.m_RAM/2;
            m_CPU_left_B += iter->second.m_CPU_num/2;
            break;
        default:
            std::cerr<<"iter->second.m_type need to be A or B or AB,but it is %d"<<iter->second.m_type<<std::endl;
            return false;
        }
        m_VM.erase(id);
        auto it = find(m_VM_ids.begin(), m_VM_ids.end(), id);
        m_VM_ids.erase(it);
                // 更新占用率
        float ram_rate_A = 1.f - 2.f * (float)m_RAM_left_A / (float)m_data.m_RAM;
        float ram_rate_B = 1.f - 2.f * (float)m_RAM_left_B / (float)m_data.m_RAM;
        float cpu_rate_A = 1.f - 2.f * (float)m_CPU_left_A / (float)m_data.m_CPU_num;
        float cpu_rate_B = 1.f - 2.f * (float)m_CPU_left_B / (float)m_data.m_CPU_num;
        //取较大的
        //m_data.occupancy_factor_A = ram_rate_A>cpu_rate_A?ram_rate_A:cpu_rate_A;
        //m_data.occupancy_factor_B = ram_rate_B>cpu_rate_B?ram_rate_B:cpu_rate_B;
        //取较小的
        m_data.occupancy_factor_A = ram_rate_A<cpu_rate_A?ram_rate_A:cpu_rate_A;
        m_data.occupancy_factor_B = ram_rate_B<cpu_rate_B?ram_rate_B:cpu_rate_B;
        return true;
    }
    else
    {
        std::cerr<<"error:can not find the VM!!!"<<std::endl;
        std::vector<int> i;
        i.at(0) = 0;
        return false;
    }
}
// 
void server::set_old()
{
    m_data.is_old = true;
    for(auto vm:m_VM_ids)
    {
        m_VM[vm].is_old = true;
    }
}

using namespace  std;
void server::reset_type(server_data temp) {
    if(temp.m_type == m_data.m_type){
        cerr<<"same server"<<endl;
        return ;
    }
    int used_cpu_a = m_data.m_CPU_num/2-m_CPU_left_A;
    int used_cpu_b = m_data.m_CPU_num/2-m_CPU_left_B;
    int used_ram_a = m_data.m_RAM/2-m_RAM_left_A;
    int used_ram_b = m_data.m_RAM/2-m_RAM_left_B;
    m_RAM_left_A = temp.m_RAM/2-used_ram_a;
    m_CPU_left_A = temp.m_CPU_num/2-used_cpu_a;
    m_RAM_left_B = temp.m_RAM/2-used_ram_b;
    m_CPU_left_B = temp.m_CPU_num/2-used_cpu_b;

    float ram_rate_A = 1.f - 2.f * (float)m_RAM_left_A / (float)m_data.m_RAM;
    float ram_rate_B = 1.f - 2.f * (float)m_RAM_left_B / (float)m_data.m_RAM;
    float cpu_rate_A = 1.f - 2.f * (float)m_CPU_left_A / (float)m_data.m_CPU_num;
    float cpu_rate_B = 1.f - 2.f * (float)m_CPU_left_B / (float)m_data.m_CPU_num;
    m_data.occupancy_factor_A = ram_rate_A<cpu_rate_A?ram_rate_A:cpu_rate_A;
    m_data.occupancy_factor_B = ram_rate_B<cpu_rate_B?ram_rate_B:cpu_rate_B;

    m_data.m_CPU_num = temp.m_CPU_num;
    m_data.m_daily_cost = temp.m_daily_cost;
    m_data.m_type = temp.m_type;
    m_data.m_price = temp.m_price;
    m_data.m_RAM = temp.m_RAM;
    m_data.m_name = temp.m_name;
}