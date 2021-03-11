#include "manager.hpp"
#include <iostream>
using namespace std;

manager::manager():m_cost(0)
{
}
// 买一个id对应的服务器
// 设置服务器 id 
// 服务器类型
bool manager::purchase_server(int id,int type)
{
    if(type>m_servers.size()){
        std::cerr<<"can not find the server!!!"<<std::endl;
        return false;
    }
    m_purchase_servers.insert(std::pair<int,server>(id,server(id,m_servers.at(type))));
    m_serverss_ids.emplace_back(id);
    m_cost += m_servers.at(type).m_price;
    return true;
}
// 往servver_id 对应的服务器上部署vm_id对应的一个虚拟机
// type选择 A B 或者 AB
bool manager::deploy_VM(int vm_id, int vm_type,int server_id, int type)
{
    // 构造一个虚拟机 
    if(vm_type>m_VMs.size()){
        std::cerr<<"can not find the virtual machine !!!"<<std::endl;
        return false;
    }
    virtual_machine VM(vm_id,m_VMs.at(vm_type));
    // 插入到服务器 
    // 查找指定的服务器
    auto iter = m_purchase_servers.find(server_id);
    if(iter == m_purchase_servers.end()){
        std::cerr<<"can not find the server !!!"<<std::endl;
        return false;
    }
    if(!iter->second.add_virtual_machine(vm_id,m_VMs.at(vm_type),type)){
        return false;
    }
    if(!VM.deploy(server_id)){
        return false;
    }
    m_deploy_VMs.insert(std::pair<int,virtual_machine>(vm_id,VM)); 
    return true; 
}
// 注销掉虚拟机
bool manager::de_deploy_VM(int id)
{
    auto iter_vm = m_deploy_VMs.find(id);
    if(iter_vm == m_deploy_VMs.end()){
        std::cerr<<"can not delet unexit VM"<<std::endl;
        return false;
    }
    auto server_id = iter_vm->second.get_server_id();
    iter_vm->second.de_deploy();
    auto iter_server = m_purchase_servers.find(server_id);
    if(iter_server == m_purchase_servers.end()){
        std::cerr<<"can not find corresponde server "<<std::endl;
        return false;
    }
    if(!iter_server->second.remove_virtual_machine(id)){
        return false;
    }
    return true;
}
// 迁移虚拟机，先删除然后再添加
// 迁移虚拟机id   vm_id 
// 迁移到这个id的服务器上 server_to 
// 在这个服务器上部署类型为 type
bool manager::migrate_VM(int vm_id, int server_to, int type)
{
    auto iter_vm = m_deploy_VMs.find(vm_id);
    if(iter_vm == m_deploy_VMs.end()){
        std::cerr<<"can not migrate not exit VM!!!"<<std::endl;
        return false;
    }
    int vm_type = iter_vm->second.get_VM_id();
    if(!de_deploy_VM(vm_id)){
        return false;
    }
    if(!deploy_VM(vm_id,vm_type,server_to,type)){
        return false;
    }
    return true;
}

float manager::cal_cost()
{
    for(auto id:m_serverss_ids){
        if(m_purchase_servers[id].is_power_on()){
            m_cost += m_purchase_servers[id].get_daily_cost();
        }
    }
    return m_cost;
}

 void manager::readTxt(const string &inputFile)
 {
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1)
        std::cerr<<"fail to open files"<<std::endl;

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
                int server_nums = val;// 服务器总数 
                val = 0;
                //开始输入接下来的server_nums行
                int id = 0;
                for (int row = 0; row < server_nums; row++)
                {
                    int coreNum=0,memory=0,firstCost=0,onCost=0;
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
                    m_servers.emplace_back(server_data(id,coreNum,memory,firstCost,onCost));
                    m_server_map.insert(make_pair(type,id));
                    id ++;
                }
            }
            else if (choice == 2)
            {
                choice++;
                int vm_nums = val;// 虚拟机类型数目
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
                    int needcoreNum = 0,needMemory=0,isdoubleNode=0;
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
                    m_VMs.emplace_back(virtual_machine_data(id,needcoreNum,needMemory,isdoubleNode));
                    m_VM_map.insert(make_pair(type,id));
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
                    int index,vm_id;
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
                        T.cmd.emplace_back(make_pair(type,make_pair(index,vm_id)));
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

// 输出调试问题 
void manager::output()
{
    for(int i=0;i<m_servers.size();i++)
    {   
        cerr<<"CPU nums:"<<m_servers.at(i).m_CPU_num<<
        "RAM size:"<<m_servers.at(i).m_RAM<<
        "price:"<<m_servers.at(i).m_price<<
        "daily cost:"<<m_servers.at(i).m_daily_cost<<endl;
    }
    for(int i=0;i<m_VMs.size();i++)
    {
        cerr<<"need CPU :"<<m_VMs.at(i).m_CPU_num<<
        "need RAM :"<<m_VMs.at(i).m_RAM<<
        "is double node :"<<m_VMs.at(i).m_is_double_node<<endl;
    }
    for(int i=0;i<m_tasks.size();i++)
    {
        cerr<<"day "<<i<<endl;
        auto C = m_tasks.at(i).cmd;
        for(int j=0;j<C.size();j++)
        {
            cerr<<C.at(j).first<<"  "
            <<C.at(j).second.first<<"  "<<C.at(j).second.second<<endl;
        }
    }
}

