#include "iostream"
#include "manager.hpp"
#include "tools.hpp"

using namespace std;

#define test

// 最终的金额计算为 537898
int main()
{
	// TODO:read standard input

	//string inputTxtName = "/home/lyc/21-Code-Craft-lyc/training-data/training-2.txt";
    string inputTxtName = "/home/jian/Downloads/demo/21-Code-Craft/training-data/training-1.txt";

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
	
	// TODO:process
	m.processing();
	// TODO:write standard output
	m.cout_result();
	// TODO:fflush(stdout);
	std::cerr<<"cost time in ms:"<<clock_end()<<std::endl;
	return 0;
}