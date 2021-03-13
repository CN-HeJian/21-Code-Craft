#include "migrate.hpp"
using namespace std;
#include "string.h"



migrate::migrate(/* args */)
{
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
    vector<std::pair<int,server_data>> &servers,
    vector<vector<pair<int,virtual_machine_data>>> &VMs,
    vector<int> &remain_CPU_A,
    vector<int> &remain_RAM_A,
    vector<int> &remain_CPU_B,
    vector<int> &remain_RAM_B) {

    //此处不判断是否合法，传进内容保证是合法的

    /* 复位
     **/
    VmNums = 0 ;
    max_migrateCnt = 0;
    std::memset(&servers_isOld,0,sizeof servers_isOld);
    std::memset(&vm_isOld,0,sizeof vm_isOld);
    std::memset(&servers_isEmpty,0,sizeof servers_isEmpty);
    std::memset(&A_CPU_used_rate,0,sizeof A_CPU_used_rate);
    std::memset(&A_RAM_used_rate,0,sizeof A_RAM_used_rate);
    std::memset(&B_CPU_used_rate,0,sizeof B_CPU_used_rate);
    std::memset(&B_RAM_used_rate,0,sizeof B_RAM_used_rate);
    std::memset(&is_highUsed,0,sizeof is_highUsed);

    //设置大小
    int s_size = servers.size();
    servers_isOld.resize(s_size);
    vm_isOld.resize(s_size);
    servers_isEmpty.resize(s_size);
    A_CPU_used_rate.resize(s_size);
    A_RAM_used_rate.resize(s_size);
    B_CPU_used_rate.resize(s_size);
    B_RAM_used_rate.resize(s_size);
    is_highUsed.resize(s_size);

    /*如果是old，则返回true
    * servers_isOld[i]
    * vm_isOld[i][j]
    */
    for(int i =0;i<s_size;++i) {
        servers_isOld[i] = servers.at(i).second.is_old;
        int oneServerVMNUms = VMs.at(i).size();
        int server_half_cpu = servers.at(i).second.m_CPU_num>>1;
        int server_half_ram = servers.at(i).second.m_RAM>>1;
        A_CPU_used_rate[i] = 1.f - remain_CPU_A[i]/server_half_cpu;
        A_RAM_used_rate[i] = 1.f - remain_RAM_A[i]/server_half_ram;
        B_CPU_used_rate[i] = 1.f - remain_CPU_B[i]/server_half_cpu;
        B_RAM_used_rate[i] = 1.f - remain_RAM_B[i]/server_half_ram;
        if (!oneServerVMNUms) //判断服务器是否被使用
            servers_isEmpty[i] = true;
        vm_isOld[i].resize(oneServerVMNUms);
        for (int j = 0; j < oneServerVMNUms; ++j) {
            vm_isOld[i][j] = VMs.at(i).at(j).second.is_old;
            VmNums++;
        }
    }

    //判断每一台服务器是否高负载,此处定义了参数 Defined_highUsed
    for(int i=0;i<s_size;++i){
        if(A_CPU_used_rate[i]> Defined_highUsed || A_RAM_used_rate[i]> Defined_highUsed || B_CPU_used_rate[i]>Defined_highUsed || B_RAM_used_rate[i]> Defined_highUsed ){
            is_highUsed[i] = true;
        }
    }

    //二次规划问题
    int cost = 0;
    /* 最大迁移次数
     * */
    max_migrateCnt = VmNums*0.005;
    std::cerr<<"VmNums "<<VmNums<<"max_migrateCnt "<<max_migrateCnt<<endl;

    //记忆化

    //设置最大迁移次数

    //定义可行域区间

    //随机初始化粒子,需要定义粒子数量，判断收敛条件

    //定义代价函数

    //迭代终止条件

    return {};
}





