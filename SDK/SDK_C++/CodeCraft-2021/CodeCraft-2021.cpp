#include "iostream"
#include "manager.hpp"

using namespace std;

int main()
{
	manager m;
	m.readTxtbyStream();
	m.processing();
    m.result();
    fflush(stdout);
	return 0;
}


