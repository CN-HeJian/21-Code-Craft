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
        return false;
    }
    // 判断是否超过当前服务器的资源
    if (require_RAM_A - m_RAM_left_A > 0 ||
        require_RAM_B - m_RAM_left_B > 0 ||
        require_CPU_A - m_CPU_left_A > 0 ||
        require_CPU_B - m_CPU_left_B > 0)
    {
        std::cerr<<"CPU or RAM is not enough !!!"<<std::endl;
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
        return true;
    }
    else
    {
        std::cerr<<"error:can not find the VM!!!"<<std::endl;
        return false;
    }
}