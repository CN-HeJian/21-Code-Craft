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

    //复位
    VmNums = 0 ;
    max_migrateCnt = 0;
    std::memset(&servers_isOld,0,sizeof servers_isOld);     //服务器是否为旧的
    std::memset(&vm_isOld,0,sizeof vm_isOld);               //虚拟机是否为旧的
    std::memset(&servers_isEmpty,0,sizeof servers_isEmpty);
    std::memset(&A_CPU_used_rate,0,sizeof A_CPU_used_rate);
    std::memset(&A_RAM_used_rate,0,sizeof A_RAM_used_rate);
    std::memset(&B_CPU_used_rate,0,sizeof B_CPU_used_rate);
    std::memset(&B_RAM_used_rate,0,sizeof B_RAM_used_rate);
    std::memset(&is_highUsed,0,sizeof is_highUsed);
    std::memset(&cpu_busy,0,sizeof cpu_busy);
    std::memset(&ram_busy,0,sizeof ram_busy);
    std::memset(&is_VMs_double,0,sizeof is_VMs_double);
    std::memset(&is_VMs_newAndDouble,0,sizeof is_VMs_newAndDouble);
    std::memset(&VMs__serverNode,0,sizeof VMs__serverNode);
    std::memset(&server_id,0,sizeof server_id);
    std::memset(&vM_id,0,sizeof vM_id);

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
    cpu_busy.resize(s_size);
    ram_busy.resize(s_size);
    server_id.resize(s_size);

    //servers_isOld[i]  vm_isOld[i][j]
    for(int i =0;i<s_size;++i) {
        server_id[i] = servers.at(i).first;
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
            vm_isOld[i][j] = VMs.at(i).at(j).second.is_old; //是否虚拟机是否为新的
            is_VMs_double.push_back(VMs.at(i).at(j).second.m_is_double_node);//判断虚拟机是否为双节点的，可以重写用空间换时间
            is_VMs_newAndDouble.push_back((VMs.at(i).at(j).second.m_is_double_node&&(~vm_isOld[i][j]))); //是否为新的并且是双节点的
            VMs__serverNode.push_back(VMs.at(i).at(j).second.node_type); //获取虚拟机当前挂在哪个服务器的哪个节点上
            vM_id.push_back(VMs.at(i).at(j).first);
            VmNums++;
        }
    }
    
    //假设同一台服务器的各种资源是均衡分配的，判断每一台服务器是否高负载,此处定义了参数 Defined_highUsed
    for(int i=0;i<s_size;++i){
        if(A_CPU_used_rate[i]> Defined_highUsed || A_RAM_used_rate[i]> Defined_highUsed || \
        B_CPU_used_rate[i]>Defined_highUsed || B_RAM_used_rate[i]> Defined_highUsed ){
            is_highUsed[i] = true;
        }
    }

    //操作,分成多种情况来判断，如果同一服务器各个资源分配不均匀
    cpu_busy[0].A = false;
    cpu_busy[0].B = true;

    // 最大迁移次数,旧的节点只可以移动千分之五次，新的节点任意移动
    max_migrateCnt = VmNums*0.005;
    std::cerr<<"VmNums "<<VmNums<<"max_migrateCnt "<<max_migrateCnt<<endl;

    //二次规划问题

    //图论问题
    //首先过滤掉使用效率较高的节点，或者加上随机采样优化
    //保证同一节点的各种资源的利用率均衡，Trick
    //稀疏图---Trick
    //设置优先级，旧的虚拟机有迁移次数限制，新的虚拟机器没有迁移次数限制，尽可能移动较多的次数，减少成本，若成本相同，按照迁移次数来评判
    //前一天的迁移结果较优--前提
    //查询某台虚拟机是否为双节点，有什么用呢？确定代价函数
    //尽量移动到旧的虚拟机上面去，空出新的虚拟机，最关键的目标,优先迁移到旧的虚拟机上面去
    //将新增的双节点的虚拟机先删除掉，暂时不考虑，减少先前占用的迁移次数，先对单节点进行资源分配更有利于资源负载均衡
    //获取单节点虚拟机设置在服务器哪个节点上


    //智能算法
    //记忆化
    //设置最大迁移次数
    //定义可行域区间
    //随机初始化粒子,需要定义粒子数量，判断收敛条件
    //定义代价函数
    //迭代终止条件


    //组合数问题

    return {};
}





