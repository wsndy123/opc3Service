// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include "seal/seal.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <bitset>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <ctime>
#include <cstdlib>
#include <stack>
#include <Eigen/Dense>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>

using namespace std;
using namespace Eigen;

#define paddinglength 10
#define uint64 unsigned long long
#define uint8 unsigned char
#define cokvsm 8192
#define cokvsr 256

string mysha256(const std::string &input);
unsigned long long hexToInt(const std::string &hexStr, int l);
uint64 randomint(uint64 t);

//map<string, int> ban;

int OPC3main(char *a, char *b, string username, string password, int bl);
int OPC3(char *a, char *b, string username, string password, int bl);

class OKVS
{
private:
    uint64 Fp = 4294967291; // 数域
    vector<uint64> L, R;

public:
    OKVS(){}
    array<uint64, 3> funlHASH(string s); // 返回3个位置，64以内
    vector<bool> funrBIT(string s); // 生成随机r比特向量
    uint64 inv(uint64 a); // 求逆
    void buildLR(vector<pair<string, int>> I, char *file);
    uint64 solve(uint64 startx, uint64 starty, vector<vector<uint64>> &A, vector<uint64> &B, int N, int M, uint64 MOD);
    vector<uint64> getans(vector<vector<uint64>> A, vector<uint64> B, uint64 N, uint64 M, uint64 MOD);
    uint64 Decode(string k);
    void checkGetLR(char* Sfile);
    void printLR();
    friend class BucketI;
};

class BucketI {
private:
    int maxcolumnsize; // 一列中的最大凭证数量
    int bucketsize; // 桶的大小，装了几个凭证
    vector<vector<string>> bucketi;
    vector<string> willdel;
    vector<pair<string, int>> I; // OKVS数据集
    OKVS Datastruct;

public:
    friend class Bucketset;
    BucketI(int column);
    BucketI() {}
    void push(string val);
    int Qbucketsize();
    void colpush(int col, string val);
    int Qmaxcolumnsize();
    int Qmincolumnsize();
    int Qbucketsize(bool T);
};

class Bucketset {
private:
    int Bucketidentifierlen; // 桶标识符长度
    int Bucketsize; // 平均桶大小
    int column;          // 每个桶有多少列
    vector<BucketI> bucket;

 public:
    Bucketset(char *file, int Bucketidentifierlen);
    Bucketset(char *file, int Bucketidentifierlen, int column);
    void Eachbucketsize();
    void Eachbucketmaxmin();
    int idlength() // 标识符长度
    {
        return Bucketidentifierlen;
    }
    int agvsize() // 桶的平均大小
    {
        uint64 tot = 0;
        for (auto& p : bucket)
        {
            tot += p.I.size();
        }
        return tot >> Bucketidentifierlen;
    }
    int MAXcolsize() // 所有桶的最大列大小
    {
        int MAX = 0;
        for (auto& p : bucket)
        {
            MAX = max(MAX, p.Qmaxcolumnsize());
        }
        return MAX;
    }
};

/*
Helper function: Prints the name of the example in a fancy banner.
*/
inline void print_example_banner(std::string title)
{
    if (!title.empty())
    {
        std::size_t title_length = title.length();
        std::size_t banner_length = title_length + 2 * 10;
        std::string banner_top = "+" + std::string(banner_length - 2, '-') + "+";
        std::string banner_middle = "|" + std::string(9, ' ') + title + std::string(9, ' ') + "|";

        std::cout << std::endl << banner_top << std::endl << banner_middle << std::endl << banner_top << std::endl;
    }
}

/*
Helper function: Prints the parameters in a SEALContext.
*/
inline void print_parameters(std::shared_ptr<seal::SEALContext> context)
{
    // Verify parameters
    if (!context)
    {
        throw std::invalid_argument("context is not set");
    }
    auto &context_data = *context->key_context_data(); // 关键信息数据

    /*
    Which scheme are we using?
    */
    std::string scheme_name;
    switch (context_data.parms().scheme())
    {
    case seal::scheme_type::BFV:
        scheme_name = "BFV";
        break;
    case seal::scheme_type::CKKS:
        scheme_name = "CKKS";
        break;
    default:
        throw std::invalid_argument("unsupported scheme");
    }
    std::cout << "/" << std::endl;
    std::cout << "| Encryption parameters :" << std::endl;
    std::cout << "|   scheme: " << scheme_name << std::endl;
    std::cout << "|   poly_modulus_degree: " << context_data.parms().poly_modulus_degree() << std::endl;

    /*
    Print the size of the true (product) coefficient modulus.
    */
    std::cout << "|   coeff_modulus size: ";
    std::cout << context_data.total_coeff_modulus_bit_count() << " ("; // 系数模一共bit
    auto coeff_modulus = context_data.parms().coeff_modulus();
    std::size_t coeff_modulus_size = coeff_modulus.size(); // 109 bit的系数模分成多少个素数
    for (std::size_t i = 0; i < coeff_modulus_size - 1; i++) // 遍历每一个素数输出bit数量
    {
        std::cout << coeff_modulus[i].bit_count() << " + "; // 每个coeff_modulus[i]是一个Modulus类
    }
    std::cout << coeff_modulus.back().bit_count();
    std::cout << ") bits" << std::endl;

    /*
    For the BFV scheme print the plain_modulus parameter.
    */
    if (context_data.parms().scheme() == seal::scheme_type::BFV)
    {
        std::cout << "|   plain_modulus: " << context_data.parms().plain_modulus().value() << std::endl;
    }

    std::cout << "\\" << std::endl;
}

/*
Helper function: Prints the `parms_id' to std::ostream.
*/
inline std::ostream &operator<<(std::ostream &stream, seal::parms_id_type parms_id)
{
    /*
    Save the formatting information for std::cout.
    */
    std::ios old_fmt(nullptr);
    old_fmt.copyfmt(std::cout);

    stream << std::hex << std::setfill('0') << std::setw(16) << parms_id[0] << " " << std::setw(16) << parms_id[1]
           << " " << std::setw(16) << parms_id[2] << " " << std::setw(16) << parms_id[3] << " ";

    /*
    Restore the old std::cout formatting.
    */
    std::cout.copyfmt(old_fmt);

    return stream;
}

/*
Helper function: Prints a vector of floating-point values.
*/
template <typename T>
inline void print_vector(std::vector<T> vec, std::size_t print_size = 4, int prec = 3)
{
    /*
    Save the formatting information for std::cout.
    */
    std::ios old_fmt(nullptr);
    old_fmt.copyfmt(std::cout);

    std::size_t slot_count = vec.size();

    std::cout << std::fixed << std::setprecision(prec);
    std::cout << std::endl;
    if (slot_count <= 2 * print_size)
    {
        std::cout << "    [";
        for (std::size_t i = 0; i < slot_count; i++)
        {
            std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    else
    {
        vec.resize(std::max(vec.size(), 2 * print_size));
        std::cout << "    [";
        for (std::size_t i = 0; i < print_size; i++)
        {
            std::cout << " " << vec[i] << ",";
        }
        if (vec.size() > 2 * print_size)
        {
            std::cout << " ...,";
        }
        for (std::size_t i = slot_count - print_size; i < slot_count; i++)
        {
            std::cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
        }
    }
    std::cout << std::endl;

    /*
    Restore the old std::cout formatting.
    */
    std::cout.copyfmt(old_fmt);
}

/*
Helper function: Prints a matrix of values.
*/
template <typename T>
inline void print_matrix(std::vector<T> matrix, std::size_t row_size)
{
    /*
    We're not going to print every column of the matrix (there are 2048). Instead
    print this many slots from beginning and end of the matrix.
    */
    std::size_t print_size = 5;

    std::cout << std::endl;
    std::cout << "    [";
    for (std::size_t i = 0; i < print_size; i++)
    {
        std::cout << std::setw(3) << std::right << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = row_size - print_size; i < row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
    }
    std::cout << "    [";
    for (std::size_t i = row_size; i < row_size + print_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = 2 * row_size - print_size; i < 2 * row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
    }
    std::cout << std::endl;
}

/*
Helper function: Print line number.
*/
inline void print_line(int line_number)
{
    std::cout << "Line " << std::setw(3) << line_number << " --> ";
}
