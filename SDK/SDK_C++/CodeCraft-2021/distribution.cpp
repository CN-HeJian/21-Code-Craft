#include "distribution.hpp"


distribution::distribution(std::vector<server_data>& servers, std::vector<virtual_machine_data>& VMs)
{
    m_VMs = VMs;
    m_servers = servers;

    max_server_typeid = get_maxServer_index();// 获取服务器表中容量最大服务器的typeid
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
        for(const auto& t:tasks.cmd)
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
                                        else {// 添加到B节点
                                            lc_B = left_CPU_B.at(server_cnt) - m_VMs[t.second.second].m_CPU_num;
                                            lr_B = left_RAM_B.at(server_cnt) - m_VMs[t.second.second].m_RAM;
                                            lc_A = 10;
                                            lr_A = 10;
                                            if (lc_B < 0 || lr_B < 0) {
                                                state_AB = 0;
                                                server_cnt++;
                                                if (server_cnt == servers_type_id.size())
                                                    if (server_cnt == server_num) {
                                                        server_cnt = 0;
                                                        op.distribution_type = add;
                                                        break;
                                                    }
                                            } else {
                                                left_CPU_B.at(server_cnt) = lc_B;
                                                left_RAM_B.at(server_cnt) = lr_B;
                                                op.distribution_type = norm;
                                                op.server_id = server_cnt;
                                                op.node_type = B;
                                            }
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


// 获取容量最大f服务器的索引
int distribution::get_maxServer_index()
{
    int max_typeid = 0;
    for(int i=0; i<m_servers.size(); i++){
        if(m_servers[max_typeid].m_CPU_num <= m_servers[i].m_CPU_num
            && m_servers[max_typeid].m_RAM <= m_servers[i].m_RAM){
                max_typeid = i;
            }
    }
    return max_typeid;
}

std::vector<distribution_operation> distribution::try_distribution(
        std::vector<int>& servers_type_id,  //所有的服务器对应的型号
        std::vector<std::vector<int>>& VMs_type_id, //所有服务器上装的虚拟机
        task& task_today,
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

    std::vector<int> split_pos;//存放del命令在今日命令中的索引
    // 将今日命令中del命令的vm型号都置为-1,表示不考虑买入，用户设置的id---虚拟机type
    // 根据del所在位置分割命令，进行分段排序，按照需求容量从小到大
    for(int i=0; i<task_today.cmd.size(); i++){
        if(task_today.cmd[i].first == "del"){
            task_today.cmd[i].second.second = -1;
            split_pos.emplace_back(i);
        }
    }

    // sorted_vm_id[i]表示第i小需求对应的今日任务index
    std::vector<int> sorted_vm_id;
    for(int i = 0; i < task_whole_num; i++){
        sorted_vm_id.emplace_back(i);// 进行初始化
    }
    if(split_pos.size() == 0){
        // 1.1 如果没有del命令，就对今天的所有命令进行排序
        std::sort(sorted_vm_id.begin(), sorted_vm_id.end(), [this, task_today](int a, int b){
            virtual_machine_data data_a = m_VMs[task_today.cmd[a].second.second];
            virtual_machine_data data_b = m_VMs[task_today.cmd[b].second.second];
            int need_of_a = data_a.m_CPU_num + data_a.m_RAM;
            int need_of_b = data_b.m_CPU_num + data_b.m_RAM;
            return need_of_a < need_of_b;
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
                int need_of_a = data_a.m_CPU_num + data_a.m_RAM;
                int need_of_b = data_b.m_CPU_num + data_b.m_RAM;
                return need_of_a < need_of_b;
            });
            start_pose = cur_interval+1;
        }
        //排最后情况
        if(start_pose < sorted_vm_id.size()-1){
            std::sort(sorted_vm_id.begin()+start_pose, sorted_vm_id.end(), 
            [this, task_today](int a, int b){
                virtual_machine_data data_a = m_VMs[task_today.cmd[a].second.second];
                virtual_machine_data data_b = m_VMs[task_today.cmd[b].second.second];
                int need_of_a = data_a.m_CPU_num + data_a.m_RAM;
                int need_of_b = data_b.m_CPU_num + data_b.m_RAM;
                return need_of_a < need_of_b;
            });
        }
    }

    // 2 对现有的所有服务器按照剩余容量从小到大排序，只在算法开始时排序一次，进行一半之后再排序一次
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
    int needCpuNum=0, needRamNum=0;// 还缺的cpu、ram资源总和
    // 开始根据命令进行每条命令的处理
    while(count < task_whole_num){

        // 2.1 对现有的所有服务器按照剩余容量从小到大排序
        if(count == 0 || count == task_whole_num/2){
            std::sort(sorted_server_id.begin(), sorted_server_id.end(),[
                remain_CPU_A, remain_RAM_A, remain_CPU_B, remain_RAM_B
            ](int a, int b){
                int sum_of_a = remain_CPU_A[a]+remain_RAM_A[a]+remain_CPU_B[a]+remain_RAM_B[a];
                int sum_of_b = remain_CPU_A[b]+remain_RAM_A[b]+remain_CPU_B[b]+remain_RAM_B[b];
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
            if(m_VMs[vm_typeid].m_is_double_node){
                needCpuNum += m_VMs[vm_typeid].m_CPU_num;
                needRamNum += m_VMs[vm_typeid].m_RAM;
            }
            else{
                needCpuNum += m_VMs[vm_typeid].m_CPU_num * 2;
                needRamNum += m_VMs[vm_typeid].m_RAM * 2;
            }
        }
        count++;
    }

    // 3 如果存在没有完成的命令
    if(remains_task_num){
        // 输出记录：是否有一次分配没有完成的任务
        std::cerr<<"handle remainning task!!!"<<endl;
    }
    while(remains_task_num){
        // 然后根据还需要的cpu和内存，计算合适的server型号和数量
        int index_fit, server_num_fit;
        int ph_index = 0;
        // 遍历服务器表，查找一个能够装下一整天消耗的服务器，如果没找到就求需要买多少台最大的服务器
        for(; ph_index<m_servers.size(); ph_index++)
        {
            if(m_servers[ph_index].m_CPU_num >= needCpuNum &&
                m_servers[ph_index].m_RAM >= needRamNum){
                index_fit = ph_index;//适合的服务器的索引
                server_num_fit = 1;
                break;
            }
        }
        if(ph_index == m_servers.size()){
            index_fit = max_server_typeid; //
            server_num_fit = max(needCpuNum / m_servers[index_fit].m_CPU_num,
                            needRamNum / m_servers[index_fit].m_RAM) + 1;
        } 
        // index_fit就是这一天适合采购的服务器在表单中的索引
        // 开始购买服务器，并将购买信息放置在数组最后，最后返回时需要将数组中购买信息放置到数组开始位置
        for(int i=0; i<server_num_fit; i++){
            distribution_operation _operation;
            _operation.distribution_type = add;
            _operation.server_id = next_server_id;
            _operation.node_type = AB;
            _operation.server_type = index_fit;
            distribution_result_queue.push_back(_operation);
            next_server_id++;
        }
        // 继续开始放置
        // 开始遍历服务器，判断是否可以放进去
        for(int i = next_server_id - server_num_fit; i<next_server_id; i++){
            int CPU_A = m_servers[index_fit].m_CPU_num/2;
            int CPU_B = m_servers[index_fit].m_CPU_num/2;
            int RAM_A = m_servers[index_fit].m_RAM/2;
            int RAM_B = m_servers[index_fit].m_RAM/2;
            // 处理每一个还未处理的任务
            for(int j=0; j<remains_task_index.size(); j++){
                int task_id = remains_task_index[j];// 取出未完成放置的任务
                if(task_id == -1) continue;
                int vm_typeid = task_today.cmd[task_id].second.second;
                virtual_machine_data cur_vm = m_VMs[vm_typeid];
                if(cur_vm.m_is_double_node){
                    // 双节点虚拟机
                    if(CPU_A >= (cur_vm.m_CPU_num>>1) && CPU_B >= (cur_vm.m_CPU_num>>1)
                        && RAM_A >= (cur_vm.m_RAM>>1) && RAM_B >= (cur_vm.m_RAM>>1))
                    {
                        distribution_operation _operation;
                        _operation.distribution_type = norm;
                        _operation.node_type = AB;
                        _operation.server_id = i;
                        _operation.server_type = -1;// 表示不添加任何服务器
                        distribution_result_queue[task_id] = _operation;
                        is_assigned_correct[task_id] = true;
                        remains_task_index[j] = -1;
                        remains_task_num--;

                        CPU_A -= cur_vm.m_CPU_num/2;
                        CPU_B -= cur_vm.m_CPU_num/2;
                        RAM_A -= cur_vm.m_RAM/2;
                        RAM_B -= cur_vm.m_RAM/2;

                        needCpuNum -= cur_vm.m_CPU_num;
                        needRamNum -= cur_vm.m_RAM;
                    }
                }
                else{
                    // 单节点虚拟机
                    if(CPU_A >= cur_vm.m_CPU_num && RAM_A >= cur_vm.m_RAM){
                        distribution_operation _operation;
                        _operation.distribution_type = norm;
                        _operation.node_type = A;
                        _operation.server_id = i;
                        _operation.server_type = -1;// 表示不添加任何服务器
                        distribution_result_queue[task_id] = _operation;
                        is_assigned_correct[task_id] = true;

                        remains_task_index[j] = -1;
                        remains_task_num--;

                        CPU_A -= cur_vm.m_CPU_num;
                        RAM_A -= cur_vm.m_RAM;

                        needCpuNum -= cur_vm.m_CPU_num;
                        needRamNum -= cur_vm.m_RAM;

                    }
                    else if(CPU_B >= cur_vm.m_CPU_num && RAM_B >= cur_vm.m_RAM){
                        distribution_operation _operation;
                        _operation.distribution_type = norm;
                        _operation.node_type = B;
                        _operation.server_id = i;
                        _operation.server_type = -1;// 表示不添加任何服务器
                        distribution_result_queue[task_id] = _operation;
                        is_assigned_correct[task_id] = true;

                        remains_task_index[j] = -1;
                        remains_task_num--;

                        CPU_B -= cur_vm.m_CPU_num;
                        RAM_B -= cur_vm.m_RAM;

                        needCpuNum -= cur_vm.m_CPU_num;
                        needRamNum -= cur_vm.m_RAM;
                    }
                }
            }
        }
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

