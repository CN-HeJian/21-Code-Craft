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
    //max_server_typeid = get_maxServer_index();// 获取服务器表中容量最大服务器的typeid
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
//1000个虚拟机只可以迁移5次
#define SAMPLENUM  1000
//评分函数的侧重点不一致
#define EXTREMUM 0.4
#define AVERGAE 0.2
#define VARIANCE 0.4

//1000个虚拟机只可以迁移5次

int migrate::rand_one(int min, int max){
    srand((unsigned int)time(NULL));
    return rand()%(max - min + 1)+min;
}

vector<migrate_operation> migrate::try_migrate(
    std::vector<int>& servers_type_id,
    std::vector<std::vector<int>>& VMs_type_id,
    const vector<int>& service_is_new,
    vector<vector<int>> vm_is_new,
    std::vector<int>& remain_CPU_A,
    std::vector<int>& remain_RAM_A,
    std::vector<int>& remain_CPU_B,
    std::vector<int>& remain_RAM_B)
{
    //统计信息
    int serverNum = servers_type_id.size();//服务器数目
    int old_vmNum = 0;//旧的虚拟机数目
    vector<pair<int,int>>  recordOriginPos;//记录展开后的一维虚拟机的原始位置
    for(int i=0;i<serverNum;+i){
        for(int j=0;j<VMs_type_id[i].size();++j){
            recordOriginPos.emplace_back(make_pair(i,j));
            if(!vm_is_new[i][j])
                ++old_vmNum;
        }
    }

    //确定采样的虚拟机的个数
    int sampleNum = 0;
    if(old_vmNum<SAMPLENUM)
        sampleNum = old_vmNum;
    else
        sampleNum = SAMPLENUM;

    //随机选择虚拟机的过程
    vector<pair<int,int>> recordSelectedPos;
    //如果虚拟机数量小于采样数目，所有的用来寻优,否则部分寻优
    if(sampleNum == SAMPLENUM) {
        vector<bool> is_selected(old_vmNum,false); //用来记录虚拟机是否被选中了
        int selected_nums = 0;//当前选择了多少虚拟机
        while(selected_nums<sampleNum){
            bool isRepeat = false;
            while(!isRepeat){
                int val = rand_one(0, old_vmNum);//随机选择的是虚拟机的id
                if(!is_selected[val]) {
                    selected_nums++;
                    isRepeat = true;
                    int whichServe = recordOriginPos[val].first;
                    int whichVm = recordOriginPos[val].second;
                    recordSelectedPos.emplace_back(whichServe,whichVm);
                    is_selected[val] = true;
                }
            }
        }
    }else{
        for(int i=0 ;i<serverNum;++i){
            for(int j =0;j<VMs_type_id[i].size();++i){
                recordSelectedPos.emplace_back(make_pair(i,j));
            }
        }
    }

    //逐次比较
    change_twoVm temp_change_twoVm{};
    vector<change_twoVm> recode_jude;
    for(int i=0;i<recordSelectedPos.size();++i){
        for(int j = i;j<recordSelectedPos.size();++j){
            //判断能否进行迁移,是否有必要迁移以及能否迁移
            bool flag = canMigrate(servers_type_id,VMs_type_id,remain_CPU_A,remain_RAM_A,\
            remain_CPU_B,remain_RAM_B,recordSelectedPos,i,j);
            if(flag ) {
                //评判过程
                temp_change_twoVm.jude_point = judge_operate(servers_type_id,VMs_type_id,remain_CPU_A,remain_RAM_A,\
                remain_CPU_B,remain_RAM_B,recordSelectedPos,i,j);

                temp_change_twoVm.one_sever_id = recordSelectedPos[i].first;
                temp_change_twoVm.one_vm_id = recordSelectedPos[i].second;
                temp_change_twoVm.another_server_id = recordSelectedPos[j].first;
                temp_change_twoVm.another_vm_id = recordSelectedPos[j].second;
                recode_jude.emplace_back(temp_change_twoVm);
            }
        }
    }

    //对尝试交换的虚拟机进行排序，选择性价比最高的几个操作,从大到小操作
    vector<int> record_sort_index;
    for(int i=0;i<recode_jude.size();++i){
        record_sort_index[i] = i;
    }
    sort(record_sort_index.begin(),record_sort_index.end(),[recode_jude](int a,int b ){
        return recode_jude[a].jude_point  <recode_jude[b].jude_point;
    });
    //确定最大的迁移次数
    int maxMigrateCnt = int(old_vmNum*0.005);
    //迁移操作集合
    vector<migrate_operation> res;
    //临时操作
    migrate_operation temp_migrate_operator;
    //开始选择
    for(int i=0;i<maxMigrateCnt;++i){
        if(i>maxMigrateCnt)
            cout<<"out of maxMigrate time"<<endl;
        //排序之后的id
        int index = record_sort_index[i];
        //找到对应的哪台VM
        temp_migrate_operator.is_new = false;
        temp_migrate_operator.vm_id = VMs_type_id[recode_jude[index].one_sever_id][recode_jude[index].one_vm_id];
        temp_migrate_operator.to_server_id = servers_type_id[recode_jude[index].another_server_id];
        int oneWhichNode = m_VMs[VMs_type_id[recode_jude[index].one_sever_id][recode_jude[index].one_vm_id]].node_type;
        if(oneWhichNode==2)
            temp_migrate_operator.node_type = 2;
        int need_cpu = m_VMs[VMs_type_id[recode_jude[index].one_sever_id][recode_jude[index].one_vm_id]].m_CPU_num;
        int need_ram = m_VMs[VMs_type_id[recode_jude[index].one_sever_id][recode_jude[index].one_vm_id]].m_RAM;
        if(remain_CPU_A[recode_jude[index].another_server_id]>need_cpu && remain_RAM_A[recode_jude[index].another_server_id]>need_ram){
            temp_migrate_operator.node_type = 1; //B节点
        }else if(remain_CPU_B[recode_jude[index].another_server_id]>need_cpu && remain_RAM_B[recode_jude[index].another_server_id]>need_ram){
            temp_migrate_operator.node_type = 0; //A节点
        }else{
            cout<<"***************************judge comes wrong********************"<<endl;
        }
        res.push_back(temp_migrate_operator);
    }
    return res;
}

//如果进行了迁移操作，计算操作后的得分
float migrate::judge_operate(std::vector<int>& servers_type_id,
                             std::vector<std::vector<int>>& VMs_type_id,
                             std::vector<int>& remain_CPU_A,
                             std::vector<int>& remain_RAM_A,
                             std::vector<int>& remain_CPU_B,
                             std::vector<int>& remain_RAM_B,
                             vector<pair<int,int>> &recordSelectedPos,
                             int &oneVM,
                             int &anotherVM){
    float judge_point = 0.f;
    //计算当前虚拟机在哪个服务器中，
    int oneVm_serverId = recordSelectedPos[oneVM].first;
    //计算服务器的当前配置
    int origin_cpu_1 = m_servers[servers_type_id[oneVm_serverId]].m_CPU_num;
    int origin_ram_1 = m_servers[servers_type_id[oneVm_serverId]].m_RAM;
    //计算占用比例
    float orgin_cpu_used_1 = 1.f - (remain_CPU_A[oneVm_serverId]+remain_CPU_B[oneVm_serverId])/origin_cpu_1;
    float origin_ram_used_1 = 1.f - (remain_RAM_A[oneVm_serverId]+remain_RAM_B[oneVm_serverId])/origin_ram_1;
    //计算另外一个
    int anotherVm_serverId = recordSelectedPos[anotherVM].first;
    //计算服务器的当前配置
    int origin_cpu_2 = m_servers[servers_type_id[anotherVm_serverId]].m_CPU_num;
    int origin_ram_2 = m_servers[servers_type_id[anotherVm_serverId]].m_RAM;
    //计算占用比例
    float orgin_cpu_used_2 = 1.f - (remain_CPU_A[anotherVm_serverId]+remain_CPU_B[anotherVm_serverId])/origin_cpu_2;
    float origin_ram_used_2 = 1.f - (remain_RAM_A[anotherVm_serverId]+remain_RAM_B[anotherVm_serverId])/origin_ram_2;
    //统计一下结果
    float origin_extremum = max(orgin_cpu_used_1,origin_ram_used_1)+max(orgin_cpu_used_2,origin_ram_used_2);
    cout<<"origin_extremum"<<origin_extremum<<endl;
    float cpu_average = (orgin_cpu_used_1+orgin_cpu_used_2)/2;
    float ram_average = (origin_ram_used_1+origin_ram_used_2)/2;
    float origin_average = (cpu_average+ram_average)/2;
    cout<<"origin_average"<<origin_average<<endl;

    //转移之后的情况
    int oneVm_VmsId = recordSelectedPos[oneVM].second;
    int changeCpu = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_CPU_num;
    int changeRam = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_RAM;
    float change_cpu_1 =  1.f - (remain_CPU_A[oneVm_serverId]+remain_CPU_B[oneVm_serverId]+changeCpu)/origin_cpu_1;
    if(change_cpu_1<0.00001)
        cout<<"---------------------------------------turn offf------------------------------"<<endl;
    float change_ram_1 =  1.f - (remain_CPU_A[oneVm_serverId]+remain_CPU_B[oneVm_serverId]+changeRam)/origin_ram_1;
    float change_cpu_2 =  1.f - (remain_CPU_A[anotherVm_serverId]+remain_CPU_B[anotherVm_serverId]-changeCpu)/origin_cpu_2;
    float change_ram_2 =  1.f - (remain_CPU_A[anotherVm_serverId]+remain_CPU_B[anotherVm_serverId]-changeRam)/origin_ram_2;
    //统计一下结果
    float change_extremum = max(orgin_cpu_used_1,origin_ram_used_1)+max(orgin_cpu_used_2,origin_ram_used_2);
    cout<<"total_extremum"<<change_extremum<<endl;
    float change_cpu_average = (change_cpu_1+change_cpu_2)/2;
    float change_ram_average = (change_ram_1+change_ram_2)/2;
    float change_average = (change_cpu_average+change_ram_average)/2;
    cout<<"change_average"<<change_average<<endl;

    judge_point = EXTREMUM*(change_extremum-origin_extremum)+AVERGAE*(change_average-origin_average);
    return judge_point;
}

//判断能否进行迁移
bool migrate::canMigrate(std::vector<int>& servers_type_id,
                std::vector<std::vector<int>>& VMs_type_id,
                std::vector<int>& remain_CPU_A,
                std::vector<int>& remain_RAM_A,
                std::vector<int>& remain_CPU_B,
                std::vector<int>& remain_RAM_B,
                vector<pair<int,int>> &recordSelectedPos,
                int &oneVM,
                int &anotherVM){
    //异常报错，选择了同一个虚拟机
    if(oneVM == anotherVM){
        cerr<<"you have select the same VM"<<endl;
    }
    //获取当前虚拟机在哪个服务器中
    int oneVm_serverId = recordSelectedPos[oneVM].first;
    int anotherVm_serverId = recordSelectedPos[anotherVM].first;
    //判断是服务器集群中的第几号虚拟机
    int oneVm_VmsId = recordSelectedPos[oneVM].second;
    int anotherVm_VmsId = recordSelectedPos[anotherVM].second;
    //对应的虚拟机器的型号
    int needCpu = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_CPU_num;
    int needRam = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_RAM;
    int half_need_cpu = needCpu>>1;
    int half_need_ram = needRam>>1;
    //剩余的资源
    int remainCPU_A = remain_CPU_A[anotherVm_serverId];
    int remainRAM_A = remain_RAM_A[anotherVm_serverId];
    int remainCPU_B = remain_CPU_B[anotherVm_serverId];
    int remainRAM_B = remain_RAM_B[anotherVm_serverId];
    //在哪个节点
    int oneWhichNode = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].node_type;
    int anotherWhichNode = m_VMs[VMs_type_id[anotherVm_serverId][anotherVm_VmsId]].node_type;
    //判断是否在同一个服务器的同一个节点中
    if((oneVm_serverId != anotherVm_serverId) || oneWhichNode != anotherWhichNode) {
        //单节点服务器，随意排放即可
        if(oneWhichNode == 0 || oneWhichNode == 1) {
            if ((remainCPU_A > needCpu && remainRAM_A > needRam) || (remainCPU_B > needCpu && remainRAM_B > needRam))
                return true;
        }
        else if ((remainCPU_A > half_need_cpu && remainRAM_A > half_need_ram) &&(remainCPU_B > half_need_cpu && remainRAM_B > half_need_ram)) {
            return true;
        }
    }
    return false;
}