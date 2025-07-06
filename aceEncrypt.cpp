// blog-encrypt.cpp: 定义应用程序的入口点。
//

#include "aceEncrypt.h"

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

// 保存加密数据为 Base64 编码文本
bool saveEncryptedDataAsBase64(const vector<uint8_t>& data, const string& filename) {
    ofstream file(filename);
    if (!file) {
        cerr << "无法打开文件进行写入: " << filename << endl;
        return false;
    }
    
    // 转换为 Base64
    string encoded = base64_encode(data.data(), data.size());
    file << encoded;
    
    return file.good();
}

// 从 Base64 编码文本加载加密数据
vector<uint8_t> loadEncryptedDataFromBase64(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "无法打开文件进行读取: " << filename << endl;
        return {};
    }
    
    // 读取整个文件内容
    string encoded((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    
    // Base64 解码
    string decoded = base64_decode(encoded);
    return vector<uint8_t>(decoded.begin(), decoded.end());
}

// 生成随机IV
std::string generateRandomIV() {
    // 使用随机设备作为种子
    std::random_device rd;
    // 使用梅森旋转算法引擎
    std::mt19937 gen(rd());
    // 均匀分布在 0 到 255 之间
    std::uniform_int_distribution<> dis(0, 255);
    
    // 创建一个 16 字节的数组
    std::array<char, 16> iv_array;
    
    // 填充随机数据
    for(int i = 0; i < 16; i++) {
        iv_array[i] = static_cast<char>(dis(gen));
    }
    
    // 转换为 string
    return std::string(iv_array.data(), iv_array.size());
}

// 修正 deriveKeyFromPassword 函数
std::string deriveKeyFromPassword(const std::string& password, std::string& salt_inout, unsigned int keyLength = 32) {
    // 如果未提供盐值，则生成新的随机盐
    if (salt_inout.empty()) {
        // 创建16字节随机盐
        salt_inout = generateRandomIV(); // 复用IV生成函数
    }

    // 使用PBKDF2算法
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
    
    // 关键参数：10000次迭代，这个值可以根据安全需要和性能平衡调整
    const unsigned int iterations = 10000;
    
    // 派生密钥
    CryptoPP::byte keyBuffer[64]; // 足够大的缓冲区
    pbkdf.DeriveKey(
        keyBuffer, keyLength, 0, 
        (const CryptoPP::byte*)password.data(), password.size(),
        (const CryptoPP::byte*)salt_inout.data(), salt_inout.size(),
        iterations
    );
    
    // 转换为string并返回
    return std::string(reinterpret_cast<char*>(keyBuffer), keyLength);
}

// 加密函数 - 使用密码派生密钥
std::string AesEncrypt(const std::string& plaintext, const std::string& password) {
    std::string ciphertext;

    // 生成随机盐值 (会在 deriveKeyFromPassword 中自动生成)
    std::string salt = "";
    
    // 从密码派生密钥
    std::string key = deriveKeyFromPassword(password, salt); // salt 在这里会被更新
    
    // 使用C++标准库生成随机IV
    const std::string iv = generateRandomIV();
    
    try {
        // 明确指定密钥长度为派生的密钥长度
        CryptoPP::AES::Encryption aesEncryption((CryptoPP::byte*)key.c_str(), key.size());
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, (CryptoPP::byte*)iv.c_str());
        
        CryptoPP::StringSource ss(plaintext, true, 
            new CryptoPP::StreamTransformationFilter(cbcEncryption,
                new CryptoPP::StringSink(ciphertext)
            )
        );
    }
    catch(const CryptoPP::Exception& e) {
        std::cerr << "加密错误: " << e.what() << std::endl;
    }
    
    // 将盐值和IV添加到密文前面，便于解密
    // 格式: [16字节盐值][16字节IV][密文]
    return salt + iv + ciphertext;
}

// 解密函数 - 使用密码和盐值派生密钥
std::string AesDecrypt(const std::string& encrypted, const std::string& password) {
    std::string decryptedtext;

    // 确保输入数据至少包含盐值和IV长度
    if (encrypted.length() < 32) {  // 16字节盐值 + 16字节IV
        std::cerr << "错误: 加密数据长度不足" << std::endl;
        return "";
    }

    // 从加密数据中提取盐值、IV和实际密文
    std::string salt = encrypted.substr(0, 16);
    const std::string iv = encrypted.substr(16, 16);
    std::string ciphertext = encrypted.substr(32);
    
    // 从密码和盐值派生密钥
    std::string key = deriveKeyFromPassword(password, salt);
    
    try {
        // 明确指定密钥长度为派生的密钥长度
        CryptoPP::AES::Decryption aesDecryption((CryptoPP::byte*)key.c_str(), key.size());
        CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, (CryptoPP::byte*)iv.c_str());
        
        CryptoPP::StringSource ss(ciphertext, true, 
            new CryptoPP::StreamTransformationFilter(cbcDecryption,
                new CryptoPP::StringSink(decryptedtext)
            )
        );
    }
    catch(const CryptoPP::Exception& e) {
        std::cerr << "解密错误: " << e.what() << std::endl;
    }
    
    return decryptedtext;
}