// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "examples.h"
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <conio.h>
#include <Eigen/Dense>

using namespace std;
using namespace seal;
using namespace Eigen;

void print_hex(const unsigned char *data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        printf("%02x", data[i]);
    }
    printf("\n");
}
uint64 inv(uint64 a, uint64 mod)
{
    //cout << "kkkkkkkkkkkkkkkkkkkkkkkkk " << a << "aaaaaaaaaaaaaaaaaaaaaaaaa" << mod;
    uint64 ret = 1, b = mod - 2;
    for (; b; b >>= 1)
    {
        if (b & 1)
            ret = (uint64)((ret * (a % mod) % mod)) % mod;
        a = (uint64)(a * a) % mod;
    }
    //cout << "   " << ret % mod << "\n";
    return ret % mod;
}

void OUTA(vector<vector<uint64>> A)
{
    cout << "*****************A\n";
    for (auto& a : A)
    {
        for (auto& b : a)
        {
            cout << b << " ";
        }
        cout << "\n";
    }
    cout << "****************over A\n";
}

void OUTB(vector<uint64> B)
{
    cout << "*****************B\n";
    for (auto& b : B)
    {
        cout << b << " ";
    }
    cout << "\n";
    cout << "****************over B\n";
}

uint64 solve(uint64 startx, uint64 starty, vector<vector<uint64>>& A, vector<uint64>& B, int N, int M, uint64 MOD)
{
    //cout << "startx = " << startx << "  starty = " << starty << "\n";
    for (int i = 0; i + startx < N; i++)
    {
        int targetrow = -1;
        for (int j = i; j + startx < N; j++)
        {
            if (A[startx + j][starty + i] != 0)
            {
                targetrow = j;
                break;
            }
        }
        //cout << "targetrow = " << targetrow << "\n";
        if (targetrow == -1)
        {
            return i;
            //cout << "hang = " << i << "没找到\n";
            //continue;
        }
            

        for (int j = 0; j + starty < M; j++)
            swap(A[startx + i][starty + j], A[startx + targetrow][starty + j]);
        //OUTA(A);
        swap(B[startx + i], B[startx + targetrow]);
        //OUTB(B);
        uint64 INV = inv(A[startx + i][starty + i], MOD);
        //cout << "*******************INV= " << INV << "\n";
        for (int j = 0; j + starty < M; j++)
        {
            A[startx + i][starty + j] = (A[startx + i][starty + j] * INV % MOD) % MOD;
        }
        //OUTA(A);
        B[startx + i] = (B[startx + i] * INV % MOD) % MOD;
        //OUTB(B);
        // 消元
        for (int j = i + 1; j + startx < N; j++)
        {
            uint64 imp = A[startx + j][starty + i];
            for (int k = i; k + starty < M; k++)
            {
                //cout << A[j][k] << " " << A[j][i] << " " << INV << " " << A[i][k] << "\n";
                A[startx + j][starty + k] = (A[startx + j][starty + k] + MOD - ((imp % MOD * A[startx + i][starty + k] % MOD) % MOD)) % MOD;
                //cout << A[j][k] << " " << A[j][i] << " " << INV << " " << A[i][k] << "\n";
            }
            B[startx + j] = (B[startx + j] + MOD - ((imp % MOD * B[startx + i] % MOD) % MOD)) % MOD;
        }
        //OUTA(A);
        //OUTB(B);
    }
    return -1;
}

vector<uint64> getans(vector<vector<uint64>> A, vector<uint64> B, uint64 N, uint64 M, uint64 MOD)
{
    for (int i = 0; i < N; i++)
    {
        bool allzero = true;
        for (int j = 0; j < M; j++)
        {
            if (A[i][j])
            {
                allzero = false;
                break;
            }
        }
        if (allzero == true && B[i])
        {
            cout << "----------该桶对应的数据结构无解-----------\n";
        }
    }
    vector<uint64> LR(M, 0);
    for (int i = N - 1; i >= 0; i--)
    {
        int sj = 0;
        while (A[i][sj] == 0)
        {
            sj++;
        }
        uint64 sigma = 0;
        for (int j = sj + 1; j < M; j++)
        {
            sigma = (sigma + (A[i][j] * LR[j]) % MOD) % MOD;
        }

        LR[sj] = (B[i] + MOD - (sigma % MOD)) % MOD;
    }
    // cout << "----------该桶对应的数据结构有解   ";

    bool checkok = true;
    for (int i = 0; i < N; i++)
    {
        uint64 calc = 0;
        for (int j = 0; j < M; j++)
        {
            calc = (calc + (A[i][j] * LR[j]) % MOD) % MOD;
        }
        if (calc % MOD != B[i] % MOD)
        {
            checkok = false;
            break;
        }
    }
    if (checkok)
        cout << "测试通过\n";
    else
    {
        cout << "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n";
    }
    return LR;
}

void test()
{
    //srand(time(0));
    uint64 mod = 4294967291;


    int n = 5, m = 7;
    ofstream fout;
    ifstream fin;
    fout.open("D:\\毕设\\data\\数据处理\\in.txt");
    fin.open("D:\\毕设\\data\\数据处理\\in.txt");
    cin >> n >> m;
    fout << n << " " << m << " ";
    vector<vector<uint64>> A(n, vector<uint64>(m, 0));
    vector<uint64> B(n, 0);
    vector<uint64> X(m, 0);
    
    //cout << "---------------A:\n";
    for (int i = 0; i < n; i++)
    {
        int a1 = rand() % m;
        int a2 = rand() % m;
        while (a2 == a1)
            a2 = rand() % m;
        int a3 = rand() % m;
        while (a3 == a2 || a3 == a1)
            a3 = rand() % m;
        //for (int j = 0; j < m; j++)
        //{
          //  A[i][j]= rand() % 2;
            //cout << A[i][j] << " ";
            //fout << A[i][j] << " ";
        //}
        //cout << "\n";
        A[i][a1] = 1;
        A[i][a2] = 1;
        A[i][a3] = 1;
    }
    //cout << "---------------X:\n";
    for (int i = 0; i < m; i++)
    {
        X[i] = rand() % mod;
        //cout << X[i] << " ";
        fout << X[i] << " ";
    }
    //cout << "\n";
    //cout << "---------------B:\n";
    for (int i = 0; i < n; i++)
    {
        uint64 num = 0;
        for (int j = 0; j < m; j++)
        {
            num = (num + A[i][j] * X[j] % mod) % mod;
        }
        B[i] = num;
        //cout << num << " ";
        fout << num << " ";
    }
   // cout << "\n";
    uint64 startx = 0, starty = 0;
    while (1)
    {
        int check = solve(startx, starty, A, B, n, m, 1ll * mod);
        if (check == -1)
            break;
        startx += check;
        starty += check + 1ll;
    }
    
    vector<uint64> LR = getans(A, B, n, m, 1ll * mod);
    return;
    cout << "标准: ";
    for (auto &num : X)
        cout << num << " ";
    //puts("");
    for (auto &num : LR)
    {
        cout << num << " ";
    }
}


array<uint64, 3> funlHASH(string s)
{
    // 使用字符串 s 作为种子创建一个随机数引擎
    std::seed_seq seed(s.begin(), s.end());
    std::mt19937 gen(seed);

    std::uniform_int_distribution<> dis(0, cokvsm - 1);
    // 绑定随机数引擎和分布
    auto rand_bool = std::bind(dis, gen);
    return { static_cast<uint64>(rand_bool()), static_cast<uint64>(rand_bool()), static_cast<uint64>(rand_bool()) };
}
std::string getHiddenInput(const std::string &prompt = "")
{
    std::cout << prompt;
    std::string input;
    char ch;

    while ((ch = _getch()) != '\r')
    { // 回车键结束输入
        if (ch == '\b')
        { // 处理退格键
            if (!input.empty())
            {
                input.pop_back();
                std::cout << "\b \b"; // 删除星号
            }
        }
        else if (ch >= 32 && ch <= 126)
        { // 可打印字符
            input.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return input;
}
int main()
{
    //C:\\Users\\liyongqi\\Desktop\\sq\\data.txt
    //C:\\Users\\liyongqi\\Desktop\\sq10\\data.txt
    // 13541243055
    // xuziyin
    /*第一部分放到桶里*/
    // 读入泄露数据路文件路径
    
    //ban["123456"] = 1;

    

    char upfilepath[100] = "C:\\Users\\liyongqi\\Desktop\\sq\\data.txt";
    cout << "请输入工作目录（存放初始数据库，保存中间数据）：\n";
    cin.getline(upfilepath, 100);
    //cout << strlen(upfilepath) << "\n";
    //exit(0);
    
    int idlong; // = 12;
    cout << "请输入桶标识符长度（10， 12）：\n";
    cin >> idlong;
    //char lspath[] = "C:\\Users\\liyongqi\\Desktop\\sq\\data.txt";
    // 数据库预处理
    cout << "是否已经建立数据库？(y/n)\n";
    char c = getchar();
    c = getchar();
    if (c == 'n')
    {
        
        cout << "正在建立数据库......\n";
        Bucketset(upfilepath, idlong, 100);
    }
    

    //return 0;
    
    char S[100];  //= "D:\\zzz10\\0000.txt";
    char data[100];  //= "D:\\xxx10\\0000.txt";

    int upfilelen = strlen(upfilepath);
    //char bucketfilepath[100];
    strcpy(S, upfilepath);
    strcpy(data, upfilepath);
    const char *insertchar1 = "buck0000.txt";
    const char *insertchar2 = "stru0000.txt";
    strncpy(S + upfilelen - 8, insertchar2, strlen(insertchar2));
    strncpy(data + upfilelen - 8, insertchar1, strlen(insertchar1));
    S[upfilelen + 4] = '\0';
    data[upfilelen + 4] = '\0';
    //cout << strlen(S) << '\n';
    //cout << S << "\n";
    //cout << data << "\n";
    string username = "13530307376";
    string password = "huhaidong";
    cout << "############## OPC3: 口令泄露检测服务##############\n\n\n";
    int js = 0;
    while (1) {
        cout << "\n no." << ++js << "\n";
        cout << "请输入要检测的用户名：";
        cin >> username;
        cout << "请输入对应的口令：";
        password = getHiddenInput("");
        // std::cout << "You entered: " << password << std::endl;
        cout << "\n\n";
        OPC3(data, S, username, password, idlong); // 桶标识符bit长度

        cout << "\n 退出？(y\\n): ";
        char s;
        cin >> s;
        if (s == 'y')
        {
            break;
        }
    }
    return 0;
}
