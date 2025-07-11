#include "tool.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR
#endif

using namespace std;

static const char* levelToStr(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default:              return "INFO";
    }
}

void logToFile(const std::string& msg, LogLevel level) {
    std::ofstream logFile("log.txt", std::ios::app);
    if (!logFile.is_open()) return;
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    logFile << "[" << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << "]";
    logFile << "[" << levelToStr(level) << "] ";
    logFile << msg << std::endl;
}

std::string base64Encode(const std::string& input) {
    std::string encoded;
    CryptoPP::StringSource ss(
        input, true,
        new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(encoded), false // false 表示不自动换行
        )
    );
    return encoded;
}

// 读取整个文件内容到字符串（支持Windows下UTF-8路径）
std::string readFileToString(const std::string& filePath, const std::string& encoding) {
    std::string content;
#ifdef _WIN32
    // 将UTF-8路径转为UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
    if (wlen <= 0) {
        std::cerr << "路径转码失败: " << filePath << std::endl;
        return "";
    }
    std::wstring wpath(wlen - 1, 0); // -1 去掉结尾\0
    MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &wpath[0], wlen);
    std::ifstream file(wpath, std::ios::binary);
#else
    std::ifstream file(filePath, std::ios::binary);
#endif
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();

    // 移除UTF-8 BOM
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content = content.substr(3);
    }

    if (encoding != "utf-8" && encoding != "UTF-8") {
        std::cerr << "注意：目前只支持直接读取UTF-8编码。对于其他编码，请先使用外部工具转换。" << std::endl;
    }
    return content;
}

// 将字符串内容写入文件（支持Windows下UTF-8路径）
bool writeStringToFile(const std::string& filePath, const std::string& content) {
#ifdef _WIN32
    int wlen = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
    if (wlen <= 0) {
        std::cerr << "路径转码失败: " << filePath << std::endl;
        return false;
    }
    std::wstring wpath(wlen - 1, 0); // -1 去掉结尾\0
    MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &wpath[0], wlen);
    std::ofstream file(wpath, std::ios::binary);
#else
    std::ofstream file(filePath, std::ios::binary);
#endif
    if (!file.is_open()) {
        std::cerr << "无法打开文件进行写入: " << filePath << std::endl;
        return false;
    }
    // 写入UTF-8 BOM
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    file.write(reinterpret_cast<const char*>(bom), 3);
    file << content;
    return file.good();
}

// 字符串全替换
std::string replaceAll(std::string str, const std::string &from, const std::string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

// 读取JSON配置文件
bool loadConfigJson(const fs::path& jsonFilePath, json& configJson) {
    if (!fs::exists(jsonFilePath)) {
        cerr << "错误: 无法找到文件 " << jsonFilePath.string() << endl;
        logToFile("错误: 无法找到文件 " + jsonFilePath.string(), LogLevel::ERROR);
        return false;
    }
    try {
        ifstream jsonFile(jsonFilePath);
        if (!jsonFile.is_open()) {
            cerr << "错误: 无法打开JSON文件" << endl;
            logToFile("错误: 无法打开JSON文件", LogLevel::ERROR);
            return false;
        }
        jsonFile >> configJson;
        jsonFile.close();
        cout << "成功读取JSON配置文件" << endl;
        logToFile("成功读取JSON配置文件: " + jsonFilePath.string(), LogLevel::INFO);
    } catch (json::parse_error &e) {
        cerr << "JSON解析错误: " << endl;
        logToFile(std::string("JSON解析错误: ") + e.what(), LogLevel::ERROR);
        return false;
    } catch (exception &e) {
        cerr << "读取JSON文件时发生错误: " << endl;
        logToFile(std::string("读取JSON文件时发生错误: ") + e.what(), LogLevel::ERROR);
        return false;
    }
    return true;
}

// URL解码函数
std::string urlDecode(const std::string& str) {
    std::string result;
    size_t i = 0;
    while (i < str.length()) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // 解析一个字节
            int value = 0;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 3;
            } else {
                // 非法转义，原样输出
                result += '%';
                i++;
            }
        } else if (str[i] == '+') {
            result += ' ';
            i++;
        } else {
            result += str[i];
            i++;
        }
    }
    return result;
}