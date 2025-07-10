#pragma once

#include <string>
#include <array>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/pwdbased.h>  // PBKDF2
#include <cryptopp/sha.h>       // SHA256

// 辅助函数：打印十六进制数据
void printHex(const std::string& title, const uint8_t* data, size_t len);

std::string AesEncrypt(const std::string& plaintext, const std::string& key);

// AES解密函数 - 使用Crypto++
std::string AesDecrypt(const std::string& ciphertext, const std::string& key);