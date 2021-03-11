#include "GSO.hpp"
GSO::GSO(const string& inputFile)
{
    mgt.readTxtbyStream(inputFile);
}

void GSO::initGSO()
{
    // 一天一天处理，没有智能优化的过程
    int server_buy_id = -1;
    for(int d=0; d<mgt.get_days(); d++)
    {
        if(mgt.get_severNums()==0)
        {
        // 第一天还没有服务器时候
            task t = mgt.get_tasks(d);
            // 遍历这一天的所有任务，求总的需要多少cpu和内存
            int cpuNum = 0, memoryNum = 0;
            for(int j=0; j<t.cmd.size(); j++)
            {
                int type, id;
                if(t.cmd[j].first == "add"){
                    type = t.cmd[j].second.second;
                    if(mgt.m_VMs[type].m_is_double_node){
                        cpuNum += mgt.m_VMs[type].m_CPU_num;
                        memoryNum += mgt.m_VMs[type].m_RAM;
                    }else{
                        // 单节点的话为防止资源不足，当作双倍消耗计算
                        cpuNum += 2*mgt.m_VMs[type].m_CPU_num;
                        memoryNum += 2*mgt.m_VMs[type].m_RAM;
                    }
                }
            }
            // 然后根据这一天的cpu和内存，计算合适的server型号和数量
            int index_max = 0; 
            int index_fit, server_num_fit;
            int ph_index = 0;
            for(; ph_index<mgt.m_servers.size(); ph_index++)
            {
                // 遍历服务器表，查找一个能够装下一整天消耗的服务器，
                // 如果没找到就求需要买多少台最大的服务器
                if(mgt.m_servers[ph_index].m_CPU_num >= cpuNum &&
                    mgt.m_servers[ph_index].m_RAM >= memoryNum){
                    index_fit = ph_index;
                    server_num_fit = 1;
                    break;
                }
                if(mgt.m_servers[index_max].m_CPU_num >= mgt.m_servers[ph_index].m_CPU_num
                    && mgt.m_servers[index_max].m_RAM >= mgt.m_servers[ph_index].m_RAM){
                    index_max = ph_index;   
                }
            }
            if(ph_index == mgt.m_servers.size()){
                index_fit = index_max;
                server_num_fit = max(cpuNum / mgt.m_servers[index_fit].m_CPU_num,
                                memoryNum / mgt.m_servers[index_fit].m_RAM) + 1;
            } 

            // index_fit就是这一天适合采购的服务器在表单中的索引
            for(int i=0; i<server_num_fit; i++){
                mgt.purchase_server(++server_buy_id, index_fit);
            }
            // 购买服务器完毕
            // 接下来再遍历
            for(int j=0; j<t.cmd.size(); j++)
            {
                //将这一天的命令进行解析，装入或者删除
            }
            continue;
        }
        // 2—n天已经存在服务器群，先判断能否装一部分vm，如果不可以就采购
        mgt.deploy_VM();//完成部分放置虚拟机任务
        // 然后遍历这一天的剩余任务，求总的需要多少cpu和内存
        // 继续求解新一天需要购买的server型号和数目
        // 解析命令：添加或删除
    }
}

void GSO::GSOVMP(std::vector<int>& serverList, int vm)
{
    /*
    g = newGlowworm(serverlist, vm);//
    PHid = luciferin.getPh();//获取一个PH
    currentluc = g.calculateLuc(PHid);// 计算荧光素
    initialise iteration count t = 0;
    while termination_condition not met do:
        for j = 1 to M do:
            newluc =  g.calculateNewLuc(PHid);// 公式7
            neighbours = g.getNeighbors(PHid, neighbourSize);// 公式8
            while j < neighbours.size() do
                neighbourId = neighbours.get(j)
                neighbourluc=g.calculateLuc(neighbourId)
                if neighbourluc = 0 then
                    currentluc = neighbourluc
                    neighbourPhId = neighbours.get(j)
                    j= neighbours.size()
                end if
                luc = currentluc−neighbourluc
                if luc>luciferin then
                    if neighbourPhId<γjd then
                    luciferin = luc
                    currentluc = neighbourluc
                    neighbourPhId = neighbours.get(j)
                    end if
                end if
            end while
            allocatedPh = PhList.get(neighbourPhId)
            update radial range by Eq.(11)
        end for
        t=t+1
    end while
    return allocatedPh
     
    */
}