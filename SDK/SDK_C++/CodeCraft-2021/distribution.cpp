#include "distribution.hpp"

distribution::distribution(std::vector<server_data>& servers, std::vector<virtual_machine_data>& VMs)
{
    m_VMs = VMs;
    m_servers = servers;

    sort_Server_Index();

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
 * @param remain_CPU_A 对应第i台服务器A节点剩余CPU容量
 * @param remain_RAM_A 对应第i台服务器A节点剩余RAM容量
 * @param remain_CPU_B 对应第i台服务器B节点剩余CPU容量
 * @param remain_RAM_B 对应第i台服务器B节点剩余RAM容量
 * @return 需要进行的操作
 *                    填充数据  std::vector<distribution_operation>  
 *                    按照如下的顺序进行
 *                    1、添加服务器命令
 *                    2、正常任务，一定要按照task的顺序来，不然就错了
 *                    3、删除服务器命令
 *          
 * */
std::vector<distribution_operation> distribution::try_violence_distribution(
        std::vector<int> servers_type_id,
        std::vector<std::vector<int>> VMs_type_id,
        task tasks,
        std::vector<int> left_CPU_A,
        std::vector<int> left_RAM_A,
        std::vector<int> left_CPU_B,
        std::vector<int> left_RAM_B)
{
        int server_cnt = 0;
        int state_AB = 0;// A = 0 , B = 1
        int server_num = servers_type_id.size();
        std::vector<distribution_operation> result;
        int cnt = 0;
        for(auto t:tasks.cmd)
        {
                cnt ++;
                distribution_operation op;
                if(t.first == "add")
                {// 如果是添加
                        if(m_VMs[t.second.second].m_is_double_node)
                        {// 双节点添加
                                int lc_A = left_CPU_A.at(server_cnt) - m_VMs[t.second.second].m_CPU_num/2;
                                int lc_B = left_CPU_B.at(server_cnt) - m_VMs[t.second.second].m_CPU_num/2;
                                int lr_A = left_RAM_A.at(server_cnt) - m_VMs[t.second.second].m_RAM/2;
                                int lr_B = left_RAM_B.at(server_cnt) - m_VMs[t.second.second].m_RAM/2;
                                // 保证当前有足够的资源
                                bool get_it = true;
                                while(lc_A < 0 || lc_B < 0 || lr_A < 0 || lr_B < 0)
                                {
                                    server_cnt ++;
                                    if(server_cnt == server_num)
                                    {
                                        server_cnt = 0;
                                        op.distribution_type = add;
                                        get_it = false;
                                        break;
                                    }

                                    lc_A = left_CPU_A.at(server_cnt) - m_VMs[t.second.second].m_CPU_num/2;
                                    lc_B = left_CPU_B.at(server_cnt) - m_VMs[t.second.second].m_CPU_num/2;
                                    lr_A = left_RAM_A.at(server_cnt) - m_VMs[t.second.second].m_RAM/2;
                                    lr_B = left_RAM_B.at(server_cnt) - m_VMs[t.second.second].m_RAM/2;

                                }
                                if(get_it)
                                {
                                    left_CPU_A.at(server_cnt) = lc_A;
                                    left_CPU_B.at(server_cnt) = lc_B;
                                    left_RAM_A.at(server_cnt) = lr_A;
                                    left_RAM_B.at(server_cnt) = lr_B;
                                    op.distribution_type = norm;
                                    op.server_id = server_cnt;
                                    op.node_type = AB;
                                }
                        }
                        else
                        {// 单节点添加
                                int lc_A = -10;
                                int lc_B = -10;
                                int lr_A = -10;
                                int lr_B = -10;
                                while (lc_A < 0 || lc_B<0 || lr_A<0 || lr_B<0)
                                {
                                        if(state_AB == 0)
                                        {// 从A节点开始添加
                                                lc_A = left_CPU_A.at(server_cnt) - m_VMs[t.second.second].m_CPU_num;
                                                lr_A = left_RAM_A.at(server_cnt) - m_VMs[t.second.second].m_RAM;
                                                lc_B = 10;
                                                lr_B = 10;
                                                if(lc_A < 0 || lr_A < 0)
                                                {
                                                        state_AB = 1;
                                                }
                                                else
                                                {
                                                        left_CPU_A.at(server_cnt) = lc_A;
                                                        left_RAM_A.at(server_cnt) = lr_A;
                                                        op.distribution_type = norm;
                                                        op.server_id = server_cnt;
                                                        op.node_type = A;
                                                }
                                        }
                                        else if(state_AB == 1)
                                        {// 添加到B节点
                                                lc_B = left_CPU_B.at(server_cnt) - m_VMs[t.second.second].m_CPU_num;
                                                lr_B = left_RAM_B.at(server_cnt) - m_VMs[t.second.second].m_RAM;
                                                lc_A = 10;
                                                lr_A = 10;
                                                if(lc_B < 0 || lr_B < 0)
                                                {
                                                        state_AB = 0;
                                                        server_cnt ++;                                    if(server_cnt == servers_type_id.size())
                                                        if(server_cnt == server_num)
                                                        {
                                                            server_cnt = 0;
                                                            op.distribution_type = add;
                                                            break;
                                                        }
                                                }
                                                else
                                                {
                                                        left_CPU_B.at(server_cnt) = lc_B;
                                                        left_RAM_B.at(server_cnt) = lr_B;
                                                        op.distribution_type = norm;
                                                        op.server_id = server_cnt;
                                                        op.node_type = B;
                                                }
                                        }
                                        else
                                        {
                                                std::cerr<<"state_AB is :"<<state_AB<<std::endl;
                                        }
                                }
                        }
                }
                else
                {// 删除操作
                        op.distribution_type = norm;
                }
                if(op.distribution_type != norm)
                {
                    op.distribution_type = add;
                    for(const auto& s:m_servers)
                    {
                        if(m_VMs[t.second.second].m_is_double_node)
                        {
                            int lc = s.m_CPU_num - m_VMs[t.second.second].m_CPU_num;
                            int lr = s.m_RAM - m_VMs[t.second.second].m_RAM;
                            if(lc > 0 && lr > 0)
                            {
                                op.server_id = server_num;
                                op.server_type = s.m_type;
                                server_num ++;
                                servers_type_id.emplace_back(server_num);
                                left_CPU_A.emplace_back(lc/2);
                                left_RAM_A.emplace_back(lr/2);
                                left_CPU_B.emplace_back(lc/2);
                                left_RAM_B.emplace_back(lr/2);
                                break;
                            }
                        }
                        else
                        {
                            int lc = s.m_CPU_num/2 - m_VMs[t.second.second].m_CPU_num;
                            int lr = s.m_RAM/2 - m_VMs[t.second.second].m_RAM;
                            if(lc > 0 && lr > 0)
                            {
                                op.server_id = server_num;
                                op.server_type = s.m_type;
                                server_num ++;
                                servers_type_id.emplace_back(server_num);
                                left_CPU_A.emplace_back(lc);
                                left_RAM_A.emplace_back(lr);
                                left_CPU_B.emplace_back(m_VMs[t.second.second].m_CPU_num);
                                left_RAM_B.emplace_back(m_VMs[t.second.second].m_RAM);
                                break;
                            }
                        }
                    }
                }
                // 添加到结果中
                result.emplace_back(op);
        }
        return result;
}



// 按照服务器cpu+ram从小到大的顺序排列一张新的索引表
void distribution::sort_Server_Index()
{
    int num = m_servers.size();
    //初始化
    for(int i=0; i<num; i++){
        sorted_server_table.emplace_back(i);
    }
    sort(sorted_server_table.begin(), sorted_server_table.end(), [this](int a, int b){
        server_data server_a = m_servers[a];
        server_data server_b = m_servers[b];
        int capacity_a = server_a.m_CPU_num + server_a.m_RAM;
        int capacity_b = server_b.m_CPU_num + server_b.m_RAM;
        return capacity_a < capacity_b;
    });
}

std::vector<distribution_operation> distribution::try_distribution(
        std::vector<int>& servers_type_id,  //所有的服务器对应的型号
        std::vector<std::vector<int>>& VMs_type_id, //所有服务器上装的虚拟机
        task task_today,
        std::vector<int>& remain_CPU_A,
        std::vector<int>& remain_RAM_A,
        std::vector<int>& remain_CPU_B,
        std::vector<int>& remain_RAM_B)
{
    int next_server_id = servers_type_id.size();// 如果需要购买服务器，则是购买服务器的起始id，变
    int task_whole_num = task_today.cmd.size();
    int server_whole_num = servers_type_id.size();//不变
    distribution_result_queue.clear();//操作
    distribution_result_queue.resize(task_whole_num);// 初始化返回结果的数组

    /********* 1.根据权重睐进行对每日任务的排序，并依据此权重排序服务器 ************/
    // 将今日命令中del命令的vm型号都置为-1,表示不考虑买入，用户设置的id---虚拟机type
    // 根据del所在位置分割命令，进行分段排序，按照需求容量从小到大
    // 更改需求的定义方式，对cpu和ram增加权重因子再进行排序
    int need_cpu_num = 0, need_ram_num = 0;
    split_pos.clear();// 清空del命令的分割点，并进行初始化
    for(int i=0; i<task_today.cmd.size(); i++){
        if(task_today.cmd[i].first == "del"){
            task_today.cmd[i].second.second = -1;
            split_pos.emplace_back(i);
        }else{
            virtual_machine_data add_type = m_VMs[task_today.cmd[i].second.second];
            if(add_type.m_is_double_node){
                need_cpu_num += add_type.m_CPU_num;
                need_ram_num += add_type.m_RAM;
            }else{
                need_cpu_num += add_type.m_CPU_num * 2;
                need_ram_num += add_type.m_RAM * 2;
            }
        }
    }
    // 计算权重因子
    //weight_cpu = need_cpu_num / (need_cpu_num + need_ram_num);
    //weight_ram = 1 - weight_cpu;

    // sorted_vm_id[i]表示第i小需求对应的今日任务index
    std::vector<int> sorted_vm_id;
    for(int i = 0; i < task_whole_num; i++){
        sorted_vm_id.emplace_back(i);// 进行初始化
    }
    // 修改：按照降序排列
    if(split_pos.size() == 0){
        // 1.1 如果没有del命令，就对今天的所有命令进行排序
        std::sort(sorted_vm_id.begin(), sorted_vm_id.end(), [this, task_today]
            (int a, int b){
            virtual_machine_data data_a = m_VMs[task_today.cmd[a].second.second];
            virtual_machine_data data_b = m_VMs[task_today.cmd[b].second.second];
            int need_of_a = weight_cpu * data_a.m_CPU_num + weight_ram * data_a.m_RAM;
            int need_of_b = weight_cpu * data_b.m_CPU_num + weight_ram * data_b.m_RAM;
            return need_of_a > need_of_b;
        });
    }
    else{
        // 1.2 按照del命令分段对今天的命令进行排序
        int start_pose = 0;
        for(int i = 0; i < split_pos.size(); i++){
            int cur_interval = split_pos[i];// 指向del命令在当前命令列表中的下标
            std::sort(sorted_vm_id.begin()+start_pose, sorted_vm_id.begin()+cur_interval,
            [this, task_today](int a, int b){
                virtual_machine_data data_a = m_VMs[task_today.cmd[a].second.second];
                virtual_machine_data data_b = m_VMs[task_today.cmd[b].second.second];
                int need_of_a = weight_cpu * data_a.m_CPU_num + weight_ram * data_a.m_RAM;
                int need_of_b = weight_cpu * data_b.m_CPU_num + weight_ram * data_b.m_RAM;
                return need_of_a > need_of_b;
            });
            start_pose = cur_interval+1;
        }
        //排最后情况
        if(start_pose < sorted_vm_id.size()-1){
            std::sort(sorted_vm_id.begin()+start_pose, sorted_vm_id.end(), 
            [this, task_today](int a, int b){
                virtual_machine_data data_a = m_VMs[task_today.cmd[a].second.second];
                virtual_machine_data data_b = m_VMs[task_today.cmd[b].second.second];
                int need_of_a = weight_cpu * data_a.m_CPU_num + weight_ram * data_a.m_RAM;
                int need_of_b = weight_cpu * data_b.m_CPU_num + weight_ram * data_b.m_RAM;
                return need_of_a > need_of_b;
            });
        }
    }

     /********* 2 对现有的所有服务器按照剩余容量从小到大排序，只在算法开始时排序一次，进行一半之后再排序一次*********/
    //   ，并根据前面排序好的命令行顺序来进行逐一放置
    // sorted_server_id[i]表示第i小容量对应的服务器id
    std::vector<int> sorted_server_id;
    for(int i=0; i<server_whole_num; i++){
        sorted_server_id.emplace_back(i);// 进行初始化
        
    }
    int count = 0;
    std::vector<bool> is_assigned_correct(task_whole_num ,false);// 标志位判断是否放置成功
    int remains_task_num = 0;// 未完成的放置任务的数量
    std::vector<int> remains_task_index;// 未完成的放置任务在任务数组中的索引

    // 开始根据命令进行每条命令的处理
    while(count < task_whole_num){
        
        // 2.1 对现有的所有服务器按照剩余容量从小到大排序
        if(count == 0 || count == task_whole_num/2){
            std::sort(sorted_server_id.begin(), sorted_server_id.end(),
            [this, remain_CPU_A, remain_RAM_A, remain_CPU_B, remain_RAM_B](int a, int b){
                int sum_of_a = weight_cpu * (remain_CPU_A[a]+remain_CPU_B[a]) + weight_ram * (remain_RAM_A[a]+remain_RAM_B[a]);
                int sum_of_b = weight_cpu * (remain_CPU_A[b]+remain_CPU_B[b]) + weight_ram * (remain_RAM_A[b]+remain_RAM_B[b]);
                return  sum_of_a < sum_of_b;
            });
        }
        // 2.2 服务器容量和命令所需容量都是升序排列，逐vector begin 和end 次挨个放置尝试<first-fit算法>
        int cur_vmtypeid_intask = sorted_vm_id[count];// 取出今日任务中需要放置的第count小虚拟机，当前虚拟机在任务列表的索引
        // 修复bug
        int vm_typeid = task_today.cmd[cur_vmtypeid_intask].second.second;// 当前任务要处理的vm型号，-1表示是删除命令
        // 删除命令的处理
        if(vm_typeid == -1){
            is_assigned_correct[cur_vmtypeid_intask] = true;
            distribution_operation _operation;
            _operation.distribution_type = norm;
            _operation.node_type = -1;
            _operation.server_id = -1;
            _operation.server_type = -1;// 表示不添加任何服务器
            is_assigned_correct[cur_vmtypeid_intask] = true;
            distribution_result_queue[cur_vmtypeid_intask] = _operation;
            count++;
            continue;
        }
        // 添加命令的处理
        // 开始遍历服务器，判断是否可以放进去
        virtual_machine_data cur_vm = m_VMs[vm_typeid];
        for(int i = 0; i < server_whole_num; i++){
            int cur_j = sorted_server_id[i];// 按排序顺序取出剩余容量从小到大的服务器
            if(cur_vm.m_is_double_node){
                // 双节点虚拟机
                if( remain_CPU_A[cur_j] >= (cur_vm.m_CPU_num>>1) && remain_CPU_B[cur_j] >= (cur_vm.m_CPU_num>>1) 
                    && remain_RAM_A[cur_j] >= (cur_vm.m_RAM>>1) && remain_RAM_B[cur_j] >= (cur_vm.m_RAM>>1))
                {
                    distribution_operation _operation;
                    _operation.distribution_type = norm;
                    _operation.node_type = AB;
                    _operation.server_id = cur_j;
                    _operation.server_type = -1;// 表示不添加任何服务器
                    distribution_result_queue[cur_vmtypeid_intask] = _operation;
                    is_assigned_correct[cur_vmtypeid_intask] = true;
                    // 更新剩余容量
                    remain_CPU_A[cur_j] -= (cur_vm.m_CPU_num>>1);
                    remain_CPU_B[cur_j] -= (cur_vm.m_CPU_num>>1);
                    remain_RAM_A[cur_j] -= (cur_vm.m_RAM>>1);
                    remain_RAM_B[cur_j] -= (cur_vm.m_RAM>>1);
                    break;// 加入成功就退出对服务器的遍历
                }
            }
            else{
                // 单节点虚拟机
                if(remain_CPU_A[cur_j] >= cur_vm.m_CPU_num && remain_RAM_A[cur_j] >= cur_vm.m_RAM){
                    distribution_operation _operation;
                    _operation.distribution_type = norm;
                    _operation.node_type = A;
                    _operation.server_id = cur_j;
                    _operation.server_type = -1;// 表示不添加任何服务器
                    distribution_result_queue[cur_vmtypeid_intask] = _operation;
                    is_assigned_correct[cur_vmtypeid_intask] = true;
                    // 更新剩余容量
                    remain_CPU_A[cur_j] -= cur_vm.m_CPU_num;
                    remain_RAM_A[cur_j] -= cur_vm.m_RAM;
                    break;// 加入成功就退出对服务器的遍历
                }
                else if(remain_CPU_B[cur_j] >= cur_vm.m_CPU_num && remain_RAM_B[cur_j] >= cur_vm.m_RAM){
                    distribution_operation _operation;
                    _operation.distribution_type = norm;
                    _operation.node_type = B;
                    _operation.server_id = cur_j;
                    _operation.server_type = -1;// 表示不添加任何服务器
                    distribution_result_queue[cur_vmtypeid_intask] = _operation;
                    is_assigned_correct[cur_vmtypeid_intask] = true;
                    // 更新剩余容量
                    remain_CPU_B[cur_j] -= cur_vm.m_CPU_num;
                    remain_RAM_B[cur_j] -= cur_vm.m_RAM;
                    break;// 加入成功就退出对服务器的遍历
                }
            }    
        }
        if(is_assigned_correct[cur_vmtypeid_intask] == false){
            // 记录没有完成的任务的数量、任务位置、还需的cpu和ram总量
            remains_task_num++;
            remains_task_index.emplace_back(cur_vmtypeid_intask);
        }
        count++;
    }

#ifdef test
    /************ 3 如果存在没有完成的命令 ***********/
    if(remains_task_num){
        // 输出记录：是否有一次分配没有完成的任务
        std::cerr<<"handle remainning task!!!"<<endl;
    }
#endif
    // 对未完成的add命令继续进行处理
    // 映射server型号<---->server_ID
    std::vector<std::pair<int, int>> need_to_buy_server;
    int last_bought_serverNum = 0;// 记录重新购买过程中：过去购买的服务器总数 

    while(remains_task_num>0){
        // 3.1 根据未完成命令的虚拟机型号，进行个性化购买，生成购买的列表
        // 将购买任务拆解成分段任务，每次只满足一半任务的需求
        int this_task_interval = (remains_task_num>>1);// 每次只购买现存未完成任务一半的服务器数量
        this_task_interval = this_task_interval==0 ? 1 : this_task_interval;// 防止还剩一个任务没有完成
        int cur_bought_serverNum = 0;// 记录此次购买的服务器总数

        for(int i=0; i<remains_task_index.size(); i++){
            // 3.1.1 找到未完成任务对应的虚拟机类型，如果任务被处理（==-1）就跳过
            // 此处可以优化tip：先尝试完成一部分任务，如果现在买的服务器已经足够，就不再购买
            if(remains_task_index[i] == -1) continue;
            //优化
            if(this_task_interval == 0) break;
            this_task_interval--;
            // 目前对所有任务都分配各自所需要的合适的虚拟机
            int vm_type = task_today.cmd[remains_task_index[i]].second.second;
            int need_cpu_cur = 0, need_ram_cur = 0; 
            if(m_VMs[vm_type].m_is_double_node){
                need_cpu_cur = m_VMs[vm_type].m_CPU_num;
                need_ram_cur = m_VMs[vm_type].m_RAM;
            }else{
                need_cpu_cur = m_VMs[vm_type].m_CPU_num * 2;
                need_ram_cur = m_VMs[vm_type].m_RAM * 2;
            }
            // 3.1.2 根据得到的放置这个虚拟机需要的资源数量，来查找合适的服务器
            for(int i=0; i<sorted_server_table.size(); i++)
            {
                int server_type = sorted_server_table[i];
                if(m_servers[server_type].m_CPU_num >= need_cpu_cur
                    && m_servers[server_type].m_RAM >= need_ram_cur)
                {
                    need_to_buy_server.push_back({server_type, next_server_id});
                    next_server_id++;
                    cur_bought_serverNum++;
                    break;
                }
            }
        }
        // 3.2 根据购买要求实际开始购买服务器
        for(int i=last_bought_serverNum; i<need_to_buy_server.size(); i++)
        {
            distribution_operation _operation;
            _operation.distribution_type = add;
            _operation.server_id = need_to_buy_server[i].second;
            _operation.node_type = AB;
            _operation.server_type = need_to_buy_server[i].first;
            distribution_result_queue.push_back(_operation);
        } 

        // 3.3 放置：开始遍历服务器，判断是否可以放进去

        for(int i = last_bought_serverNum; i < need_to_buy_server.size(); i++){
            int server_type = need_to_buy_server[i].first;
            int CPU_A = (m_servers[server_type].m_CPU_num>>1);
            int CPU_B = (m_servers[server_type].m_CPU_num>>1);
            int RAM_A = (m_servers[server_type].m_RAM>>1);
            int RAM_B = (m_servers[server_type].m_RAM>>1);
            // 遍历未完成任务，尝试放入对应id的服务器
            for(int task_left = 0; task_left < remains_task_index.size(); task_left++){
                if(remains_task_index[task_left] == -1) continue;
                int task_id = remains_task_index[task_left];
                virtual_machine_data cur_vm = m_VMs[task_today.cmd[task_id].second.second];
                if(cur_vm.m_is_double_node){
                    // 双节点虚拟机
                    if(CPU_A >= (cur_vm.m_CPU_num>>1) && CPU_B >= (cur_vm.m_CPU_num>>1)
                        && RAM_A >= (cur_vm.m_RAM>>1) && RAM_B >= (cur_vm.m_RAM>>1))
                    {
                        distribution_operation _operation;
                        _operation.distribution_type = norm;
                        _operation.node_type = AB;
                        _operation.server_id = need_to_buy_server[i].second;
                        _operation.server_type = -1;// 表示不添加任何服务器
                        distribution_result_queue[task_id] = _operation;

                        is_assigned_correct[task_id] = true;
                        remains_task_index[task_left] = -1;
                        remains_task_num--;

                        CPU_A -= cur_vm.m_CPU_num/2;
                        CPU_B -= cur_vm.m_CPU_num/2;
                        RAM_A -= cur_vm.m_RAM/2;
                        RAM_B -= cur_vm.m_RAM/2;

                    }
                }
                else{
                    // 单节点虚拟机
                    if(CPU_A >= cur_vm.m_CPU_num && RAM_A >= cur_vm.m_RAM){
                        distribution_operation _operation;
                        _operation.distribution_type = norm;
                        _operation.node_type = A;
                        _operation.server_id = need_to_buy_server[i].second;
                        _operation.server_type = -1;// 表示不添加任何服务器
                        distribution_result_queue[task_id] = _operation;
                        
                        is_assigned_correct[task_id] = true;
                        remains_task_index[task_left] = -1;
                        remains_task_num--;

                        CPU_A -= cur_vm.m_CPU_num;
                        RAM_A -= cur_vm.m_RAM;

                    }
                    else if(CPU_B >= cur_vm.m_CPU_num && RAM_B >= cur_vm.m_RAM){
                        distribution_operation _operation;
                        _operation.distribution_type = norm;
                        _operation.node_type = B;
                        _operation.server_id = need_to_buy_server[i].second;
                        _operation.server_type = -1;// 表示不添加任何服务器
                        distribution_result_queue[task_id] = _operation;
                        
                        is_assigned_correct[task_id] = true;
                        remains_task_index[task_left] = -1;
                        remains_task_num--;

                        CPU_B -= cur_vm.m_CPU_num;
                        RAM_B -= cur_vm.m_RAM;
                    }
                }
            }
        }
        last_bought_serverNum += cur_bought_serverNum;// 结束这一轮放置之后，更新之前买的总服务器台数

    }

    // 4 整理返回数据信息，将数组中顺序按照购买->分配顺序返回
    std::vector<distribution_operation> result;
    int count_buy = 0;
    for(int i = distribution_result_queue.size()-1; i >=0; i--)
    {
        if(distribution_result_queue[i].distribution_type == add){
            result.emplace_back(distribution_result_queue[i]);
            count_buy++;
        }else{
            break;
        }
    }
    for(int i=0; i<distribution_result_queue.size()-count_buy; i++)
    {
        result.emplace_back(distribution_result_queue[i]);
    }
    return result;
}

