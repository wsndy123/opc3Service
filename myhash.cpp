#include "examples.h"

using namespace std;

std::string mysha256(const std::string &input)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return "";
    }

    const EVP_MD *md = EVP_sha256();
    if (!EVP_DigestInit_ex(ctx, md, nullptr))
    {
        std::cerr << "Failed to initialize digest" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (!EVP_DigestUpdate(ctx, input.c_str(), input.length()))
    {
        std::cerr << "Failed to update digest" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (!EVP_DigestFinal_ex(ctx, hash, &hash_len))
    {
        std::cerr << "Failed to finalize digest" << std::endl;
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hash_len; ++i)
    {
        oss << std::setw(2) << static_cast<int>(hash[i]); // 每8bit转换成2长的16进制数
    }

    return oss.str();
}