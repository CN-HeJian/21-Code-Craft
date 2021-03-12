#ifndef __PSO_H
#define __PSO_H
#include <vector>
#include "manager.hpp"

struct particle
{
    std::vector<int> pos;// 粒子当前的位置
    std::vector<int> vec;// 粒子当前的速度
    std::vector<int> best;
    float best_score;
    float score;//粒子当前的得分
};

class PSO
{
public:
    PSO(const int dimension,const int num,int min,int max,manager *m);
    void random_init();// 随机初始化
    void update_pos();//更新当前所有粒子的位置
    void search_best();//搜索所有粒子中的最好的那个
private:
    std::vector<particle> m_particles;// 所有的粒子  
    int m_dimension;// 搜索的维度
    int m_num;// 粒子的数量
    int m_min;// 粒子赋值的最小值
    int m_max;// 粒子赋值的最大值
    int m_vec_max;// 粒子速度最大值
    float cal_cost(std::vector<int> pos);
    std::vector<int> m_best_pos;
    // 操作指针 
    manager * m_manager_ptr;
};


#endif // __PSO_H
