#include "iostream"
#include "manager.hpp"
//#include "tools.hpp"

using namespace std;

// 最终的金额计算为 537898
int main()
{
	// TODO:read standard input

	//string inputTxtName = "/home/lyc/21-Code-Craft/training-data/training-1.txt";
  //string inputTxtName = "/home/jian/Downloads/demo/21-Code-Craft/training-data/training-1.txt";
	string inputTxtName = "/home/ljh/huawei2021/21-Code-Craft/training-data/training-2.txt";

	clock_start();
	// TODO:read standard input
	manager m;
	#ifdef test
		m.readTxt(inputTxtName);
	#else
		m.readTxtbyStream();
	#endif
	// TODO:process
	m.processing();
	// TODO:write standard output
  //m.result();
	// TODO:fflush(stdout);
	fflush(stdout);
	std::cerr<<"cost time:"<<clock_end()<<std::endl;
	return 0;
}


