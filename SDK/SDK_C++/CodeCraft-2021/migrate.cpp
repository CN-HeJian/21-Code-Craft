#include "migrate.hpp"
using namespace std;
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



migrate::migrate(std::vector<server_data>& servers, std::vector<virtual_machine_data>& VMs)
{
    m_VMs = VMs;
    m_servers = servers;
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
 * @Step
 *      1）statistic  VmNums
 *      2）judge service_isOld Vm_is_Old
 *      3) define Max_migrate_Cnt
 * */

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
    cerr<< "-----------------------------------------------a new day start----------------------------------------------------"<< endl;
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
#ifdefine test
    cout<<"serverNum "<<serverNum<<" old_vmNum "<<old_vmNum<<endl;
#endif
    //当虚拟机数量大于200时才可以迁移
    if (old_vmNum>200){
        //为记录排序之后的标号
        vector<int> record_server_pos(servers_type_id.size(), 0);
        for (int i = 0; i < servers_type_id.size(); ++i){
            record_server_pos[i] = i;
        }
        //统计每个服务器的利用率，当服务器cpu的利用率和ram的利用率一致时，计算出来的利用率是最高的
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
    #ifdefine test
        //输出排序后的利用效率,denbug选项
        for (auto c:record_server_pos) {
            cerr << imbalance_rate[c] << " " << endl;
        }
    #endif
        //临界利用效率，也就是利用率大于临界值的服务器暂时不动，去一半的服务器进行迁移
        float limit_used_rate = imbalance_rate[imbalance_rate.size() - 1] >> 1;
        //选择过程，将利用效率较低的服务器加入
        vector<int> sorted_elected_server
        for (int i = 0; i < imbalance_rate.size(); ++i) {
            if (imbalance[record_server_pos[i]] < limit_used_rate) {
                sorted_elected_server.push_back(record_server_pos[i]);
            }
        }
        //确定选中的服务器中中利用率高的,1---ram利用高，2-----cpu利用高，3-----利用均衡
        vetor<int> which_use_high(sorted_elected_server.size(), 3);
        for (int i = 0; i < sorted_elected_server.size(); ++i) {
            int server_inx = sorted_elected_server[i];//第几个服务器的编号，sorted_elected_server中存的就是给的服务器的编号
            int origin_cpu = m_servers[servers_type_id[server_inx]].m_CPU_num;
            int origin_ram = m_servers[servers_type_id[server_inx]].m_RAM;
            float cpu_use_rate = (float(remain_CPU_A[server_inx]) + float(remain_CPU_B[server_inx])) / origin_cpu;
            float ram_use_rate = (float(remain_RAM_A[server_inx]) + float(remain_RAM_B[server_inx])) / origin_ram;
            if (cpu_use_rate < ram_use_rate) {
                which_use_high[server_inx] = 1; //ram占用高
            } else {
                which_use_high[server_inx] = 2; //cpu占用高
            }else{
                //如果进入这儿，考虑是不是计算出错
                cout << "sever used balance" << endl;//which_use_high
            }
        }
        //Whichwhich_use_high就是排序之后的每个服务器所有的属性,然后进一步排序,sorted_elected_server前面的是ram占用高的server，后面是cpu占用高的server
        sort(sorted_elected_server.begin(), sorted_elected_server.end(), [which_use_high](int a, int b) {
            return which_use_high[a] < which_use_high[b];
        });
        //sorted_selected_server表示的是我们需要采样的服务器，其中服务器按照高占用ram与高占用cpu来进行分配的,从ram占用高的服务器中选择一个需要ram最大的虚拟机
        vector<pair<int,int>> max_ram_vm_pos;//记录每一个高占用ram的服务器中，最大ram的虚拟机的位置
        vector<int> hight_cpu_used;//可以转移到的位置,记录可以迁移到到的高cpu占用的服务器的位置
        for(int i=0;i<sorted_elected_server.size();++i) {
            if(whichType[sorted_elected_server[i]]==1){
                int maxram = -1;
                int maxram_vmid=-1;
                for(int vm_id=0;vm_id<VMs_type_id[i].size();++vm_id){
                    int curren_ram = m_VMs[vmId_2_id[VMs_type_id[i][vm_id]]].m_RAM;
                    if(curren_ram>maxram){ //这儿的选择可能会导致选择了一个ram最大虚拟机，其他的都塞不进去
                        maxram_vmid = vm_id;
                    }
                }
                max_ram_vm_pos.push_back(make_pair(i,maxram_vmid));
            }else if(whichType[sorted_elected_server[i]]==2){
                hight_cpu_used.push_back(sorted_elected_server[i]);
            }else{
                cerr<<"debug: cpuUsed == ramUssed"<<endl;
            }
        }
        vector<change_twoVm> record_jude;
        for(int i=0;i<max_ram_vm_pos.size();++i){//尝试将前面排序ram占用高的虚拟机迁移到后面cpu占用高的服务器上，尝试虚拟机迁移到每一个cpu占用的服务器中
            change_twoVm temp_change_twoVm{};
            temp_change_twoVm.one_sever_id = max_ram_vm_pos[i].first;
            temp_change_twoVm.one_vm_id = max_ram_vm_pos[i].second;
            for(int serverid=0;i<hight_cpu_used.size();++serverid){
                temp_change_twoVm.another_server_id = hight_cpu_used[serverid];
                record_jude.push_back(temp_change_twoVm);
            }
        }
    }else {
        record_jude= {};
    }
    //或许不存在可以迁移的服务器，塞的很满，迁移不动
    int changeCnt = 0;//调试用，用于记录可以交换的次数
    int max_migrateCnt = old_vmNum*0.005;//最大迁移次数
    vector<migrate_operation>  res;
    for(int i=0;i<record_jude.size();++i){
        change_twoVm temp = record_jude[i];
        int vm_server_id = temp.one_sever_id;
        int vm_vms_id = temp.one_vm_id;
        int togo_server_id = temp.another_server_id;
        char flag = canMigrate(m_deploy_VMs,vmid_t_type, servers_type_id, VMs_type_id, remain_CPU_A, remain_RAM_A, remain_CPU_B, remain_RAM_B, vm_server_id, vm_vms_id,togo_server_id);//判断能否
        if(flag != 'D'){
            //不能超过最大迁移次数
            migrate_operation temp_mg_op;
            if(max_migrateCnt>changeCnt) {
                changeCnt++;//迁移次数++
                if(flag=='A'){//单节点，选择放在A节点上
                    temp_mg_op.is_new = false;
                    temp_mg_op.vm_id = vmId_2_id[VMs_type_id[vm_server_id][vm_vms_id]];
                    temp_mg_op.to_server_idv = servers_type_id[togo_server_id];
                    temp_mg_op.node_type = 0;
                }else if(flag=='B'){//单节点，选择放在B节点上
                    temp_mg_op.is_new = false;
                    temp_mg_op.vm_id = vmId_2_id[VMs_type_id[vm_server_id][vm_vms_id]];
                    temp_mg_op.to_server_idv = servers_type_id[togo_server_id];
                    temp_mg_op.node_type = 0;
                }else if(flag=='C'){//双节点服务器，AB节点同时放置
                    temp_mg_op.is_new = false;
                    temp_mg_op.vm_id = vmId_2_id[VMs_type_id[vm_server_id][vm_vms_id]];
                    temp_mg_op.to_server_idv = servers_type_id[togo_server_id];
                    temp_mg_op.node_type = 0;
                }
            }else{
                break; //达到最大迁移次数，跳出循环
            }
        }else{
            cout<<"debug: "<<"not enough cpu or ram"<<endl;//如果出现的次数很多，则满足迁移的虚拟机数量太少
        }
    }
    #ifdef test
        cout<<"debug: canchangeCnt " <<canchangeCnt<<endl;
    #endif
    return res;
}

/*
//如果进行了迁移操作，计算操作后的得分
float migrate::judge_operate( std::unordered_map<int,virtual_machine> &m_deplyed_vm,
std::unordered_map<int,int> &vmId_2_id,
                             std::vector<int>& servers_type_id,
                             std::vector<std::vector<int>>& VMs_type_id,
                             std::vector<int>& remain_CPU_A,
                             std::vector<int>& remain_RAM_A,
                             std::vector<int>& remain_CPU_B,
                             std::vector<int>& remain_RAM_B,
                             vector<pair<int,int>> &recordSelectedPos,
                             int &oneVM,
                             int &anotherVM){
    //应该是用来判断利用效率的
    float judge_point = 0.f;
    //计算当前虚拟机在哪个服务器中，
    int oneVm_serverId = recordSelectedPos[oneVM].first;
    //计算服务器的当前配置
    int origin_cpu_1 = m_servers[servers_type_id[oneVm_serverId]].m_CPU_num;
    int origin_ram_1 = m_servers[servers_type_id[oneVm_serverId]].m_RAM;
    //计算占用比例
    float orgin_cpu_used_1 = 1.f - double(remain_CPU_A[oneVm_serverId]+remain_CPU_B[oneVm_serverId])/float(origin_cpu_1);
    float origin_ram_used_1 = 1.f - float(remain_RAM_A[oneVm_serverId]+remain_RAM_B[oneVm_serverId])/float(origin_ram_1);
    //计算另外一个
    int anotherVm_serverId = recordSelectedPos[anotherVM].first;
    //计算服务器的当前配置
    int origin_cpu_2 = m_servers[servers_type_id[anotherVm_serverId]].m_CPU_num;
    int origin_ram_2 = m_servers[servers_type_id[anotherVm_serverId]].m_RAM;
    //计算占用比例
    float orgin_cpu_used_2 = 1.f - float(remain_CPU_A[anotherVm_serverId]+remain_CPU_B[anotherVm_serverId])/float(origin_cpu_2);
    float origin_ram_used_2 = 1.f - float(remain_RAM_A[anotherVm_serverId]+remain_RAM_B[anotherVm_serverId])/float(origin_ram_2);
    //统计一下结果
    float origin_extremum = max(orgin_cpu_used_1,origin_ram_used_1)+max(orgin_cpu_used_2,origin_ram_used_2);
    float cpu_average = (orgin_cpu_used_1+orgin_cpu_used_2)/2;
    float ram_average = (origin_ram_used_1+origin_ram_used_2)/2;
    float origin_average = (cpu_average+ram_average)/2;

    //转移之后的情况
    int oneVm_VmsId = recordSelectedPos[oneVM].second;
    int changeCpu = m_VMs[vmId_2_id[VMs_type_id[oneVm_serverId][oneVm_VmsId]]].m_CPU_num;
    int changeRam = m_VMs[vmId_2_id[VMs_type_id[oneVm_serverId][oneVm_VmsId]]].m_RAM;
    float change_cpu_1 =  1.f - float(remain_CPU_A[oneVm_serverId]+remain_CPU_B[oneVm_serverId]+changeCpu)/float(origin_cpu_1);
    float change_ram_1 =  1.f - float(remain_CPU_A[oneVm_serverId]+remain_CPU_B[oneVm_serverId]+changeRam)/float(origin_ram_1);
    float change_cpu_2 =  1.f - float(remain_CPU_A[anotherVm_serverId]+remain_CPU_B[anotherVm_serverId]-changeCpu)/float(origin_cpu_2);
    float change_ram_2 =  1.f - float(remain_CPU_A[anotherVm_serverId]+remain_CPU_B[anotherVm_serverId]-changeRam)/float(origin_ram_2);
    //统计一下结果
    float change_extremum = max(orgin_cpu_used_1,origin_ram_used_1)+max(orgin_cpu_used_2,origin_ram_used_2);
    float change_cpu_average = (change_cpu_1+change_cpu_2)/2;
    float change_ram_average = (change_ram_1+change_ram_2)/2;
    float change_average = (change_cpu_average+change_ram_average)/2;

    judge_point = EXTREMUM*(change_extremum-origin_extremum)+AVERGAE*(change_average-origin_average);
    return judge_point;
}
*/

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
    int half_needcpu = needcpu>>1;
    int half_needram = needram>>1;
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
            return A;//放置在A节点
        }else if((needcpu<remaincpu_b) &&(needram<remainram_b)){
            //原来服务器减去
            remain_CPU_B[vm_server_id] += needcpu;
            remain_RAM_B[vm_server_id] += needram;
            //目的地容量减少
            remain_CPU_B[togo_serverid] -= needcpu;
            remain_RAM_B[togo_serverid] -= needram;
            return B;//放置在B节点
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
            return C;//放置双节点
        }
    } else{
        cerr<<"wrong: wrong type"<<endl;
    }
    return D;//不能转移
}
