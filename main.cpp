#include <fstream>
#include <iostream>
#include <filesystem>
#include "aceEncrypt.h"
#include "praseHtml.h"
#include "tool.h"
#include "encryptConfig.h"

using namespace std;

EncryptConfig config;  // 全局加密配置对象
fs::path jsonFilePath, rootDir;  // 配置文件路径、根目录

// 解析命令行参数，确定配置文件路径和根目录
bool parseInputPath(int argc, char* argv[], fs::path& jsonFilePath, fs::path& rootDir) {
    if (argc < 2) {
        jsonFilePath = fs::current_path() / "encrypt.json";
        logToFile("未提供路径参数，在当前路径下查找encrypt.json", LogLevel::INFO);
        rootDir = fs::current_path();
        return true;
    }
    fs::path inputPath = argv[1];
    if (fs::is_directory(inputPath)) {
        jsonFilePath = inputPath / "encrypt.json";
        logToFile("在" + jsonFilePath.string() + "下查找encrypt.json", LogLevel::INFO);
        rootDir = inputPath;
        return true;
    }
    if (inputPath.extension() == ".json") {
        jsonFilePath = inputPath;
        cout << "加载配置文件: " << jsonFilePath.string() << endl;
        logToFile("加载配置文件: " + jsonFilePath.string(), LogLevel::INFO);
        rootDir = inputPath.parent_path();
        return true;
    }
    cerr << "错误: 提供的路径只能是文件夹或*.json文件" << endl;
    logToFile("错误: 提供的路径只能是文件夹或*.json文件", LogLevel::ERROR);
    cerr << "用法: " << argv[0] << " [文件夹|json文件]" << endl;
    return false;
}

// 读取模板内容
string loadTemplateJs(const fs::path& templateJsPath) {
    if (!fs::exists(templateJsPath)) {
        cerr << "错误: 模板文件不存在: " << templateJsPath.string() << endl;
        logToFile("错误: 模板文件不存在: " + templateJsPath.string(), LogLevel::ERROR);
        return "";
    }
    logToFile("读取模板内容: " + templateJsPath.string(), LogLevel::DEBUG);
    return readFileToString(templateJsPath.string());
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

void processAllArticles(){}

void processPartialArticles(){}

string processNode(string defaultPassword,
                const std::shared_ptr<LexborNode> &node,
                const EncryptedItem &item,
                std::shared_ptr<LexborNode> docRoot, 
                nlohmann::json &result) {
    
    string password, encryptedBase64;
    if (item.password.empty()) {
        password = defaultPassword;
    } else {
        password = node->querySelector(item.password)->getContent();
        if (password.empty()) {
            password = defaultPassword;
        }
    }
    string content = node->getContent();
    if (!content.empty()) {
        string encryptedContent = AesEncrypt(content, password);
        encryptedBase64 = base64Encode(encryptedContent);
        logToFile("加密内容: " + item.name + ", 内容(Base64前100): " + encryptedBase64.substr(0, 100), LogLevel::DEBUG);
    } else {
        encryptedBase64 = "";
        logToFile("内容为空无法加密: " + item.name + ", 内容: " + content.substr(0, 100), LogLevel::DEBUG);
    }

    if (item.replace && item.replace->innerHTML) {
        // 替换节点内容
        node->setInnerHtml(item.replace->content);
        logToFile("InnerHtml替换: " + item.name + ", 内容: " + item.replace->content.substr(0, 100), LogLevel::DEBUG);
    } else if (item.replace) {
        // 替换节点外部HTML
        node->setOuterHtml(item.replace->content);
        logToFile("OuterHtml替换: " + item.name + ", 内容: " + item.replace->content.substr(0, 100), LogLevel::DEBUG);
    }

    return encryptedBase64;
}

// 处理单篇文章
void processArticle(
    const ArticleItem &article
) {
    // 定义变量保存文章加密结果
    nlohmann::json result;

    fs::path filePath = rootDir / fs::path(article.filePath);

    // 加载文章
    auto doc = LexborDocument::fromFile(filePath.string());
    if (!doc) {
        std::cout << "打开文件失败: " << filePath.string() << std::endl;
        return;
    }
    auto docRoot = doc->root();

    // 根据配置选取加密配置（整篇/局部）
    std::vector<EncryptedItem> encrypt;
    if (article.all) {
        encrypt = config.encryptedAll;
    } else {
        encrypt = config.encryptedPartial;
    }

    for (const auto &item : encrypt) {
        cout << "处理加密配置: name=" << item.name
             << ", selector=" << item.selector
             << ", replace=" << (item.replace ? "true" : "false")
             << ", selectAll=" << (item.selectAll ? "true" : "false")
             << ", password=" << item.password
             << endl;
        logToFile("处理加密配置: name=" + item.name +
                  ", selector=" + item.selector +
                  ", replace=" + (item.replace ? "true" : "false") +
                  ", selectAll=" + (item.selectAll ? "true" : "false") +
                  ", password=" + item.password, LogLevel::DEBUG);
        
        // 设置默认密码
        string defaultPassword = config.defaultPassword;
        if (article.password.empty()) {
            defaultPassword = config.defaultPassword;
        }

        if (item.selectAll){
            std::vector<std::shared_ptr<LexborNode>> nodes = docRoot->querySelectorAll(item.selector);
            for (const std::shared_ptr<LexborNode> &node : nodes) {
                result[item.name].push_back(processNode(defaultPassword, node, item, docRoot, result));
            }
        } else {
            std::shared_ptr<LexborNode> node = docRoot->querySelector(item.selector);
            result[item.name] = processNode(defaultPassword, node, item, docRoot, result);
        }
    }

    // 写入加密数据到文件
    auto headNode = docRoot->querySelector("head");
    if (headNode) {
        headNode->appendHtml("<script>var __ENCRYPT_DATA__ = " + result.dump() + ";</script>");
    } else {
        cerr << "未找到<head>节点，无法写入加密数据。" << endl;
        logToFile("未找到<head>节点，无法写入加密数据。", LogLevel::ERROR);
        return;
    }
    // 写出文件
    if (!writeStringToFile(filePath.string(), docRoot->getContent())) {
        cerr << "写入文件失败: " << filePath.string() << endl;
        logToFile("写入文件失败: " + filePath.string(), LogLevel::ERROR);
    } else {
        cout << "已写入: " << filePath.string() << endl;
        logToFile("已写入: " + filePath.string(), LogLevel::INFO);
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

    if (!parseInputPath(argc, argv, jsonFilePath, rootDir)) return 1;

    auto configOpt = loadEncryptConfig(jsonFilePath.string());

    if (configOpt) {
        config = *configOpt;
    } else {
        cerr << "读取加密配置失败，请检查配置文件格式。" << endl;
        logToFile("读取加密配置失败，请检查配置文件格式。", LogLevel::ERROR);
        return 1;
    }

    cout << "当前使用的JSON配置文件: " << jsonFilePath.string() << endl;

    // 循环处理Articles
    for (const auto &article : config.articles) {
        processArticle(article);
    }

    removeEncryptConfigFile(jsonFilePath);

    logToFile("reimuEncrypt success exit", LogLevel::INFO);

    return 0;
}