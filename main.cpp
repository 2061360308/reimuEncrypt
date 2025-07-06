#include <fstream>
#include <iostream>
#include <filesystem>

#include "aceEncrypt.h"
#include "prase.h"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    // 确定encrypted.json文件的路径
    fs::path jsonFilePath;
    fs::path rootDir;
    
    if (argc < 2) {
        // 没有提供参数，使用当前路径下的encrypted.json
        jsonFilePath = fs::current_path() / "encrypted.json";
        cout << "未提供路径参数，在当前路径下查找encrypted.json" << endl;
    } else {
        // 提供了路径参数
        fs::path inputPath = argv[1];
        
        if (fs::is_directory(inputPath)) {
            // 如果是文件夹，在该文件夹下找encrypted.json
            jsonFilePath = inputPath / "encrypted.json";
            cout << "在" << jsonFilePath.string() << "下查找encrypted.json" << endl;
        } else if (inputPath.extension() == ".json") {
            // 如果直接给了json文件，直接使用
            jsonFilePath = inputPath;
            cout << "加载配置文件: " << jsonFilePath.string() << endl;
        } else {
            // 提供的既不是文件夹也不是json文件
            cout << "错误: 提供的路径只能是文件夹或*.json文件" << endl;
            cout << "用法: " << argv[0] << " [文件夹|json文件]" << endl;
            return 1;
        }
    }

    rootDir = jsonFilePath.parent_path();
    
    cout << "当前使用的JSON配置文件: " << jsonFilePath.string() << endl;
    cout << "识别到站点根目录: " << rootDir.string() << endl;
    
    // 检查JSON文件是否存在
    if (!fs::exists(jsonFilePath)) {
        cout << "错误: 无法找到文件 " << jsonFilePath.string() << endl;
        return 1;
    }

    

    cout << "AES-CBC 加密解密示例" << endl;
    cout << "====================" << endl;
    
    // 待加密的消息
    string message = u8"<div class=\"markdown-body\"><h1>标题</h1><p>这是一个段落。</p></div>";
    cout << "原始消息: " << message << endl;
    
    // 用于加密的密钥 - 必须是16字节(128位)、24字节(192位)或32字节(256位)
    // 这里使用16字节的密钥 (AES-128)
    string key = "020218";

    // 加密
    string encrypted = AesEncrypt(message, key);
    cout << "\n加密后的数据长度: " << encrypted.length() << " 字节" << endl;

    cout << encrypted << endl;
    
    // 将加密后的数据转换为Base64以便于显示
    string base64Encoded = base64_encode((const unsigned char*)encrypted.c_str(), encrypted.length());
    cout << "加密后的数据(Base64): " << base64Encoded << endl;
    
    // 保存加密后的数据
    vector<uint8_t> encryptedVector(encrypted.begin(), encrypted.end());
    if (saveEncryptedDataAsBase64(encryptedVector, "encrypted.txt")) {
        cout << "\n加密数据已保存到 encrypted.txt" << endl;
    }
    
    // 解密
    string decrypted = AesDecrypt(encrypted, key);
    cout << "\n解密后的消息: " << decrypted << endl;
    
    // 从文件中加载并解密
    cout << "\n从文件加载并解密:" << endl;
    vector<uint8_t> loadedData = loadEncryptedDataFromBase64("encrypted.txt");
    if (!loadedData.empty()) {
        string loadedEncrypted(loadedData.begin(), loadedData.end());
        string loadedDecrypted = AesDecrypt(loadedEncrypted, key);
        cout << "从文件解密的消息: " << loadedDecrypted << endl;
    }

    // 读取index.html并获取article元素
    cout << "\n读取index.html并获取article元素:" << endl;
    string htmlContent = readFileToString("E:\\blog-encrypt\\index.html");

    cout << "读取的HTML内容长度: " << htmlContent.length() << " 字符" << endl;

    cout << "HTML内容预览: " << endl;
    cout << htmlContent.substr(0, 600) << "..." << endl;
    
    if (!htmlContent.empty()) {
        // 查找所有article元素
        string article = querySelector(htmlContent, "article", 0);
        
        writeStringToFile("article.txt", article);
    } else {
        cout << "无法读取index.html文件或文件为空" << endl;
    }

    return 0;
}