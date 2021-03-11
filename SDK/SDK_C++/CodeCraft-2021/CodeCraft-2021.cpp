#include "iostream"
#include "manager.hpp"
#include "toos.hpp"
using namespace std;

#define test

// 最终的金额计算为 537898
int main()
{
	// TODO:read standard input
	string inputTxtName = "/home/ljh/huawei2021/21-Code-Craft/training-data/test.txt";


	//readTxt(inputTxtName);
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	clock_start();
	// TODO:read standard input
	manager m;
	#ifdef test
		m.readTxt(inputTxtName);
	#else
		m.readTxtbyStream(inputTxtName);
	#endif

	//m.output();
	std::cerr<<"cost time in ms:"<<clock_end()<<std::endl;
	// TODO:process
	int server_id = -1;
	for(int d=0;d<m.get_days();d++)
	{
		// 第d天购买服务 0
		m.purchase_server(++server_id,0);
		// 迁移测试
		if(d == 2){
			// 第三天的时候
			m.purchase_server(++server_id,1);// 多买一台
			// 把第二天的迁移到这一台上
			auto s = m.get_server(1);
			for(int i=0;i<s.get_VM_num();i++){
				m.migrate_VM(s.get_VM_ids().at(i),server_id-1,AB);
			}
		}
		auto t = m.get_tasks(d);
		// 两条部署上去 
		for(int i=0;i<t.cmd.size();i++)
		{
			auto c = t.cmd.at(i);// 第i条指令
			if(c.first == "add"){
				m.deploy_VM(c.second.first,c.second.second,server_id,AB);
			}
			else{
				m.de_deploy_VM(c.second.first);
			}
		}
		// 计算当天的电费
		m.cal_cost();
		m.finish_oneday();// 一天结束的标志
	}
	// TODO:write standard output
	m.cout_result();
	// TODO:fflush(stdout);

	return 0;
}
