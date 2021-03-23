#include "migrate.hpp"
using namespace std;
#include <stdlib.h>

migrate::migrate(std::vector<server_data>& servers, std::vector<virtual_machine_data>& VMs)
{
    m_VMs = VMs;
    m_servers = servers;
}


migrate::~migrate()
{
}



vector<migrate_operation> migrate::try_migrate(
    std::unordered_map<int,virtual_machine> &m_deploy_VMs,
    std::unordered_map<int,int> &vmid_t_type,
    std::vector<int>& servers_type_id,
    std::vector<std::vector<int>>& VMs_type_id,
    const vector<bool>& service_is_new,
    vector<vector<bool>> vm_is_new,
    std::vector<int>& remain_CPU_A,
    std::vector<int>& remain_RAM_A,
    std::vector<int>& remain_CPU_B,
    std::vector<int>& remain_RAM_B)
{

#ifdef test
    //cerr<< "-----------------------------------------------a new day start----------------------------------------------------"<< endl;
#endif
    //统计信息
    int serverNum = servers_type_id.size();
    int old_vmNum = 0;
    int new_vmNum = 0;
    int sum_vmNum = 0;
    for (int i = 0; i < serverNum; ++i){
        for (int j = 0; j < VMs_type_id[i].size(); ++j){
            if (!vm_is_new[i][j]){
                ++old_vmNum;
            }else{
                ++new_vmNum;
            }
            ++sum_vmNum;
        }
    }
    //最大迁移次数
    int max_migrateCnt = old_vmNum*0.005;
    int changeCnt = 0;//调试用，用于记录可以交换的次数
#ifdef  test
    //cout<<"serverNum "<<serverNum<<" old_vmNum "<<old_vmNum<<endl;
#endif
    //当虚拟机数量大于200时才可以迁移
    vector<change_twoVm> record_jude;
    if (old_vmNum>200){
        //为记录排序之后的标号
        vector<int> record_server_pos(servers_type_id.size(), 0);
        for (int i = 0; i < servers_type_id.size(); ++i){
            record_server_pos[i] = i;
        }
        //统计每个服务器的利用率，当服务器cpu的利用率和ram的利用率一致时，计算出来的利用率是最高的,当为空时，利用效率较低
        vector<double> imbalance_rate(servers_type_id.size(), 0);
        for (int i = 0; i < servers_type_id.size(); ++i) {
            double cpuTotal = m_servers[servers_type_id[i]].m_CPU_num;
            double ramTotal = m_servers[servers_type_id[i]].m_RAM;
            double remain_cpu = remain_CPU_A[i] + remain_CPU_B[i];
            double remain_ram = remain_RAM_A[i] + remain_RAM_B[i];
            double molecular = 1.0 - abs(1.0 * remain_cpu / cpuTotal - 1.0 * remain_ram / ramTotal);
            double denominator = abs(remain_cpu / cpuTotal + remain_ram / ramTotal) + 1.0;
            double one_imbalance_rate = molecular / denominator;
            imbalance_rate[i] = one_imbalance_rate;
        }
        //按照服务器利用率从小到大进行排序，record_server_pos中的数值便是服务器原始的位置
        sort(record_server_pos.begin(), record_server_pos.end(), [imbalance_rate](int a, int b)
        {
            return imbalance_rate[a] < imbalance_rate[b];
        });
    #ifdef test
        //输出排序后的利用效率,denbug选项
        //for (auto c:record_server_pos) {
            //cerr << imbalance_rate[c] << " " << endl;
        //}
    #endif
        float max_imbalance_rate = *max_element(imbalance_rate.begin(),imbalance_rate.end());

        //临界利用效率，也就是利用率大于临界值的服务器暂时不动，去一半的服务器进行迁移
        float limit_used_rate = 0.4;
        //选择过程，将利用效率较低的服务器加入,存入的就是原始服务器的编号
        vector<int> sorted_elected_server;
        for (int i = 0; i < imbalance_rate.size(); ++i) {
            if (imbalance_rate[record_server_pos[i]] < limit_used_rate) {
                sorted_elected_server.push_back(record_server_pos[i]);
            }
        }
        int sort_elected_server_size =  sorted_elected_server.size();
        //cout<<"sort_elected_server_size "<<sort_elected_server_size<<endl;

        //确定选中的服务器中中利用率高的,1---ram利用高，2-----cpu利用高，3-----利用均衡
        vector<int> which_use_high(sorted_elected_server.size(), 3);
        for (int i = 0; i < sorted_elected_server.size(); ++i) {
            int server_inx = sorted_elected_server[i];//第几个服务器的编号，sorted_elected_server中存的就是给的服务器的编号
            int origin_cpu = m_servers[servers_type_id[server_inx]].m_CPU_num;
            int origin_ram = m_servers[servers_type_id[server_inx]].m_RAM;
            float cpu_use_rate = (float(remain_CPU_A[server_inx]) + float(remain_CPU_B[server_inx])) / float(origin_cpu);
            float ram_use_rate = (float(remain_RAM_A[server_inx]) + float(remain_RAM_B[server_inx])) / float(origin_ram);
            if (cpu_use_rate < ram_use_rate) {
                which_use_high[i] = 1; //ram占用高
            } else {
                which_use_high[i] = 2; //cpu占用高，存的是按照sorted_elected_server索引的type
            }
        }

        //sorted_selected_server表示的是我们需要采样的服务器，其中服务器按照高占用ram与高占用cpu来进行分配的,从ram占用高的服务器中选择一个需要ram最大的虚拟机
        vector<pair<int,int>> max_ram_vm_pos;//记录每一个高占用ram的服务器中，最大ram的虚拟机的位置
        vector<int> hight_cpu_used;//可以转移到的位置,记录可以迁移到到的高cpu占用的服务器的位置
        for(int i=0;i<sorted_elected_server.size();++i) {
            if(which_use_high[i]==1){
                int maxram = -1;
                int maxram_vmid=-1;
                for(int vm_id=0;vm_id<VMs_type_id[sorted_elected_server[i]].size();++vm_id){
                    int curren_ram = m_VMs[vmid_t_type[VMs_type_id[sorted_elected_server[i]][vm_id]]].m_RAM;
                    if(curren_ram>maxram){ //这儿的选择可能会导致选择了一个ram最大虚拟机，其他的都塞不进去
                        maxram_vmid = vm_id;
                    }
                }
                if(maxram_vmid!=-1) {
                    max_ram_vm_pos.push_back(make_pair(sorted_elected_server[i], maxram_vmid));
                }
            }else if(which_use_high[i]==2){
                hight_cpu_used.push_back(sorted_elected_server[i]);
            }else{
                //cerr<<"debug: cpuUsed == ramUssed"<<endl;
            }
        }

        for(int i=0;i<max_ram_vm_pos.size();++i){//尝试将前面排序ram占用高的虚拟机迁移到后面cpu占用高的服务器上，尝试虚拟机迁移到每一个cpu占用的服务器中
            change_twoVm temp_change_twoVm{};
            temp_change_twoVm.one_sever_id = max_ram_vm_pos[i].first;
            temp_change_twoVm.one_vm_id = max_ram_vm_pos[i].second;
            if(hight_cpu_used.size()) {
                for (int serverid = 0; serverid < hight_cpu_used.size(); ++serverid) {
                    temp_change_twoVm.another_server_id = hight_cpu_used[serverid];
                    temp_change_twoVm.another_vm_id = -1;
                    record_jude.push_back(temp_change_twoVm);
                }
            }
        }
    }else {
        record_jude = {};
    }

    //判断虚拟机是否迁移过了
    unordered_map<int,bool> isMigrate;
   for(int i=0;i<record_jude.size();++i){
       auto temp = record_jude[i];
       isMigrate[temp.one_sever_id] = false; //判断当前服务器上选择的虚拟机已经被迁移过了
   }

    //或许不存在可以迁移的服务器，塞的很满，迁移不动
    vector<migrate_operation>  res;
    for(int i=0;i<record_jude.size();++i) {
        change_twoVm temp = record_jude[i];
        int vm_server_id = temp.one_sever_id;
        int vm_vms_id = temp.one_vm_id;
        int togo_server_id = temp.another_server_id;
        if (changeCnt < 0) {
            if (!isMigrate[vm_server_id]) {
                char flag = canChange(m_deploy_VMs, vmid_t_type, servers_type_id, VMs_type_id, remain_CPU_A,
                                      remain_RAM_A,
                                      remain_CPU_B, remain_RAM_B, vm_server_id, vm_vms_id, togo_server_id);//判断能否
                //不能超过最大迁移次数
                migrate_operation temp_mg_op;
                //可以控制平衡次数
                if (changeCnt < 0) {
                    changeCnt++;//迁移次数++
                    if (flag == 'A') {//单节点，选择放在A节点上
                        temp_mg_op.is_new = false;
                        temp_mg_op.vm_id = VMs_type_id[vm_server_id][vm_vms_id];
                        temp_mg_op.to_server_id = togo_server_id;
                        temp_mg_op.node_type = 0;
                        isMigrate[vm_server_id] = true;
                        res.push_back(temp_mg_op);
                    } else if (flag == 'B') {//单节点，选择放在B节点上
                        temp_mg_op.is_new = false;
                        temp_mg_op.vm_id = VMs_type_id[vm_server_id][vm_vms_id];
                        temp_mg_op.to_server_id = togo_server_id;
                        temp_mg_op.node_type = 1;
                        isMigrate[vm_server_id] = true;
                        res.push_back(temp_mg_op);
                    } else if (flag == 'C') {//双节点服务器，AB节点同时放置
                        temp_mg_op.is_new = false;
                        temp_mg_op.vm_id = VMs_type_id[vm_server_id][vm_vms_id];
                        temp_mg_op.to_server_id = togo_server_id;
                        temp_mg_op.node_type = 2;
                        isMigrate[vm_server_id] = true;
                        res.push_back(temp_mg_op);
                    }
                } else {
                    break;
                }
            }
        }
    }
    //因为之前remain已经更新,需要更新文件
    vector<double> used_rate(servers_type_id.size(), 0);//统计每个服务器的利用率
    for (int i = 0; i < servers_type_id.size(); ++i) {
        double cpuTotal = m_servers[servers_type_id[i]].m_CPU_num;
        double ramTotal = m_servers[servers_type_id[i]].m_RAM;
        double remain_cpu = remain_CPU_A[i] + remain_CPU_B[i];
        double remain_ram = remain_RAM_A[i] + remain_RAM_B[i];
        double molecular = remain_cpu/cpuTotal+remain_ram/ramTotal;
        double denominator = 2;
        double one_used_rate = 1.0-molecular / denominator;
        used_rate[i] = one_used_rate;
    }
    vector<int> record_server_pos(servers_type_id.size(), 0);
    for (int i = 0; i < servers_type_id.size(); ++i){
        record_server_pos[i] = i;
    }
    sort(record_server_pos.begin(), record_server_pos.end(), [used_rate](int a, int b)
    {
        return used_rate[a] < used_rate[b];
    });

    if(max_migrateCnt>=1) {
        vector<int> wait_to_migrate;//利用率低的服务器
        vector<int> receive_vm;//选择接受的服务器
        for (int i = 0; i < used_rate.size(); ++i) {
            if (used_rate[record_server_pos[i]] < 0.7 && used_rate[record_server_pos[i]]>1e-20) {
                wait_to_migrate.push_back(record_server_pos[i]);
            } else if (used_rate[record_server_pos[i]] > 0.72 && used_rate[record_server_pos[i]] < 0.95) {
                receive_vm.push_back(record_server_pos[i]);
            }
        }
        //cout << "wait_to_migrate " << wait_to_migrate.size() << " wait_to_migrate " << receive_vm.size()<< " maxmigrate " << max_migrateCnt << endl;
        for (int wait_mig_index = 0; wait_mig_index < wait_to_migrate.size(); ++wait_mig_index) {//遍历所有需要移动的服务器
            int reach_maxMig_flag = 0;
            int finish_flag = 0;
            int server_id = wait_to_migrate[wait_mig_index];//需要考虑迁移的服务器的id
            int vmNuM = VMs_type_id[server_id].size();
            for (int vm_id = 0; vm_id < VMs_type_id[server_id].size(); vm_id++) {//遍历需要移动的服务器上的虚拟机
                //如果有哪个虚拟机器不能关掉，后面的虚拟机也无法关掉
                int needcpu = m_VMs[vmid_t_type[VMs_type_id[server_id][vm_id]]].m_CPU_num; //需要的cpu
                int needram = m_VMs[vmid_t_type[VMs_type_id[server_id][vm_id]]].m_RAM;     //需要的ram
                int half_need_cpu = needcpu >> 1;
                int half_need_ram = needram >> 1;
                int vm_type = m_deploy_VMs[VMs_type_id[server_id][vm_id]].get_type();
                int rec_inx = 0;
                for (rec_inx = 0; rec_inx < receive_vm.size(); ++rec_inx) { //遍历可供选择的服务器
                    int remain_cpu_a = remain_CPU_A[receive_vm[rec_inx]];
                    int remain_cpu_b = remain_CPU_B[receive_vm[rec_inx]];
                    int remain_ram_a = remain_RAM_A[receive_vm[rec_inx]];
                    int remain_ram_b = remain_RAM_B[receive_vm[rec_inx]];
                    //如果为单节点虚拟机
                    migrate_operation temp_mg_op;
                    if (vm_type == 0 || vm_type == 1) {
                        if ((needcpu < remain_cpu_a) && (needram < remain_ram_a)) {
                            //原来服务器减去
                            remain_CPU_A[server_id] += needcpu;
                            remain_RAM_A[server_id] += needram;
                            //目的地容量减少
                            remain_CPU_A[receive_vm[rec_inx]] -= needcpu;
                            remain_RAM_A[receive_vm[rec_inx]] -= needram;
                            //存入操作
                            temp_mg_op.is_new = false;
                            temp_mg_op.vm_id = VMs_type_id[server_id][vm_id];
                            temp_mg_op.to_server_id = receive_vm[rec_inx];
                            temp_mg_op.node_type = 0;
                            //isMigrate[vm_server_id] = true;
                            res.push_back(temp_mg_op);
                            //迁移次数+1
                            changeCnt++;
                            //cout << "changeCnt--A " << changeCnt << " rec_ser " << receive_vm[rec_inx] << " maxCnt "<< max_migrateCnt << endl;
                            if (changeCnt >= max_migrateCnt) {
                                reach_maxMig_flag = 1;
                                break;
                            }
                            break;
                        } else if ((needcpu < remain_cpu_b) && (needram < remain_ram_b)) {
                            //原来服务器减去
                            remain_CPU_B[server_id] += needcpu;
                            remain_RAM_B[server_id] += needram;
                            //目的地容量减少
                            remain_CPU_B[receive_vm[rec_inx]] -= needcpu;
                            remain_RAM_B[receive_vm[rec_inx]] -= needram;
                            //存入操作
                            temp_mg_op.is_new = false;
                            temp_mg_op.vm_id = VMs_type_id[server_id][vm_id];
                            temp_mg_op.to_server_id = receive_vm[rec_inx];
                            temp_mg_op.node_type = 1;
                            //isMigrate[vm_server_id] = true;
                            res.push_back(temp_mg_op);
                            changeCnt++;
                            //cout << "changeCnt--B " << changeCnt << " rec_ser " << receive_vm[rec_inx] << " maxCnt "<< max_migrateCnt << endl;
                            if (changeCnt >= max_migrateCnt) {
                                reach_maxMig_flag = 1;
                                break;
                            }
                            break;
                        }
                    } else if (vm_type == 2) { //如果为双节点虚拟机
                        if (half_need_cpu < remain_cpu_a && half_need_cpu < remain_cpu_b &&
                            half_need_ram < remain_ram_a && half_need_ram < remain_ram_b) {
                            //原来服务器减去
                            remain_CPU_A[server_id] += half_need_cpu;
                            remain_RAM_A[server_id] += half_need_ram;
                            remain_CPU_B[server_id] += half_need_cpu;
                            remain_RAM_B[server_id] += half_need_ram;
                            //目的地容量减少
                            remain_CPU_A[receive_vm[rec_inx]] -= half_need_cpu;
                            remain_RAM_A[receive_vm[rec_inx]] -= half_need_ram;
                            remain_CPU_B[receive_vm[rec_inx]] -= half_need_cpu;
                            remain_RAM_B[receive_vm[rec_inx]] -= half_need_ram;
                            //存入操作
                            temp_mg_op.is_new = false;
                            temp_mg_op.vm_id = VMs_type_id[server_id][vm_id];
                            temp_mg_op.to_server_id = receive_vm[rec_inx];
                            temp_mg_op.node_type = 2;
                            //isMigrate[vm_server_id] = true;
                            res.push_back(temp_mg_op);
                            //迁移次数+1
                            changeCnt++;
                            //cout << "changeCnt--C " << changeCnt << " rec_ser " << receive_vm[rec_inx] << " maxCnt "<< max_migrateCnt << endl;
                            if (changeCnt >= max_migrateCnt) {
                                reach_maxMig_flag = 1;
                                break;
                            }
                            break;
                        }
                    }
                }
                //当访问到最后一个服务器或者达到最大迁移次数限制
                if (rec_inx == receive_vm.size() - 1 || reach_maxMig_flag == 1) {
                    finish_flag = 1; //
                    break;
                }
            }
            if (finish_flag) {
                break;
            }
        }
    }
    return res;
}

//判断能否进行迁移
char migrate::canChange(
        std::unordered_map<int,virtual_machine> &m_deplyed_vm,
        std::unordered_map<int,int> &vmId_2_id,
        std::vector<int>& servers_type_id,
        std::vector<std::vector<int>>& VMs_type_id,
        std::vector<int>& remain_CPU_A,
        std::vector<int>& remain_RAM_A,
        std::vector<int>& remain_CPU_B,
        std::vector<int>& remain_RAM_B,
        int vm_server_id,
        int vm_vms_id,
        int togo_serverid){
    //需要迁移的虚拟机的一些信息
    int needcpu = m_VMs[vmId_2_id[VMs_type_id[vm_server_id][vm_vms_id]]].m_CPU_num;
    int needram = m_VMs[vmId_2_id[VMs_type_id[vm_server_id][vm_vms_id]]].m_RAM;
    int half_needcpu = needcpu/2;
    int half_needram = needram/2;
    int vm_type = m_deplyed_vm[VMs_type_id[vm_server_id][vm_vms_id]].get_type();
    //接收虚拟机服务器的一些信息

    int remaincpu_a = remain_CPU_A[togo_serverid];
    int remainram_a = remain_RAM_A[togo_serverid];
    int remaincpu_b = remain_CPU_B[togo_serverid];
    int remainram_b = remain_RAM_B[togo_serverid];
    if(vm_type==0 || vm_type==1 ){
        if((needcpu<remaincpu_a) &&(needram<remainram_a))  {
            //原来服务器减去
            remain_CPU_A[vm_server_id] += needcpu;
            remain_RAM_A[vm_server_id] += needram;
            //目的地容量减少
            remain_CPU_A[togo_serverid] -= needcpu;
            remain_RAM_A[togo_serverid] -= needram;
            return 'A';//放置在A节点
        }else if((needcpu<remaincpu_b) &&(needram<remainram_b)){
            //原来服务器减去
            remain_CPU_B[vm_server_id] += needcpu;
            remain_RAM_B[vm_server_id] += needram;
            //目的地容量减少
            remain_CPU_B[togo_serverid] -= needcpu;
            remain_RAM_B[togo_serverid] -= needram;
            return 'B';//放置在B节点
        }
    }else if(vm_type ==2){
        if(half_needcpu<remaincpu_a && half_needcpu<remaincpu_b && half_needram<remainram_a &&half_needram<remainram_b){
            //原来服务器减去
            remain_CPU_A[vm_server_id] += half_needcpu;
            remain_RAM_A[vm_server_id] += half_needram;
            remain_CPU_B[vm_server_id] += half_needcpu;
            remain_RAM_B[vm_server_id] += half_needram;
            //目的地容量减少
            remain_CPU_A[togo_serverid] -= half_needcpu;
            remain_RAM_A[togo_serverid] -= half_needram;
            remain_CPU_B[togo_serverid] -= half_needcpu;
            remain_RAM_B[togo_serverid] -= half_needram;
            return 'C';//放置双节点
        }
    } else{
        //cerr<<"wrong: wrong type"<<endl;
    }
    return 'D';//不能转移
}

