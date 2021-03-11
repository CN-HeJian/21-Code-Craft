/*思路来自论文
* <Glowworm Swarm Optimisation algorithm for virtual Machine Placement int Cloud Computing> 
* 
* 解决多目标优化问题，用于VMP
*
*/

#include <iostream>
#include <vector>
using namespace std;


class GSO
{
private:
    /* data */
public:
    GSO(/* args */);
    void initGSO();// 对参数进行初始化，并完成第一天的工作
    // 输入：服务器列表、当前需处理虚拟机
    // 输出：vm的分配情况
    void GSOVMP(vector<int>& serverList, int vm);
    
    ~GSO();
};

GSO::GSO(/* args */)
{


}

GSO::~GSO()
{

}

