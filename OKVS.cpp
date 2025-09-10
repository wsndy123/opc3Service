#include "examples.h"

int jsjsjs;

array<uint64, 3> OKVS::funlHASH(string s)
{
    // 返回三个不同的位置
    // 使用字符串 s 作为种子创建一个随机数引擎
    std::seed_seq seed(s.begin(), s.end());
    std::mt19937 gen(seed);

    // 创建一个分布，用于生成 0 或 1
    std::uniform_int_distribution<> dis(0, cokvsm - 1);
    // 绑定随机数引擎和分布
    auto rand_bool = std::bind(dis, gen);
    array<uint64, 3> ret = { static_cast<uint64>(rand_bool()), static_cast<uint64>(rand_bool()),
                             static_cast<uint64>(rand_bool()) };
    while (ret[1] == ret[0])
    {
        ret[1]++;
        ret[1] %= cokvsm;
    }
    while (ret[2] == ret[0])
    {
        ret[2]++;
        ret[2] %= cokvsm;
    }
        
    while (ret[2] == ret[1])
    {
        ret[1]++;
        ret[1] %= cokvsm;
    }
        
    return ret;
}

vector<bool> OKVS::funrBIT(string s)
{
    // 使用字符串 s 作为种子创建一个随机数引擎
    std::seed_seq seed(s.begin(), s.end());
    std::mt19937 gen(seed);

    // 创建一个分布，用于生成 0 或 1
    std::uniform_int_distribution<> dis(0, 1);

    // 绑定随机数引擎和分布
    auto rand_bool = std::bind(dis, gen);

    // 创建一个长度为 okvsr 的 vector<bool>
    std::vector<bool> result(cokvsr);

    // 填充 vector<bool> 为随机的布尔值
    for (size_t i = 0; i < cokvsr; ++i)
    {
        result[i] = static_cast<bool>(rand_bool());
    }

    return result;
}

uint64 OKVS::inv(uint64 a)
{
    uint64 ret = 1, b = Fp - 2, mod = Fp;
    for (; b; b >>= 1)
    {
        if (b & 1)
            ret = (uint64)((ret * (a % mod) % mod)) % mod;
        a = (uint64) (a * a) % mod;
    }
    return ret % mod;
}

uint64 OKVS::solve(uint64 startx, uint64 starty, vector<vector<uint64>> &A, vector<uint64> &B, int N, int M, uint64 MOD)
{
    // cout << "startx = " << startx << "  starty = " << starty << "\n";
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
        // cout << "targetrow = " << targetrow << "\n";
        if (targetrow == -1)
        {
            return i;
            // cout << "hang = " << i << "没找到\n";
            // continue;
        }

        for (int j = 0; j + starty < M; j++)
            swap(A[startx + i][starty + j], A[startx + targetrow][starty + j]);
        // OUTA(A);
        swap(B[startx + i], B[startx + targetrow]);
        // OUTB(B);
        uint64 INV = inv(A[startx + i][starty + i]);
        // cout << "*******************INV= " << INV << "\n";
        for (int j = 0; j + starty < M; j++)
        {
            A[startx + i][starty + j] = (A[startx + i][starty + j] * INV % MOD) % MOD;
        }
        // OUTA(A);
        B[startx + i] = (B[startx + i] * INV % MOD) % MOD;
        // OUTB(B);
        // 消元
        for (int j = i + 1; j + startx < N; j++)
        {
            uint64 imp = A[startx + j][starty + i];
            for (int k = i; k + starty < M; k++)
            {
                // cout << A[j][k] << " " << A[j][i] << " " << INV << " " << A[i][k] << "\n";
                A[startx + j][starty + k] =
                    (A[startx + j][starty + k] + MOD - ((imp % MOD * A[startx + i][starty + k] % MOD) % MOD)) % MOD;
                // cout << A[j][k] << " " << A[j][i] << " " << INV << " " << A[i][k] << "\n";
            }
            B[startx + j] = (B[startx + j] + MOD - ((imp % MOD * B[startx + i] % MOD) % MOD)) % MOD;
        }
        // OUTA(A);
        // OUTB(B);
    }
    return -1;
}


vector<uint64> OKVS::getans(vector<vector<uint64>> A, vector<uint64> B, uint64 N, uint64 M, uint64 MOD)
{
    vector<uint64> LR(M, 0);
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
            exit(0);
            return LR;
        }
    }
    
    for (int i = N - 1; i >= 0; i--)
    {
        int sj = 0;
        while (A[i][sj] == 0 && sj < M)
        {
            sj++;
        }
        uint64 sigma = 0;
        for (int j = sj + 1; j < M; j++)
        {
            if (LR[j] == 0)
            {
                LR[j] = randomint(Fp - 1) % Fp;
            }
            sigma = (sigma + (A[i][j] * LR[j]) % MOD) % MOD;
        }

        if (sj == M) // 这一行全是0  自由变量
        {
            LR[i] = randomint(Fp - 1) % Fp;
        }
        else
        {
            LR[sj] = (B[i] + MOD - (sigma % MOD)) % MOD;
        }
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
    {
        cout << "高斯消元测试通过\n";
    }
        
    else
    {
        cout << "高斯消元测试失败\n";
        exit(0);
    }
    return LR;
}
int bucketjs;


uint64 OKVS::Decode(string k)
{
    array<uint64, 3> h = funlHASH(k);
    vector<bool> fr = funrBIT(k);
    uint64 v = (L[h[0]] + L[h[1]] + L[h[2]]) % Fp;
    for (int i = 0; i < cokvsr; i++)
    {
        v = (v + R[i] * fr[i]) % Fp;
    }
    return v % Fp;
}

void OKVS::printLR()
{
    for (auto& a : L)
    {
        cout << a << " ";
    }
    for (auto& a : R)
    {
        cout << a << " ";
    }
    cout << "\n";
}

void OKVS::checkGetLR(char* Sfile)
{
    ifstream fin;
    fin.open(Sfile);
    for (int i = 0; i < cokvsm; i++)
    {
        uint64 num;
        fin >> num;
        L.push_back(num);
    }
    for (int i = 0; i < cokvsr; i++)
    {
        uint64 num;
        fin >> num;
        R.push_back(num);
    }
    fin.close();
}

void OKVS::buildLR(vector<pair<string, int>> I, char *file)
{
   // auto start = std::chrono::high_resolution_clock::now();

    //cout << "bucket new -----------------------------------------------------\n";
    vector<pair<string, int>> isolated; // 孤立键
    vector<vector<pair<string, int>>> buck(cokvsm);
    map<pair<string, int>, int> keyToplace; //孤立键的位置
    map<pair<string, int>, int> inisolated;
    bucketjs++;

    cout << "start \n";
    map<string, int> mp;
    cout << I.size();
    for (auto& kv : I)
    {
        if (mp[kv.first] == 1)
        {
            cout << "有重复的键\n";
            exit(0);
        }
        mp[kv.first] = 1;
        
        //cout << "### " << kv.first << " " << kv.second << "\n";
        for (auto &p : funlHASH(kv.first))
        {
            
            buck[p].push_back(kv);

        }
        
    }
    
    for (int i = 0; i < cokvsm; i++)
    {
        //cout << buck[i].size() << " ";
        if (buck[i].size() == 1 && inisolated[buck[i][0]] == 0)
        {
            inisolated[buck[i][0]] = 1;
            isolated.push_back(buck[i][0]);
            keyToplace[buck[i][0]] = i; // 这个键值对的孤立键位置
        }
    }
    //cout << "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n";
    //cout << "孤立键个数 " << isolated.size() << "\n";
    // 利用非孤立键创建方程组 inisolated = 1

    uint64 N = I.size() - isolated.size();
    uint64 M = cokvsm + cokvsr;
    //cout << "N= " << N << " M=" << M << "\n";
    vector<vector<uint64>> A(N, vector<uint64>(M, 0));
    vector<uint64> B(N, 0);
    int n = 0, m = M;
    for (auto &kv : I)
    {
        if (inisolated[kv])
            continue;
        B[n] = kv.second;
        for (auto &p : funlHASH(kv.first))
        {
            A[n][p] = 1;
        }
        vector<bool> fr = funrBIT(kv.first);
        for (int i = 0; i < cokvsr; i++)
        {
            A[n][1ll * cokvsm + i] = (uint64)(fr[i] == true);
        }
        n++;
    }
    ofstream fout;
    cout << "正在高斯消元 \n";
    uint64 startx = 0, starty = 0;
    while (1)
    {
        int check = solve(startx, starty, A, B, n, m, Fp);
        if (check == -1)
            break;
        startx += check;
        starty += check + 1ll;
    }
    vector<uint64> LR = getans(A, B, n, m, Fp);

    for (int i = 0; i < M; i++)
    {
        if (LR[i] == 0)
        {
            //print_line(__LINE__);
            //cout << "触发了\n";
            LR[i] = randomint(Fp - 1) % Fp;
        }
    }
    for (int i = 0; i < cokvsm; i++)
        L.push_back(LR[i]);
    for (int i = cokvsm; i < M; i++)
        R.push_back(LR[i]);

    

    for (auto& kv : isolated)
    {

        uint64 cdot = 0;
        vector<bool> fr = funrBIT(kv.first);
        for (int i = 0; i < cokvsr; i++)
        {
            cdot = (cdot + (R[i] * (int)fr[i]) % Fp) % Fp;
        }

        array<uint64, 3> h = funlHASH(kv.first);
        uint64 h1 = keyToplace[kv];
        uint64 h2 = (h[0] == h1 ? h[1] : h[0]);
        uint64 h3 = (h[0] + h[1] + h[2] - h1 - h2);
        L[h1] = (((kv.second + (Fp - L[h2])) % Fp + (Fp - L[h3])) % Fp + (Fp - cdot)) % Fp;
    }
    auto end = std::chrono::high_resolution_clock::now();
    //auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    //cout << "编码执行时间: " << duration.count() << " 毫秒" << std::endl;
    // ************** 测试一下
    
    cout << "当前桶大小" << I.size() << "\n";
    if (I.size() == 932)
    {
        cout << "ok\n";
    }
    for (int i = 0; (I.size() != 0) & (i < 10); i++)
    {
        
        int id = rand() % I.size();
        //print_line(__LINE__);
        cout << "test id= " << id << " ";
        if (Decode(I[id].first) == I[id].second)
        {
            cout << "解码测试通过\n";
        }
        else
        {
            cout << "解码测试失败";
            exit(0);
            cout << "\n";
        }
    }
    
    // **************

   // return;
    

    int upfilelen = strlen(file);
    char strufilepath[100];
    strcpy(strufilepath, file);
    const char *insertchar = "stru0000.txt";
    strncpy(strufilepath + upfilelen - 8, insertchar, strlen(insertchar));
    strufilepath[upfilelen + 4] = '\0';
    //char ss[50] = "D:\\zzz\\1111.txt";
    int num = bucketjs - 1;
    int lenstrupath = strlen(strufilepath);
    strufilepath[lenstrupath - 5] = num % 10 + '0';
    strufilepath[lenstrupath - 6] = (num / 10) % 10 + '0';
    strufilepath[lenstrupath - 7] = (num / 100) % 10 + '0';
    strufilepath[lenstrupath - 8] = (num / 1000) + '0';
    ofstream ffout;

   ffout.open(strufilepath);
    for (auto& a : L)
    {
        ffout << a << " ";
    }
    for (auto& a : R)
    {
        ffout << a << " ";
    }
    cout << "数据结构" << num << "保存完毕\n";
    return;
}