#include "examples.h"

unsigned long long hexToInt(const std::string &hexStr, int l) { // 返回16进制字面字符串的前l比特的整数值
    uint64 num = 0;
    for (char c : hexStr) {
        if (l == 0)
        {
            break;
        }
        if (l >= 4) {
            char str[2] = { c, '\0' };
            num <<= 4;
            num |= static_cast<int>(std::strtol(str, nullptr, 16));
            l -= 4;
        }
        else
        {
            char str[2] = { c, '\0' };
            num <<= l;
            num |= (static_cast<int>(std::strtol(str, nullptr, 16))) >> ( 4 - l);
            break;
        }
    }
    return num;
}

uint64 randomint(uint64 t)
{
    std::random_device rd;                                                // 使用随机设备作为种子
    std::seed_seq seed{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() }; // 使用多个随机设备值作为种子序列
    std::mt19937 gen(seed);                                               // 使用Mersenne Twister算法
    std::uniform_int_distribution<> dis(1, t - 1);                          // 生成1到100之间的随机数

    return dis(gen);

}
