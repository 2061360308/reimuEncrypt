#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>

#include "aceEncrypt.h"
#include "tool.h"

using namespace std;

#define CBC 1
#define AES256 1

// 辅助函数：打印十六进制数据
void printHex(const string& title, const uint8_t* data, size_t len) {
    cout << title << ": ";
    for (size_t i = 0; i < len; i++) {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(data[i]) << " ";
    }
    cout << dec << endl;
}

// 生成随机IV
std::string generateRandomIV() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::array<char, 16> iv_array;
    for(int i = 0; i < 16; i++) {
        iv_array[i] = static_cast<char>(dis(gen));
    }
    logToFile("生成随机IV", LogLevel::DEBUG);
    return std::string(iv_array.data(), iv_array.size());
}

// 修正 deriveKeyFromPassword 函数
std::string deriveKeyFromPassword(const std::string& password, std::string& salt_inout, unsigned int keyLength = 32) {
    if (salt_inout.empty()) {
        salt_inout = generateRandomIV();
        logToFile("生成随机盐值", LogLevel::DEBUG);
    }
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
    const unsigned int iterations = 10000;
    CryptoPP::byte keyBuffer[64];
    pbkdf.DeriveKey(
        keyBuffer, keyLength, 0, 
        (const CryptoPP::byte*)password.data(), password.size(),
        (const CryptoPP::byte*)salt_inout.data(), salt_inout.size(),
        iterations
    );
    logToFile("从密码派生密钥，迭代次数: " + std::to_string(iterations), LogLevel::DEBUG);
    return std::string(reinterpret_cast<char*>(keyBuffer), keyLength);
}

// 加密函数 - 使用密码派生密钥
std::string AesEncrypt(const std::string& plaintext, const std::string& password) {
    std::string ciphertext;
    std::string salt = "";
    std::string key = deriveKeyFromPassword(password, salt);
    const std::string iv = generateRandomIV();
    logToFile("开始加密数据", LogLevel::INFO);
    try {
        CryptoPP::AES::Encryption aesEncryption((CryptoPP::byte*)key.c_str(), key.size());
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, (CryptoPP::byte*)iv.c_str());
        CryptoPP::StringSource ss(plaintext, true, 
            new CryptoPP::StreamTransformationFilter(cbcEncryption,
                new CryptoPP::StringSink(ciphertext)
            )
        );
        logToFile("加密成功", LogLevel::INFO);
    }
    catch(const CryptoPP::Exception& e) {
        cerr << "加密错误" << endl;
        logToFile(std::string("加密错误: ") + e.what(), LogLevel::ERROR);
    }
    return salt + iv + ciphertext;
}

// 解密函数 - 使用密码和盐值派生密钥
std::string AesDecrypt(const std::string& encrypted, const std::string& password) {
    std::string decryptedtext;
    if (encrypted.length() < 32) {
        cerr << "错误: 加密数据长度不足" << endl;
        logToFile("加密数据长度不足", LogLevel::ERROR);
        return "";
    }
    std::string salt = encrypted.substr(0, 16);
    const std::string iv = encrypted.substr(16, 16);
    std::string ciphertext = encrypted.substr(32);
    std::string key = deriveKeyFromPassword(password, salt);
    logToFile("开始解密数据", LogLevel::INFO);
    try {
        CryptoPP::AES::Decryption aesDecryption((CryptoPP::byte*)key.c_str(), key.size());
        CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, (CryptoPP::byte*)iv.c_str());
        CryptoPP::StringSource ss(ciphertext, true, 
            new CryptoPP::StreamTransformationFilter(cbcDecryption,
                new CryptoPP::StringSink(decryptedtext)
            )
        );
        logToFile("解密成功", LogLevel::INFO);
    }
    catch(const CryptoPP::Exception& e) {
        cerr << "解密错误" << endl;
        logToFile(std::string("解密错误: ") + e.what(), LogLevel::ERROR);
    }
    return decryptedtext;
}