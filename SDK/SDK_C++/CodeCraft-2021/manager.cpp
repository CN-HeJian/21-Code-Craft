#include "manager.hpp"
#include <iostream>
using namespace std;

manager::manager() : m_cost(0)
{
}
// 买一个id对应的服务器
// 设置服务器 id
// 服务器类型
bool manager::purchase_server(int id, int server_typeId)
{
    if (server_typeId > m_servers.size() - 1)
    {
        std::cerr << "can not find the server!!!" << std::endl;
        return false;
    }
    m_purchase_servers.insert(std::pair<int, server>(id, server(id, m_servers.at(server_typeId))));
    m_serverss_ids.emplace_back(id);
    m_cost += m_servers.at(server_typeId).m_price;
    // 记录操作
    m_operators.purchase_server(m_servers.at(server_typeId).m_name);
    return true;
}
// 往servver_id 对应的服务器上部署vm_id对应的一个虚拟机
// type选择 A B 或者 AB
bool manager::deploy_VM(int vm_id, int vm_typeId, int server_id, int type, bool is_log)
{
    // 构造一个虚拟机
    if (vm_typeId > m_VMs.size() - 1)
    {
        std::cerr << "can not find the virtual machine !!!" << std::endl;
        return false;
    }
    virtual_machine VM(vm_id, m_VMs.at(vm_typeId));
    // 插入到服务器
    // 查找指定的服务器
    auto iter = m_purchase_servers.find(server_id);
    if (iter == m_purchase_servers.end())
    {
        std::cerr << "can not find the server !!!" << std::endl;
        return false;
    }
    if (!iter->second.add_virtual_machine(vm_id, m_VMs.at(vm_typeId), type))
    {
        return false;
    }
    if (!VM.deploy(server_id))
    {
        return false;
    }
    m_deploy_VMs.insert(std::pair<int, virtual_machine>(vm_id, VM));
    // 记录操作
    if (is_log)
        m_operators.deploy_VM(server_id, (type == AB), type == A ? "A" : "B");
    return true;
}
// 注销掉虚拟机
bool manager::de_deploy_VM(int vm_id)
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
    return true;
}
/*功能：迁移虚拟机，先删除然后再添加
* 输入参数：
* vm_id : 迁移虚拟机id
* server_to : 迁移到这个id的服务器上
* type : 在这个服务器上部署节点的类型
*/
bool manager::migrate_VM(int vm_id, int server_to, int type)
{
    auto iter_vm = m_deploy_VMs.find(vm_id);
    if (iter_vm == m_deploy_VMs.end())
    {
        std::cerr << "can not migrate not exit VM!!!" << std::endl;
        return false;
    }
    int vm_type = iter_vm->second.get_VM_id();
    if (!de_deploy_VM(vm_id))
    {
        return false;
    }
    if (!deploy_VM(vm_id, vm_type, server_to, type, false))
    {
        return false;
    }
    // 记录操作
    m_operators.migrate_VM(vm_id, server_to, (type == AB), type == A ? "A" : "B");
    return true;
}

float manager::cal_cost()
{
    for (auto id : m_serverss_ids)
    {
        if (m_purchase_servers[id].is_power_on())
        {
            m_cost += m_purchase_servers[id].get_daily_cost();
        }
    }
    return m_cost;
}

void manager::finish_oneday()
{
    m_operators.finish_oneday();
    m_current_day++;
}
void manager::re_begin()
{
    m_operators.re_begin();
    m_current_day = 0;
}

std::vector<int> manager::coarse_init()
{
    m_coarse_init = new Integer_program(m_serverss_ids.size());
    auto task = m_tasks.at(m_current_day).cmd;
    int sum_cpu = 0,sum_ram = 0;
    for(auto t:task)
    {// 遍历当前所有任务
        if(t.first == "add")
        {
            int vm_type_id = t.second.second;
            sum_cpu += m_VMs.at(vm_type_id).m_CPU_num;
            sum_ram += m_VMs.at(vm_type_id).m_RAM;
        }
    }
    m_coarse_init->set_all_servers(m_servers,sum_cpu*1.2,sum_ram*1.2);
    return m_coarse_init->solve(true);
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
    for (size_t i = 0; i < current_day_task.size(); i++)
    {
        if (current_day_task.at(i).first == "add")
        { // 添加操作

        }
        else
        {   // 删除操作
            // 找到需要删除的虚拟机
            auto iter_vm = m_deploy_VMs.find(current_day_task.at(i).second.first);
            if (iter_vm == m_deploy_VMs.end())
            {
                std::cerr << "can not delet unexit VM" << std::endl;
                return 1e50;
            }
            auto server_id = iter_vm->second.get_server_id();// 找到对应的服务器id
            auto iter_server = m_purchase_servers.find(server_id);
            if (iter_server == m_purchase_servers.end())
            {
                std::cerr << "can not find corresponde server " << std::endl;
                return 1e50;
            }
        }
    }
}

// 直接赋值一天的数据
float manager::assign_oneday(int day, std::vector<int> distribution, std::vector<int> node_type)
{
}

void manager::cout_result()
{
    for (int i = 0; i < m_operators.days(); i++)
    { // 遍历所有天
        auto op = m_operators.get_operator(i);
        // 当前购买服务器
        std::cout << "(purchase, " << op.m_purchases.size() << ")" << std::endl;
        auto iter = op.m_purchases.begin();
        for (int j = 0; j < op.m_purchases.size(); j++)
        {
            std::cout << "(" << iter->first << ", " << iter->second << ")" << std::endl;
            iter++;
        }
        // 当前迁移服务器
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
        for (auto d : op.m_deploys)
        {
            if (d.is_double)
            { // 双节点
                std::cout << "(" << d.server_id << ")" << std::endl;
            }
            else
            {
                std::cout << "(" << d.server_id << ", " << d.node << ")" << std::endl;
            }
        }
    }
}

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

void manager::readTxtbyStream(const string &inputFile)
{

    //std::freopen(inputFile.c_str(), "rb", stdin);// 文件重定向
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
    for (int i = 0; i < m_servers.size(); i++)
    {
        cerr << "CPU nums:" << m_servers.at(i).m_CPU_num << "RAM size:" << m_servers.at(i).m_RAM << "price:" << m_servers.at(i).m_price << "daily cost:" << m_servers.at(i).m_daily_cost << endl;
    }
    for (int i = 0; i < m_VMs.size(); i++)
    {
        cerr << "need CPU :" << m_VMs.at(i).m_CPU_num << "need RAM :" << m_VMs.at(i).m_RAM << "is double node :" << m_VMs.at(i).m_is_double_node << endl;
    }
    for (int i = 0; i < m_tasks.size(); i++)
    {
        cerr << "day " << i << endl;
        auto C = m_tasks.at(i).cmd;
        for (int j = 0; j < C.size(); j++)
        {
            cerr << C.at(j).first << "  "
                 << C.at(j).second.first << "  " << C.at(j).second.second << endl;
        }
    }
}
