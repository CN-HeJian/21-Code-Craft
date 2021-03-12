#include "Integer_program.hpp"

// 单纯形法
Simplex::Simplex(int _m, int _n)
{
    this->m = _m;
    this->n = _n;
    int i;
    for (i = 1; i <= n; i++)
    {
        idx[i] = i; //基变量
    }
    for (i = 1; i <= m; i++)
    //添加后的松弛变量
    {
        idy[i] = i + n; //非基变量
    }
}

void Simplex::set_objective(double ci[mxn])
{
    for (int i = 1; i <= n; i++)
    {
        a[0][i] = -ci[i - 1]; //第0行作为目标函数
    }
}

void Simplex::set_co_matrix(double co_matrix[mxm][mxn])
{
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            a[i][j] = co_matrix[i - 1][j - 1];
        }
    }
}

void Simplex::set_bi_matrix(double b_matrix[mxm])
{
    for (int i = 1; i <= m; i++)
    {
        a[i][0] = b_matrix[i - 1]; //第0列作为b矩阵
    }
    a[0][0] = 0;
}

int Simplex::init_simplex()
{
    while (1)
    {

        int i, x = 0, y = 0;
        for (i = 1; i <= m; i++)
        {
            if (a[i][0] < -eps && ((!x) || (rand() & 1)))
            //如果有某个b[i] 即是约束小于0的
            {
                x = i;
            }
        }
        if (!x)
            break; //没有小于0的
        for (i = 1; i <= n; i++)
        {
            if (a[x][i] < -eps && ((!y) || (rand() & 1)))
            //从刚刚那一行bi小于0的那行,找到另外一个小于0的
            {
                y = i;
            }
        }
        if (!y)
        {
            std::cout << "Infeasible";
            return 0;
        }
        Pivot(x, y); //把第x行的第y列的元素作为主元 进行高斯消元
    }
    return 1;
}

void Simplex::Pivot(int x, int y)
{
    //用idy代换idx
    std::swap(idy[x], idx[y]);
    double tmp = a[x][y];
    a[x][y] = 1 / a[x][y];
    int i, j;
    top = 0;
    for (i = 0; i <= n; i++)
    {
        if (y != i)
            a[x][i] /= tmp;
    }

    for (i = 0; i <= n; i++)
    {
        if ((y != i) && fabs(a[x][i]) > eps)
        {
            st[++top] = i;
        }
    }
    for (i = 0; i <= m; i++)
    {
        if ((i == x) || (fabs(a[i][y]) < eps))
        {
            continue;
        }
        for (j = 1; j <= top; j++)
        {
            a[i][st[j]] -= a[x][st[j]] * a[i][y];
        }
        a[i][y] = -a[i][y] / tmp;
    }
    return;
}

int Simplex::run()
{
    int init = init_simplex();
    if (init == 0)
    {
        return init; //无解
    }
    int i, j;
    while (1)
    {
        int x = 0, y = 0;
        double mn = 1e15;
        for (i = 1; i <= n; i++)
        {
            if (a[0][i] > eps)
            {
                y = i;
                break;
            }
        }
        if (!y)
            break;
        for (i = 1; i <= m; i++)
        {
            if (a[i][y] > eps && (a[i][0] / a[i][y] < mn))
            {
                mn = a[i][0] / a[i][y];
                x = i;
            }
        }
        if (!x)
        {
            std::cout << "Unbounded";
            return -1; //无界
        }
        Pivot(x, y);
    }
    return 1; //有解
}

std::pair<std::vector<double>, double> Simplex::getans()
{
    std::vector<double> x;
    double z;
    int i;
    z = a[0][0];
    for (i = 1; i <= n; i++)
    {
        a[0][i] = 0;
    }
    for (i = 1; i <= m; i++)
    {
        if (idy[i] <= n)
            a[0][idy[i]] = a[i][0];
    }
    for (i = 1; i <= n; i++)
    {
        x.push_back(a[0][i]);
    }
    return std::pair<std::vector<double>, double>(x, z);
}

// 分支定界法求解整数规划问题

Integer_program::Integer_program(int server_num)
{

}

Integer_program::~Integer_program()
{
    delete[] m_solver;
}

void Integer_program::set_all_servers(std::vector<server_data> server_data, int min_cpu, int min_ram)
{
    m_server_num = server_data.size();
    // 约束 server_num+2
    // 变量 server_num+1
    m_solver = new Simplex(m_server_num + 2, m_server_num + 1);
    // cpu 约束
    b_matrix[0] = -min_cpu;
    for (int i = 0; i < server_data.size(); i++)
    { // 遍历所有的服务器获取其对应的cpu数目
        co_matrix[0][i] = -server_data.at(i).m_CPU_num;
    }
    // ram 约束
    b_matrix[1] = -min_ram;
    for (int i = 0; i < server_data.size(); i++)
    { // 遍历所有服务其获取其对应的ram数目
        co_matrix[1][i] = -server_data.at(i).m_RAM;
    }
    // 非零约束
    for (int i = 0; i < server_data.size(); i++)
    {
        co_matrix[2 + i][i] = -1;
    }
}

// 求解
std::vector<int> Integer_program::solve(bool fast)
{
    std::vector<int> result;
    if (fast)
    {
        // 目前采用单纯形法求松弛问题
        m_solver->set_objective(c_matrix);
        m_solver->set_co_matrix(co_matrix);
        m_solver->set_bi_matrix(b_matrix);
        m_solver->run();
        // 直接取整数
        std::pair<std::vector<double>, double> rst = m_solver->getans();
        for(int i = 0;i<m_server_num;i++)
        {
            result.emplace_back((rst.first.at(i) + 0.99));//基本等于向上取整 
        }
    }
    return result;
}
