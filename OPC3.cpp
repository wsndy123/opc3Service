#include "examples.h"
#include <stack>
#include <cctype>

using namespace seal;

void stringTonum(vector<uint64>& ls, string d)
{
    stack<uint64> S;
    for (int i = 0; i < 4; i++)
    {
        uint64 num = 0;
        for (int j = 64 - (i + 1) * 16; j < 64 - i * 16; j++)
        {
            char str[2] = { d[j], '\0' };
            num <<= 4;
            num |= static_cast<uint64>(std::strtol(str, nullptr, 16));
        }
        S.push(num);
    }
    while (!S.empty())
    {
        ls.push_back(S.top());
        //cout << S.top() << " ";
        S.pop();
    }
}

void dataTonum(vector<uint64>& ret, vector<string> data)
{
	for (auto& d : data)
	{
        //cout << d << " ";
        stringTonum(ret, d);
	}
}

string uint64Tohexstring(uint64 a)
{
    std::stringstream ss;
    ss << std::hex << a; // 使用 std::hex 将十进制数值转换为十六进制字符串
    std::string hex_string = ss.str();
    for (char &c : hex_string)
    {
        if (islower(c))
        {
            c = std::toupper(c);
        }
    }
    return hex_string;
}

inline void ciphervectorAdd(vector<Ciphertext> a, vector<Ciphertext>& sigma, Evaluator& e) 
{
    if (sigma.size() == 0) {
        for (auto& b : a) {
            sigma.push_back(b);
        }
        return ;
    }
    int la = a.size(), lsigma = sigma.size(), i = 0;
    for (i = 0; i < min(la, lsigma); i++)
    {
        e.add_inplace(sigma[i], a[i]);
    }
    for (; i < la; i++)
    {
        sigma.push_back(a[i]);
    }
}

void checkfile(string s, char *datafile)
{
    ifstream fin;
    for (int i = 0; i < 1024; i++)
    {
        /*
        datafile[12 - 2] = i % 10 + '0';
        datafile[11 - 2] = i / 10 % 10 + '0';
        datafile[10 - 2] = i / 100 % 10 + '0';
        datafile[9 - 2] = i / 1000 + '0';
        */
        datafile[12] = i % 10 + '0';
        datafile[11] = i / 10 % 10 + '0';
        datafile[10] = i / 100 % 10 + '0';
        datafile[9] = i / 1000 + '0';

        fin.open(datafile);
        string stringdata;
        uint64 col;
        while (fin >> stringdata >> col)
        {
            if (stringdata == s)
            {
                cout << "当前查询在桶" << i << "中\n";
                return;
            }
        }
        fin.close();
    }
    cout << "检测到不在任何桶中\n";
}




int OPC3(char *datafile, char *Sfile, string username, string password, int bl)
{
    /*
    这里只实现PIR部分
    */
    //cout << "datafile = " << datafile << "\n";
    //cout << "Sfile    = " << Sfile << "\n";
    string upC = mysha256(username + password + mysha256(username + password)); // 自己数据
    //cout << "upC=" << upC << "\n";
    //checkfile(upC, datafile);

    uint64 HL = hexToInt(mysha256(username), bl);
    cout << "**提示：如果检测到泄露理应在桶" << HL << "中\n\n";
    const uint64 n = 100, sqrtn = 10;
    vector<vector<vector<uint64>>> data(sqrtn, vector<vector<uint64>>(sqrtn));
    vector<vector<vector<string>>> datastring(sqrtn, vector<vector<string>>(sqrtn));
    ifstream fin;

    int lendatapath = strlen(datafile);
    int lenSfilepath = strlen(Sfile);
    datafile[lendatapath - 5] = HL % 10 + '0';
    datafile[lendatapath - 6] = HL / 10 % 10 + '0';
    datafile[lendatapath - 7] = HL / 100 % 10 + '0';
    datafile[lendatapath - 8] = HL / 1000 + '0';
    Sfile[lenSfilepath - 5] = HL % 10 + '0';
    Sfile[lenSfilepath - 6] = HL / 10 % 10 + '0';
    Sfile[lenSfilepath - 7] = HL / 100 % 10 + '0';
    Sfile[lenSfilepath - 8] = HL / 1000 + '0';

    //cout << "数据文件" << datafile << "\n";
    //cout << "结构文件" << Sfile << "\n";
    fin.open(datafile);

    string fins;
    uint64 fina;
    // 服务器编码数据库
    double time1;
    auto start = std::chrono::high_resolution_clock::now();

    while (fin >> fins >> fina)
    {
        datastring[fina / sqrtn][fina % sqrtn].push_back(fins);
         //cout << fins << " " << fina << "\n";
    }
    // 每64比特是一个ull
    for (int i = 0; i < sqrtn; i++)
    {
        for (int j = 0; j < sqrtn; j++)
        {
            dataTonum(data[i][j], datastring[i][j]);
        }
    }
    //for (auto lls : data[0][1])
   // {
        //cout << lls << " ";
  //  }

    fin.close();
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    auto context = SEALContext::Create(parms);
    KeyGenerator keygen(context);
    PublicKey public_key = keygen.public_key();
    SecretKey secret_key = keygen.secret_key();
    RelinKeys relin_keys = keygen.relin_keys_local();
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    BatchEncoder batch_encoder(context);

    size_t slot_count = batch_encoder.slot_count();
    //cout << "槽" << slot_count;
    //size_t row_size = slot_count / 2;

    vector<vector<Plaintext>> plaindata(sqrtn, vector<Plaintext>(sqrtn));
    vector<vector<Ciphertext>> cipherdata(sqrtn, vector<Ciphertext>(sqrtn));
    int maxjs = 0;
    for (int i = 0; i < sqrtn; i++)
    {
        for (int j = 0; j < sqrtn; j++)
        {
            size_t size_ = data[i][j].size();
            vector<uint64> ls(slot_count, 0ULL);
            ls[slot_count - 1] = 1;
            int js = 0;
            // 每个64比特拆成16比特放到一个槽里
            for (int k = 0; k < size_ / 4; k++)
            {
                int s = k * 4;
                int t = s + 4;
                for (int r = s; r < t; r++)
                {
                    uint64 num = data[i][j][r];
                    ls[js++] = (num >> 48);
                    ls[js++] = ((num << 16) >> 48);
                    ls[js++] = ((num << 32) >> 48);
                    ls[js++] = ((num << 48) >> 48);
                }
            }
            if (js > maxjs)
            {
                maxjs = js;
            }
            Plaintext P;
            batch_encoder.encode(ls, P);
            plaindata[i][j] = P;
        }
    }
    //cout << "maxjs = " << maxjs << "\n";
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    time1 = duration.count();
    

    double time2;
    start = std::chrono::high_resolution_clock::now();
    // 客户端 构造查询密文 加密01
    OKVS ls;
    ls.checkGetLR(Sfile);
    // ls.printLR();
    uint64 belcol = ls.Decode(upC);
    //cout << "belcol = " << belcol << "\n";
    if (belcol < 0 || belcol > 100)
    {
        cout << "***************************************************\n";
        cout << "**  该用户名-口令对没有在本数据库中泄露！！！    **\n";
        cout << "***************************************************\n";
        return 0;
    }
    else {
        cout << "**提示：服务器数据库编码时间: " << time1 << " 毫秒" << std::endl;
    }
    // return 0;
    uint64 askx = belcol / sqrtn;
    uint64 asky = belcol % sqrtn;
    cout << askx << " " << asky << "\n";
    vector<Ciphertext> askcipherx, askciphery;

    for (int i = 0; i < sqrtn; i++)
    {
        vector<uint64> all1(slot_count, 1ULL), all0(slot_count, 0ULL);
        all0[slot_count - 1] = 1;
        Plaintext one, zero;
        batch_encoder.encode(all1, one);
        batch_encoder.encode(all0, zero);
        Ciphertext Eone, Ezero;
        if (i == askx)
        {
            encryptor.encrypt(one, Eone);
            askcipherx.push_back(Eone);
        }
        else
        {
            encryptor.encrypt(zero, Ezero);
            askcipherx.push_back(Ezero);
        }
        if (i == asky)
        {
            encryptor.encrypt(one, Eone);
            askciphery.push_back(Eone);
        }
        else
        {
            encryptor.encrypt(zero, Ezero);
            askciphery.push_back(Ezero);
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    time2 = duration.count();
    cout << "**提示：客户端形成查询密文: " << time2 << " 毫秒" << std::endl;

    // 服务器形成 Answer
    double time3;
    start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < sqrtn; j++)
    {
        for (int i = 0; i < sqrtn; i++)
        {
            auto d = plaindata[i][j];
            Ciphertext data01;
            evaluator.multiply_plain(askciphery[j], d, data01);
            cipherdata[i][j] = data01;
        }
    }

    vector<Ciphertext> sigmaline;
    for (int i = 0; i < sqrtn; i++)
    {
        Ciphertext sigma;
        evaluator.add(cipherdata[i][0], cipherdata[i][1], sigma);
        for (int j = 2; j < sqrtn; j++)
        {
            evaluator.add_inplace(sigma, cipherdata[i][j]);
        }
        sigmaline.push_back(sigma);
    }
    for (int i = 0; i < sqrtn; i++)
    {
        evaluator.multiply_inplace(sigmaline[i], askcipherx[i]);
    }
    Ciphertext Answer;
    evaluator.add(sigmaline[0], sigmaline[1], Answer);
    for (int i = 2; i < sqrtn; i++)
    {
        evaluator.add_inplace(Answer, sigmaline[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    time3 = duration.count();
    cout << "**提示：服务器形成查询Answer: " << time3 << " 毫秒" << std::endl;
    // 客户端Recover
    double time4;
    start = std::chrono::high_resolution_clock::now();
    vector<uint64> recover, Recover;
    Plaintext reply;
    decryptor.decrypt(Answer, reply);
    batch_encoder.decode(reply, recover);
    size_t size_ = recover.size();
    for (int i = 0; i < size_;)
    {
        uint64 num1 = recover[i++], num2 = recover[i++], num3 = recover[i++], num4 = recover[i++];
        uint64 ans = (num1 << 48) | (num2 << 32) | (num3 << 16) | num4;
        Recover.push_back(ans);
    }
    vector<uint64> numupC;
    stringTonum(numupC, upC);
    if (Recover.size() % 4)
    {
        cout << "Error！！！！！！！\n";
    }
    size_ = Recover.size();
    bool flag = 0;
    for (int i = 0; i < size_ / 4; i++)
    {
        bool nowflag = 1;
        for (int j = 0; j < 4; j++)
        {
            if (Recover[j + (i * 4)] != numupC[j])
            {
                nowflag = 0;
                break;
            }
        }
        if (nowflag)
        {
            //cout << "found!!!!!!!!!!!!!!!!!!!!!!!\n";
            // return 0;
            flag = 1;
            break;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    time4 = duration.count();
    cout << "**提示：客户端Recover时间: " << time4 << " 毫秒" << std::endl;
    cout << "**提示：累计用时 " << time2 + time3 + time4 << " 毫秒\n";
    cout << "\n";
    if (!flag)
    {
        cout << "***************************************************\n";
        cout << "*   该用户名-口令对没有在本数据库中泄露！！！     *\n";
        cout << "***************************************************\n";
    }
    else {
        cout << "*************************************************************\n";
        cout << "**  该用户名-口令对在本数据库中泄露！！！请及时更换口令。  **\n";
        cout << "*************************************************************\n";
        cout << "\n";
    }
    return 0;
}