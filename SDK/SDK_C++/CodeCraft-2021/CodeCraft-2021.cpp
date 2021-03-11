#include "iostream"
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;

const int N = 1000;

struct Server{
	string type;
	int coreNum;
	int memory;
	int firstCost;
	int onCost;
}server[N];

struct VM{
	string type;
	int needcoreNum;
	int needMemory;
	int isdoubleNode;
}vm[N];

struct Operation{
	string type;
	string vm_type;
	int vm_id;
}operation[N];


int server_nums;
int vm_nums;
int days;
int operation_nums_perday[N];

void readTxt(const string& inputFile){
	int fd = open(inputFile.c_str(), O_RDONLY);
	if(fd==-1) printf("fail to open files");

    struct stat sb;
    fstat(fd, &sb);
    char *buffer = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);// 返回的是指向映射区域首地址的指针
    if(buffer==nullptr || buffer==(void*)-1){
      close(fd);
      exit(-1);
    }
    close(fd);

	int choice = 1;//1：读服务器 2：读虚拟机 3：读操作数
    int val=0;
    while(*buffer){
		// 开始进行数据读取的分析
		if(*buffer >= '0' && *buffer <= '9'){
			val = val*10 + (*buffer - '0');
		}
		
		else if(*buffer == '\n'){
			if(choice == 1){
				choice++;
				server_nums = val;
				val = 0;
				//开始输入接下来的server_nums行
				for(int row=0; row<server_nums; row++){
					buffer++;// 进入新的一行
					string temp;
					while(*buffer != '\n'){
						temp.push_back(*buffer);
						buffer++;
					}
					// 分割字符串temp，temp的样式：(host0Y6DP, 300, 830, 141730, 176)
					int len = temp.size();
					string t;
					int elemNum = 0;
					for(int n=1; n<len; n++){
						if(temp[n]==',' || temp[n]==')'){
							if(elemNum == 0){
								server[row].type = t;
								t.clear();
							}else if(elemNum == 1){
								server[row].coreNum = stoi(t);
								t.clear(); 
							}else if(elemNum == 2){
								server[row].memory = stoi(t);
								t.clear();
							}else if(elemNum == 3){
								server[row].firstCost = stoi(t);
								t.clear();
							}else{
								server[row].onCost = stoi(t);
								t.clear();
							}
							elemNum++;
							continue;
						}
						t.push_back(temp[n]);
					}
				}	
			}
			else if(choice == 2){
				choice++;
				vm_nums = val;
				val = 0;
				//开始输入接下来的vm_nums行
				for(int row=0; row<vm_nums; row++){
					buffer++;// 进入新的一行
					string temp;
					while(*buffer != '\n'){
						temp.push_back(*buffer);
						buffer++;
					}
					// 分割字符串temp，temp的样式：(vm38TGB, 124, 2, 1)
					int len = temp.size();
					string t;
					int elemNum = 0;
					for(int n=1; n<len; n++){
						if(temp[n]==',' || temp[n]==')'){
							if(elemNum == 0){
								vm[row].type = t;
								t.clear();
							}else if(elemNum == 1){
								vm[row].needcoreNum = stoi(t);
								t.clear(); 
							}else if(elemNum == 2){
								vm[row].needMemory = stoi(t);
								t.clear();
							}else{
								vm[row].isdoubleNode = stoi(t);
								t.clear();
							}
							elemNum++;
							continue;
						}
						t.push_back(temp[n]);
					}
				}
			}
			else if(choice == 3){
				days = val;// 总的运行天数:1～days
				val = 0;
				choice++;
				int count = 0;//操作数的索引下标
				//开始输入接下来的days天的操作
				for(int day=0; day<days; day++){
					// 读取每一天的操作数
					buffer++;// 进入下一行
					string temp;// 存放一行
					while(*buffer != '\n'){
						temp.push_back(*buffer);
						buffer++;						
					}
					int operations = stoi(temp);
					temp.clear();

					operation_nums_perday[day] = operations;// 每一天的操作数存入数组中

					// 读取当天所有操作的具体指令
					for(int i=0; i<operations; i++){
						buffer++;// 进入当天操作指令的第一行
						string t;
						while(*buffer != '\n'){
							t.push_back(*buffer);
							buffer++;
						}
						// 分割字符串t,t的样式：(add, vmVDAZV, 381492167)或(del, 264022204)
						int len = t.size();
						string t_t;
						int elemNum = 0;
						for(int n=1; n<len; n++){
							if(t[n]==',' || t[n]==')'){
								if(elemNum == 0){
									operation[count].type = t_t;
									t_t.clear();
								}else if(elemNum == 1 && operation[count].type == "add"){
									t_t.erase(0,1);//删除头部空格
									operation[count].vm_type = t_t;
									t_t.clear(); 
								}else{
									operation[count].vm_id = stoi(t_t);
									t_t.clear();
								}
								elemNum++;
								continue;
							}
							t_t.push_back(t[n]);
						}
						count++;
					}
				}
			}
		}
		buffer++;
    }
    munmap(buffer, sb.st_size);// 解除内存映射
}

int main()
{
	// TODO:read standard input
	string inputTxtName = "../../../training-data/test.txt";
	readTxt(inputTxtName);
	cout<<endl;
	cout<<server_nums<<" "<<vm_nums<<" "<<days<<endl;

	for(int i=0; i<server_nums; i++){
		cout<<server[i].type<<" "<<server[i].coreNum<<" "<<server[i].memory<<" "<<
		server[i].firstCost<<" "<<server[i].onCost;
	}
	cout<<endl;
	for(int i=0; i<vm_nums; i++){
		cout<<vm[i].type<<" "<<vm[i].needcoreNum<<" "<<vm[i].needMemory<<" "<<vm[i].isdoubleNode;
	}
	cout<<endl;
	for(int i=0; i<days; i++){
		cout<<operation_nums_perday[i]<<" ";
	}
	cout<<endl;

	for(int i=0; i<7; i++){
		cout<<operation[i].type<<" "<<operation[i].vm_type<<" "<<operation[i].vm_id<<" ";
	}
	cout<<endl;
	// TODO:process
	// TODO:write standard output
	// TODO:fflush(stdout);

	return 0;
}
