#include <iostream>
#include <iomanip>  // 设置输出格式
#include <fstream>  // 打开和读取文件
#include <sstream>  // 拆分 csv 文件
#include <map>      // 按类别累加金额
#include <cctype>   // 用于 isdigit 判断

#ifdef _WIN32       // 防止中文乱码
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

int main()
{
    #ifdef _WIN32   // 设置控制台输出为 UTF-8 编码
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdout), _O_U8TEXT);
    #endif

    ifstream rawDATA("data.csv");  // 打开 CSV 文件

    double tI = 0, tE = 0;  // 初始化总收入和总支出
    map<string, double> incomeCate;   // 按收入类别统计
    map<string, double> expenseCate;  // 按支出类别统计
    
    string line;
    for (int i = 0; i < 17 && getline(rawDATA, line); ++i);  // 跳过前 17 行

    while (getline(rawDATA, line))
    {
        // 拆分前6列
        stringstream DATA(line);
        string cells[6];
        for (int i = 0; i < 6; ++i)
        {
            if (!getline(DATA, cells[i], ',')) 
            cells[i] = "";
            // 去掉首尾双引号
            if (cells[i].front() == '"' && cells[i].back() == '"') 
            cells[i] = cells[i].substr(1, cells[i].size() - 2);
        }

        string category = cells[1];  // B列 交易类型
        string type = cells[4];      // E列 收入/支出
        string jin_e = cells[5];     // F列 金额

        // 去除开头非数字字符
        int position = 0;
        int& pos = position;
        while (pos < jin_e.size() && !isdigit(jin_e[pos])) ++pos;
        jin_e = jin_e.substr(pos);
        double jin_e1 = stod(jin_e);  // 转换为 double 类型

        if (type == "收入")
        {
            tI += jin_e1;
            incomeCate[category] += jin_e1;
        }
        else if (type == "支出")
        {
            tE += jin_e1;
            expenseCate[category] += jin_e1;
        }
    }

    ofstream outFile("dataex.txt");
    
    
    outFile << "===========================" << endl;
    outFile << "总收入——¥" << tI << fixed << setprecision(2) << endl;
    outFile << "这包括——————" << endl;
    for (pair pair0 : incomeCate)
    {outFile<< pair0.first << "：¥" << pair0.second << endl; }
    outFile << "===========================" << endl;
    outFile << "总支出——¥" << tE << endl;
    outFile << "这包括——————" << fixed << setprecision(2) << endl;
    for (pair pair1 : expenseCate)
    {outFile<< pair1.first << "：¥" << pair1.second << endl; }
    outFile << "===========================" << endl;
    cout << "已成功生成 dataex.txt ! " << endl;

    return 0;
}
