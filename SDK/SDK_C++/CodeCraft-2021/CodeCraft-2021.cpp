#include "iostream"
#include "manager.hpp"
//#include "tools.hpp"

using namespace std;


// 最终的金额计算为 537898
int main()
{
	// TODO:read standard input

	string inputTxtName = "/home/lyc/21-Code-Craft/training-data/training-1.txt";
    //string inputTxtName = "/home/jian/Downloads/demo/21-Code-Craft/training-data/test.txt";

	//readTxt(inputTxtName);
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	//clock_start();
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
#ifndef test
    m.result();
#endif
	// TODO:fflush(stdout);
    fflush(stdout);
	return 0;
}
