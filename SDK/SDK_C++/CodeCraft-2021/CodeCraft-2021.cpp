#include "iostream"
#include "manager.hpp"
//#include "tools.hpp"

using namespace std;

int main()
{
    clock_start();
	// TODO:read standard input
	//string inputTxtName = "/home/lyc/21-Code-Craft/training-data/training-1.txt";
    //string inputTxtName = "/home/jian/Downloads/demo/21-Code-Craft/training-data/training-1.txt";
    string inputTxtName = "/home/icf/Desktop/3_16/21-Code-Craft/training-data/training-1.txt";
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


