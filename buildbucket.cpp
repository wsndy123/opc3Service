#include "examples.h"

using namespace std;

BucketI::BucketI(int column)
{
    Datastruct = OKVS();
    bucketi.resize(column);
}

void BucketI::push(string val){ 
    willdel.push_back(val);
}

int BucketI::Qbucketsize() {
    return willdel.size();
}

int BucketI::Qbucketsize(bool T)
{
    int retsize = 0;
    for (auto& b : bucketi)
    {
        retsize += b.size();
    }
    return retsize;
}

void BucketI::colpush(int col, string val)
{ 
    bucketi[col].push_back(val);
}

int BucketI::Qmaxcolumnsize()
{
    int maxsize = 0;
    for (auto& b : bucketi)
    {
        maxsize = max(maxsize, (int)b.size());
    }
    return maxsize;
}

int BucketI::Qmincolumnsize()
{
    int minsize = 99999999;
    for (auto& b : bucketi)
    {
        minsize = min(minsize, (int)b.size());
    }
    return minsize;
}

Bucketset::Bucketset(char *file, int Bucketidentifierlen) {
    Bucketidentifierlen = Bucketidentifierlen;
    ifstream fin;
    fin.open(file);
    bucket.resize((1ll << Bucketidentifierlen));
    string username, password;
    while (fin >> username >> password) {
        string data = mysha256(username + password + mysha256(username + password));
        uint64 HL = hexToInt(data, Bucketidentifierlen);
        bucket[HL].push(data);
    }
}

Bucketset::Bucketset(char *file, int Bucketidentifierlen, int column)
{
    //srand(time(0));

    auto start = std::chrono::high_resolution_clock::now();

    this->Bucketidentifierlen = Bucketidentifierlen;
    this->column = column;
    ifstream fin;
    
    fin.open(file);
    //cout << (1 << Bucketidentifierlen) << "\n";
    for (int i = 0; i < (1 << Bucketidentifierlen); i++)
    {
        bucket.push_back(BucketI(column));
    }
    string username, password;
    map<string, int> ban;
    ban["123456"] = 1;
    while (fin >> username >> password){
        if (ban[username] == 1)
            continue;
        /* if (password == "82346971")
        {
            cout << "asdasdasdsadasdsadsadsadsadas\n";
        }*/
        //cout << username << password << "\n";
        //exit(0);
        string data = mysha256(username + password + mysha256(username + password));
        uint64 HL = hexToInt(mysha256(username), Bucketidentifierlen);
        uint64 randomcol = randomint(column);
        //cout << "```````````````````````````" << column << " " << randomcol << "\n";
        //cout << HL << " " << randomcol << "\n";
        bucket[HL].colpush(randomcol, data);
        bucket[HL].I.push_back(make_pair(data, randomcol));
    }
    auto end = std::chrono::high_resolution_clock::now();


    int upfilelen = strlen(file);
    char bucketfilepath[100];
    strcpy(bucketfilepath, file);
    const char *insertchar = "buck0000.txt";
    strncpy(bucketfilepath + upfilelen - 8, insertchar, strlen(insertchar));
    bucketfilepath[upfilelen + 4] = '\0';
    // char ss[50] = "D:\\xxx\\1111.txt";
    //cout << bucketfilepath << "\n";
    ofstream ffout;
    int lenbuckpath = strlen(bucketfilepath);
    for (int j = 0; j < bucket.size(); j++)
    {
        int num = j;
        bucketfilepath[lenbuckpath - 5] = num % 10 + '0';
        bucketfilepath[lenbuckpath - 6] = (num / 10) % 10 + '0';
        bucketfilepath[lenbuckpath - 7] = (num / 100) % 10 + '0';
        bucketfilepath[lenbuckpath - 8] = (num / 1000) + '0';
        cout << bucketfilepath << " \n";
        ffout.open(bucketfilepath);
        BucketI bi = bucket[j];
        for (int i = 0; i < bi.I.size(); i++)
        {
            //cout << bi.I[i].first << " " << bi.I[i].second << "\n";
            ffout << bi.I[i].first << " " << bi.I[i].second << "\n";
            //exit(0);
        }
        cout << "桶" << j << "保存完毕"
             << "\n";
        ffout.close();
    }
   // return;


    // 计算时间差
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 输出函数执行时间
    //std::cout << "预处理头时间: " << duration.count() << " 毫秒" << std::endl;
    int js = 0;
    for (auto &bi : bucket)
    {

        cout << "桶" << js++ << "正在建立数据结构"
             << "\n";
        bi.Datastruct.buildLR(bi.I, file);
        start = std::chrono::high_resolution_clock::now();
        // bi.Datastruct.Decode(bi.I[0].first);
        auto end = std::chrono::high_resolution_clock::now();

        // 计算时间差
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 输出函数执行时间
        // std::cout << "解码时间: " << duration.count() << " 毫秒" << std::endl;
    }
    //return;
    
    
   
}

void Bucketset::Eachbucketmaxmin()
{
    for (auto& b : bucket)
    {
        cout << "MAXcolsize = " << b.Qmaxcolumnsize() << " "
             << "MINcolsize = " << b.Qmincolumnsize() << "\n";
    }
}

void Bucketset::Eachbucketsize()
{
    int bid = 0;
    int maxe = 0, mine = 99999999;
    for (auto& a : bucket)
    {
        int size_ = a.Qbucketsize(1);
        maxe = max(maxe, size_);
        mine = min(mine, size_);
        cout << "Bucket " << bid++ << " " << size_ << "\n";
    }
    cout << "MAX = " << maxe << "\n";
    cout << "MIN = " << mine << "\n";
}
