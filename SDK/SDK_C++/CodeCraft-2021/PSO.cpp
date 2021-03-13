#include "PSO.hpp"
#include <stdlib.h> 

PSO::PSO(const int dimension, const int num,int min, int max,manager* m):
m_dimension(dimension),m_num(num),m_min(min),m_max(max),m_manager_ptr(m)
{
    m_particles.resize(num);
    m_vec_max = (max - min)*0.1;
    random_init();
}

void PSO::random_init()
{
    for(auto pa:m_particles)
    {// 遍历所有的粒子
        for(int i = 0;i < m_dimension;i ++)
        {
            pa.pos.emplace_back((rand() % (m_max - m_min + 1)) + m_min);
            pa.vec.emplace_back((rand() % (m_vec_max *2 + 1)) - m_vec_max);
        } 
    }
}

void PSO::update_pos()
{
    for(auto pa:m_particles)
    {
        for(int i = 0;i < pa.pos.size();i ++)
        {
            // 更新速度
            pa.vec.at(i) = pa.vec.at(i) *  0.8 + 2 * rand() * (pa.best.at(i) - pa.pos.at(i)) + 2 * rand() * (m_best_pos.at(i) - pa.pos.at(i));
            // 更新位置
            pa.pos.at(i) += pa.vec.at(i);
            pa.pos.at(i) = pa.pos.at(i)>m_max?m_max:pa.vec.at(i);
            pa.pos.at(i) = pa.pos.at(i)<m_min?m_min:pa.vec.at(i);
        }
    }
}

void PSO::search_best()
{
    float best = 100000;
    for(auto pa:m_particles)
    {
        pa.score = cal_cost(pa.pos);
        if(best > pa.score)
        {// 全局最优
            best = pa.score;
            m_best_pos = pa.pos;
        }
        if(pa.best_score > pa.score)
        {// 当前粒子最优
            pa.best_score = pa.score;
            pa.best = pa.pos;
        }
    }
}
// 计算在当前配置下需要消耗的
 float PSO::cal_cost(std::vector<int> pos)
 {
    // std::vector<int>m_deploy_VMs cost_CPU;// 消耗的CPU数目
    // std::vector<int> cost_RAM;// 消耗的RAM数目
    // cost_CPU.resize(m_dimension);
    // cost_RAM.resize(m_dimension);
    // float cost = 0;
    // auto server_id = m_manager_ptr->get_server_id();
    // auto task = m_manager_ptr->get_current_tasks();//当天所有任务
    // for(int i = 0;i < m_num;i++)
    // {// 遍历所有粒子
    //     int server_type = pos.at(i);
    //     if(server_type == -1)
    //         continue;
    //     // cost_CPU.at(server_type) += task.cmd.at(i).second.first;
    //     // cost_RAM.at(server_type) += ;
    // }
    // // 减去已有的
    // for(int i = 0;i < m_dimension;i ++)
    // {
    //     cost_CPU.at(i) -= ;
    //     cost_RAM.at(i) -= ;
    // }
    // // 计算需要花费价格
    // for(int i = 0;i < m_dimension;i ++)
    // {
    //     int size = 0；
    //     cost += size * (m_manager_ptr->get_servers(i) ); 
    // }
 }