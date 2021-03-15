#include "distribution.hpp"

distribution::distribution(std::vector<server_data> servers, std::vector<virtual_machine_data> VMs) 
{
         m_VMs = VMs;
        m_servers = servers;
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
 * @return 需要进行的操作
 *                    填充数据  std::vector<distribution_operation>  
 *                    按照如下的顺序进行
 *                    1、添加服务器命令
 *                    2、正常任务，一定要按照task的顺序来，不然就错了
 *                    3、删除服务器命令
 *          
 * */
std::vector<distribution_operation> distribution::try_distribution(
        std::vector<int> servers_type_id,
        std::vector<std::vector<int>> VMs_type_id,
        task tasks,
        std::vector<int> left_CPU_A,
        std::vector<int> left_CPU_B,
        std::vector<int> left_RAM_A,
        std::vector<int> left_RAM_B)
{
        int server_cnt = 0;
        int state_AB = 0;// A = 0 , B = 1


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
                                while(lc_A < 0 || lc_B < 0 || lr_A < 0 || lr_B < 0)
                                {
                                        server_cnt ++;
                                        lc_A = left_CPU_A.at(server_cnt) - m_VMs[t.second.second].m_CPU_num/2;
                                        lc_B = left_CPU_B.at(server_cnt) - m_VMs[t.second.second].m_CPU_num/2;
                                        lr_A = left_RAM_A.at(server_cnt) - m_VMs[t.second.second].m_RAM/2;
                                        lr_B = left_RAM_B.at(server_cnt) - m_VMs[t.second.second].m_RAM/2;
                                }
                                left_CPU_A.at(server_cnt) = lc_A;
                                left_CPU_B.at(server_cnt) = lc_B;
                                left_RAM_A.at(server_cnt) = lr_A;
                                left_RAM_B.at(server_cnt) = lr_B;
                                op.distribution_type = norm;
                                op.server_id = server_cnt;
                                op.node_type = AB;
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
                                                        server_cnt ++;
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
                // 添加到结果中
                result.emplace_back(op);
        }
        return result;
}