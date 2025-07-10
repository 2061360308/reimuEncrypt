#include <fstream>
#include <iostream>
#include <filesystem>
#include "aceEncrypt.h"
#include "praseHtml.h"
#include "tool.h"

using namespace std;

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

        // string content = getHtmlContent(articlePath.string(), selector);
        string htmlContent = readFileToString(articlePath.string());
        LexborDocument doc(htmlContent);
        auto root = doc.root();
        auto node = root->querySelector(selector);
        string content = node ? node->getContent() : "";

        try {
            if (encrypt && !content.empty()) {
                string encryptedContent = AesEncrypt(content, password);
                string encryptedBase64 = base64Encode(encryptedContent);
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

void testLexbor() {
    string html = "<html><body><div class='test'>Hello World</div></body></html>";
    LexborDocument doc(html);
    auto root = doc.root();
    auto node = root->querySelector(".test");
    if (node) {
        cout << "查询到内容: " << node->getContent() << endl;
    } else {
        cout << "未查询到内容" << endl;
    }
}

int main(int argc, char *argv[]) {
    testLexbor();

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
    fs::path templateJsPath = rootDir / fs::path(tpl);

    cout << "识别到站点根目录: " << rootDir.string() << endl;
    logToFile("识别到站点根目录: " + rootDir.string(), LogLevel::INFO);
    cout << "使用的模板文件: " << templateJsPath.string() << endl;
    logToFile("使用的模板文件: " + templateJsPath.string(), LogLevel::INFO);

    string templateJsContent = loadTemplateJs(templateJsPath);
    if (templateJsContent.empty()) return 1;

    // 主处理循环
    if (configJson.contains("encrypted") && configJson["encrypted"].is_array() &&
        configJson.contains("articles") && configJson["articles"].is_array()) {
        for (const auto &article : configJson["articles"]) {
            processArticle(article, configJson, rootDir, baseUrl, templateJsContent, defaultPassword);
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