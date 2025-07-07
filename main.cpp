#include <fstream>
#include <iostream>
#include <filesystem>
#include "aceEncrypt.h"
#include "praseHtml.h"
#include "nlohmann/json.hpp"
#include "tool.h"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

// 字符串全替换
std::string replaceAll(std::string str, const std::string &from, const std::string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

// 获取HTML内容
string getHtmlContent(const string& filePath, const string& selector) {
    string htmlContent = readFileToString(filePath);
    if (htmlContent.empty()) {
        cerr << "无法读取HTML文件或文件为空: " << filePath << endl;
        logToFile("无法读取HTML文件或文件为空: " + filePath, LogLevel::ERROR);
        return "";
    }
    string result = querySelector(htmlContent, selector, 0);
    logToFile("getHtmlContent: file=" + filePath + " selector=" + selector + " result.length=" + std::to_string(result.length()), LogLevel::DEBUG);
    return result;
}

// 解析命令行参数，确定配置文件路径和根目录
bool parseInputPath(int argc, char* argv[], fs::path& jsonFilePath) {
    if (argc < 2) {
        jsonFilePath = fs::current_path() / "encrypt.json";
        logToFile("未提供路径参数，在当前路径下查找encrypt.json", LogLevel::INFO);
        return true;
    }
    fs::path inputPath = argv[1];
    if (fs::is_directory(inputPath)) {
        jsonFilePath = inputPath / "encrypt.json";
        logToFile("在" + jsonFilePath.string() + "下查找encrypt.json", LogLevel::INFO);
        return true;
    }
    if (inputPath.extension() == ".json") {
        jsonFilePath = inputPath;
        cout << "加载配置文件: " << jsonFilePath.string() << endl;
        logToFile("加载配置文件: " + jsonFilePath.string(), LogLevel::INFO);
        return true;
    }
    cerr << "错误: 提供的路径只能是文件夹或*.json文件" << endl;
    logToFile("错误: 提供的路径只能是文件夹或*.json文件", LogLevel::ERROR);
    cerr << "用法: " << argv[0] << " [文件夹|json文件]" << endl;
    return false;
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

// 读取模板内容
string loadTemplate(const fs::path& templatePath) {
    if (!fs::exists(templatePath)) {
        cerr << "错误: 模板文件不存在: " << templatePath.string() << endl;
        logToFile("错误: 模板文件不存在: " + templatePath.string(), LogLevel::ERROR);
        return "";
    }
    logToFile("读取模板内容: " + templatePath.string(), LogLevel::DEBUG);
    return readFileToString(templatePath.string());
}

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

string getHtmlPath(string baseUrl, string permalink){
    // 删除permalink前的baseUrl部分
    permalink = replaceAll(permalink, baseUrl, "");
    // 去除前导斜杠
    if (!permalink.empty() && (permalink[0] == '/' || permalink[0] == '\\')) {
        permalink.erase(0, 1);
    }
    // 统一斜杠
    permalink = replaceAll(permalink, "\\", "/");

    // 判断最后一个字符是不是'/'
    if (!permalink.empty() && permalink.back() != '/') {
        permalink = permalink + "/index.html";
    } else {
        permalink = permalink + "index.html";
    }
    // 处理url解码
    permalink = urlDecode(permalink);

    std::ofstream f("test.txt", std::ios::binary);
    f << permalink;
    f.close();

    return permalink;
}

// 处理单篇文章
void processArticle(
    const json& article,
    const json& configJson,
    const fs::path& rootDir,
    const std::string& baseUrl,
    const std::string& templateContent,
    const std::string& defaultPassword
) {
    std::string title = article.value("title", "");
    std::string permalink = article.value("permalink", "");
    std::string password = article.value("password", defaultPassword);

    string filePath = getHtmlPath(baseUrl, permalink);
    fs::path articlePath = rootDir / fs::path(filePath);

    // fs::path articlePath = fs::path("E:\\hugo-reimu\\public\\post\\main---副本\\index.html");

    cout << "## 处理文章: " << title << ", 路径: " << articlePath.string() << endl;
    logToFile("处理文章: " + title + ", 路径: " + articlePath.string(), LogLevel::INFO);

    json data;
    for (const auto &item : configJson["encrypted"]) {
        std::string name = item.value("name", "");
        std::string selector = item.value("selector", "");
        bool encrypt = item.value("encrypt", false);

        cout << "处理配置项: name=" << name << ", selector=" << selector << ", encrypt=" << (encrypt ? "true" : "false") << endl;
        logToFile("处理配置项: name=" + name + ", selector=" + selector + ", encrypt=" + (encrypt ? "true" : "false"), LogLevel::DEBUG);

        if (name.empty()) {
            cerr << "警告: encrypted 配置项缺少 name 字段，已跳过。" << endl;
            logToFile("警告: encrypted 配置项缺少 name 字段，已跳过。", LogLevel::WARN);
            continue;
        }

        string content = getHtmlContent(articlePath.string(), selector);

        try {
            if (encrypt && !content.empty()) {
                string encryptedContent = AesEncrypt(content, password);
                string encryptedBase64 = base64_encode(
                    reinterpret_cast<const unsigned char*>(encryptedContent.data()),
                    encryptedContent.size());
                logToFile("加密内容: " + name + ", 内容(Base64前100): " + encryptedBase64.substr(0, 100), LogLevel::DEBUG);
                data[name] = encryptedBase64;
            } else {
                logToFile("不加密或内容为空: " + name + ", 内容: " + content.substr(0, 100), LogLevel::DEBUG);
                data[name] = content;
            }
        } catch (const std::exception &e) {
            cerr << "JSON赋值异常: " << e.what() << " name=" << name << endl;
            logToFile(std::string("JSON赋值异常: ") + e.what() + " name=" + name, LogLevel::ERROR);
        }
    }

    // 替换模板并写入文件
    string outputContent = templateContent;
    outputContent = replaceAll(outputContent, "__ENCRYPT_DATA__", data.dump());
    if (!writeStringToFile(articlePath.string(), outputContent)) {
        cerr << "写入文件失败: " << articlePath.string() << endl;
        logToFile("写入文件失败: " + articlePath.string(), LogLevel::ERROR);
    } else {
        cout << "已写入: " << articlePath.string() << endl;
        logToFile("已写入: " + articlePath.string(), LogLevel::INFO);
    }
}

/**
 * 删除加密配置文件
 */
bool removeEncryptConfigFile(fs::path jsonFilePath) {
    if (!fs::exists(jsonFilePath)) {
        cerr << "配置文件不存在: " << jsonFilePath.string() << endl;
        logToFile("配置文件不存在: " + jsonFilePath.string(), LogLevel::WARN);
        return false;
    }
    try {
        fs::remove(jsonFilePath);
        cout << "已删除加密配置文件: " << jsonFilePath.string() << endl;
        logToFile("已删除加密配置文件: " + jsonFilePath.string(), LogLevel::INFO);
        return true;
    } catch (const std::exception &e) {
        cerr << "删除配置文件失败: " << e.what() << endl;
        logToFile(std::string("删除配置文件失败: ") + e.what(), LogLevel::ERROR);
        return false;
    }
}

int main(int argc, char *argv[]) {

    logToFile("##### Hello reimuEncrypt #####", LogLevel::INFO);

    fs::path jsonFilePath, rootDir, baseURL;
    if (!parseInputPath(argc, argv, jsonFilePath)) return 1;

    cout << "当前使用的JSON配置文件: " << jsonFilePath.string() << endl;

    json configJson;
    if (!loadConfigJson(jsonFilePath, configJson)) return 1;

    // 读取基础配置
    std::string generatedAt = configJson.value("generatedAt", "");
    std::string defaultPassword = configJson.value("defaultPassword", "");
    std::string tpl = configJson.value("template", "");
    std::string _rootDir = configJson.value("rootDir", "");
    std::string baseUrl = configJson.value("baseURL", "");
    int totalCount = configJson.value("totalCount", 0);

    // 计算最终根目录
    rootDir = fs::absolute(jsonFilePath.parent_path() / fs::path(_rootDir));
    fs::path templatePath = rootDir / fs::path(tpl);

    cout << "识别到站点根目录: " << rootDir.string() << endl;
    logToFile("识别到站点根目录: " + rootDir.string(), LogLevel::INFO);
    cout << "使用的模板文件: " << templatePath.string() << endl;
    logToFile("使用的模板文件: " + templatePath.string(), LogLevel::INFO);

    string templateContent = loadTemplate(templatePath);
    if (templateContent.empty()) return 1;

    // 主处理循环
    if (configJson.contains("encrypted") && configJson["encrypted"].is_array() &&
        configJson.contains("articles") && configJson["articles"].is_array()) {
        for (const auto &article : configJson["articles"]) {
            processArticle(article, configJson, rootDir, baseUrl, templateContent, defaultPassword);
        }
    } else {
        cerr << "配置文件缺少 encrypted 或 articles 数组，或格式不正确。" << endl;
        logToFile("配置文件缺少 encrypted 或 articles 数组，或格式不正确。", LogLevel::ERROR);
        return 1;
    }

    removeEncryptConfigFile(jsonFilePath);

    logToFile("reimuEncrypt success exit", LogLevel::INFO);

    return 0;
}