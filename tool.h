#pragma once
#include <string>
#include <filesystem>
#include <cryptopp/base64.h>
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

/**
 * 将日志消息写入文件
 * 
 * @param msg 日志消息
 * @param level 日志级别，默认为INFO
 */
void logToFile(const std::string& msg, LogLevel level = LogLevel::INFO);


std::string base64Encode(const std::string& input);


/**
 * 读取整个文件内容到字符串
 * 
 * @param filePath 文件路径
 * @param encoding 文件编码 (如 "utf-8", "gbk", "gb2312" 等)
 * @return 文件内容字符串
 */
std::string readFileToString(const std::string& filePath, const std::string& encoding = "utf-8");

/**
 * 将字符串内容写入文件
 * 
 * @param filePath 文件路径
 * @param content 要写入的内容
 * @return 操作是否成功
 */
bool writeStringToFile(const std::string& filePath, const std::string& content);
