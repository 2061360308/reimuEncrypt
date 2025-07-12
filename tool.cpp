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

// 判断是否为指定的 UTF-8 空白字符
inline bool is_utf8_space(uint32_t cp) {
    // ASCII 空白
    if (cp == 0x09 || cp == 0x0A || cp == 0x0B || cp == 0x0C || cp == 0x0D || cp == 0x20) return true;
    // 不换行空格
    if (cp == 0xA0) return true;
    // 欧甘字母空格
    if (cp == 0x1680) return true;
    // 全角空格
    if (cp == 0x3000) return true;
    return false;
}

// 支持常见 UTF-8 空白的 trim
std::string trim(const std::string& str) {
    size_t len = str.size();
    size_t first = 0, last = len;

    // 前向扫描
    while (first < len) {
        unsigned char c = str[first];
        uint32_t cp = 0;
        size_t char_len = 1;
        if (c < 0x80) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0 && first + 1 < len) {
            cp = ((c & 0x1F) << 6) | (str[first+1] & 0x3F);
            char_len = 2;
        } else if ((c & 0xF0) == 0xE0 && first + 2 < len) {
            cp = ((c & 0x0F) << 12) | ((str[first+1] & 0x3F) << 6) | (str[first+2] & 0x3F);
            char_len = 3;
        } else if ((c & 0xF8) == 0xF0 && first + 3 < len) {
            cp = ((c & 0x07) << 18) | ((str[first+1] & 0x3F) << 12) | ((str[first+2] & 0x3F) << 6) | (str[first+3] & 0x3F);
            char_len = 4;
        }
        if (!is_utf8_space(cp)) break;
        first += char_len;
    }

    // 反向扫描
    while (last > first) {
        size_t i = last;
        // 找到前一个 codepoint 的起始
        do { --i; } while (i > first && (str[i] & 0xC0) == 0x80);
        unsigned char c = str[i];
        uint32_t cp = 0;
        size_t char_len = last - i;
        if (c < 0x80) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0 && char_len == 2) {
            cp = ((c & 0x1F) << 6) | (str[i+1] & 0x3F);
        } else if ((c & 0xF0) == 0xE0 && char_len == 3) {
            cp = ((c & 0x0F) << 12) | ((str[i+1] & 0x3F) << 6) | (str[i+2] & 0x3F);
        } else if ((c & 0xF8) == 0xF0 && char_len == 4) {
            cp = ((c & 0x07) << 18) | ((str[i+1] & 0x3F) << 12) | ((str[i+2] & 0x3F) << 6) | (str[i+3] & 0x3F);
        }
        if (!is_utf8_space(cp)) break;
        last = i;
    }

    return str.substr(first, last - first);
}

// URL 解码（percent-decode），返回 UTF-8 字符串
std::string decodeUrl(const std::string& str) {
    std::string result;
    size_t i = 0;
    while (i < str.size()) {
        char ch = str[i];
        if (ch == '%') {
            if (i + 2 < str.size() && std::isxdigit(str[i + 1]) && std::isxdigit(str[i + 2])) {
                // 解析两个十六进制数字
                int high = std::toupper(str[i + 1]);
                int low  = std::toupper(str[i + 2]);
                high = high > '9' ? high - 'A' + 10 : high - '0';
                low  = low  > '9' ? low  - 'A' + 10 : low  - '0';
                char decoded = static_cast<char>((high << 4) | low);
                result += decoded;
                i += 3;
            } else {
                // 非法的 % 编码，原样输出
                result += '%';
                ++i;
            }
        } else if (ch == '+') {
            result += ' '; // 按 application/x-www-form-urlencoded 规则
            ++i;
        } else {
            result += ch;
            ++i;
        }
    }
    return result;
}