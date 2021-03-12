#ifndef __INTER_PROGRAM_H
#define __INTER_PROGRAM_H 
#include<iostream>
#include<algorithm>
#include<cstring>
#include<cstdio>
#include<cmath>
#include <vector>
#include "server.hpp"

const double eps = 1e-20;
enum{ mxn = 400, mxm = 200 };//mxn是未知数的个数（包括松弛变量）mxm 约束个数

// 单纯性法求解 
class Simplex
{
public:
	int n, m, t;
	double c[mxn] ;
	double a[mxm][mxn] ;
	int idx[mxn] , idy[mxn] ;
	int st[mxn] , top = 0;

	Simplex(int _m, int _n);
	void set_objective(double ci[mxn]);
	void set_co_matrix(double co_matrix[mxm][mxn]);
	void set_bi_matrix(double b_matrix[mxm]);
	int init_simplex();
	void Pivot(int x, int y);
	int run();
	std::pair<std::vector<double>, double> getans();
};

// 分支定界法求解整数规划问题
class Integer_program
{
public:
    Integer_program(int server_num);
    ~Integer_program();
    void set_all_servers(std::vector<server_data> server_data,int min_cpu,int min_ram);
    void get_min_CPU(int cpu);
    void get_min_RAM(int ram); 
    std::vector<int> solve(bool fast);
private:
    Simplex *m_solver;// 求解器
    std::vector<server_data> m_server_data;// 服务器数据 
    int m_server_num;
    // 单纯性法求解约束
    double b_matrix[mxm] = { 0 };
    double co_matrix[mxm][mxn] = { 0 };
    double c_matrix[mxn] = { 0 };
};


#endif //__INTER_PROGRAM_H
