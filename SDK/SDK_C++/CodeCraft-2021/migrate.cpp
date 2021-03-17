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

#define SAMPLENUM  1000
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
        for(int j = i;j<recordSelectedPos.size();++j){remain_CPU_B
            //判断能否进行迁移,是否有必要迁移以及能否迁移
            bool flag = canMigrate(servers_type_id,VMs_type_id,remain_CPU_A,remain_RAM_A,\
            remain_CPU_B,remain_RAM_B,recordSelectedPos,i,j);
            if(flag ) {
                //temp_change_twoVm.jude_point = judge();
                temp_change_twoVm.one_sever_id = recordSelectedPos[i].first;
                temp_change_twoVm.one_vm_id = recordSelectedPos[i].second;
                temp_change_twoVm.another_server_id = recordSelectedPos[j].first;
                temp_change_twoVm.another_vm_id = recordSelectedPos[j].second;
                recode_jude.emplace_back(temp_change_twoVm);
            }
        }
    }

    //对尝试交换的虚拟机进行排序，选择性价比最高的几个操作,从大到小操作
    sort(recode_jude.begin(),recode_jude.end(),[](change_twoVm a,change_twoVm b ){
        return a.jude_point>b.jude_point;
    });
    //确定最大的迁移次数
    int maxMigrateCnt = int(old_vmNum*0.005);
    //迁移操作集合
    vector<migrate_operation> res;
    //当前已经操作的次数
    int current_Ope_Cnt = 0;
    //依次排入
    while(current_Ope_Cnt<maxMigrateCnt){

        current_Ope_Cnt++;
    }

    return {};
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
    //当不再同一个服务器中时，直接进去判断资源是否够用问题，否则还要判断是否是否放置在同一个节点
    if(oneVm_serverId != anotherVm_serverId){
        //进入资源是否够用的环节
        //优先排进资源平衡的服务器
        int needCpu = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_CPU_num;
        int needRam = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_RAM;
        int half_need_cpu = needCpu>>1;
        int half_need_ram = needRam>>1;
        //剩余的资源
        int remainCPU_A = remain_CPU_A[anotherVm_serverId];
        int remainRAM_A = remain_RAM_A[anotherVm_serverId];
        int remainCPU_B = remain_CPU_B[anotherVm_serverId];
        int remainRAM_B = remain_RAM_B[anotherVm_serverId];
        //如果是单节点的
        if((remainCPU_A>needCpu&&remainRAM_A>needRam) || (remainCPU_B>needCpu&& remainRAM_B>needRam)){
            return true;
        }else{
            if((remainCPU_A>half_need_cpu&&remainRAM_A>half_need_ram) && (remainCPU_B>half_need_cpu&& remainRAM_B>half_need_ram)){
                return true;
            }
        }
        return false;
    }else{
        int oneWhichNode = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].node_type;
        int anotherWhichNode = m_VMs[VMs_type_id[anotherVm_serverId][anotherVm_VmsId]].node_type;
        if(oneWhichNode == anotherWhichNode) {
            return false;
        }
        else{
            //优先排进资源平衡的服务器
            int needCpu = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_CPU_num;
            int needRam = m_VMs[VMs_type_id[oneVm_serverId][oneVm_VmsId]].m_RAM;
            int half_need_cpu = needCpu>>1;
            int half_need_ram = needRam>>1;
            //剩余的资源
            int remainCPU_A = remain_CPU_A[anotherVm_serverId];
            int remainRAM_A = remain_RAM_A[anotherVm_serverId];
            int remainCPU_B = remain_CPU_B[anotherVm_serverId];
            int remainRAM_B = remain_RAM_B[anotherVm_serverId];
            //如果是单节点的
            if((remainCPU_A>needCpu&&remainRAM_A>needRam) || (remainCPU_B>needCpu&& remainRAM_B>needRam)){
                return true;
            }else{
                if((remainCPU_A>half_need_cpu&&remainRAM_A>half_need_ram) && (remainCPU_B>half_need_cpu&& remainRAM_B>half_need_ram)){
                    return true;
                }
            }
            return false;
        }
    }
    cout<<"can not reach"<<endl;
    return false;
}


//评价当前策略的好坏
float migrate::judge_operate(std::vector<int>& servers_type_id,
                             std::vector<std::vector<int>>& VMs_type_id,
                             std::vector<int>& remain_CPU_A,
                             std::vector<int>& remain_RAM_A,
                             std::vector<int>& remain_CPU_B,
                             std::vector<int>& remain_RAM_B,
                             vector<pair<int,int>> &recordSelectedPos,
                             int &oneVM,
                             int &anotherVM){

    return 0.0;
}