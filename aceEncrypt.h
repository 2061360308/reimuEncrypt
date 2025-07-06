#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <fstream>
#include "base64.h"
#include <random>
#include <array>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/pwdbased.h>  // 包含PBKDF2
#include <cryptopp/sha.h>       // SHA256
#include <cryptopp/hmac.h>      // HMAC

// 辅助函数：打印十六进制数据
void printHex(const std::string& title, const uint8_t* data, size_t len);

// 保存加密数据为 Base64 编码文本
bool saveEncryptedDataAsBase64(const std::vector<uint8_t>& data, const std::string& filename);

// 从 Base64 编码文本加载加密数据
std::vector<uint8_t> loadEncryptedDataFromBase64(const std::string& filename);

std::string AesEncrypt(const std::string& plaintext, const std::string& key);

// AES解密函数 - 使用Crypto++
std::string AesDecrypt(const std::string& ciphertext, const std::string& key);