#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

// 替换配置
struct ReplaceConfig {
    bool innerHTML = true;
    std::string content;

    static std::optional<ReplaceConfig> fromJson(const nlohmann::json& j);
};

// 单个加密项
struct EncryptedItem {
    std::string name;
    std::string selector;
    bool selectAll = false;
    std::optional<ReplaceConfig> replace;
    std::string password;

    static EncryptedItem fromJson(const nlohmann::json& j);
};

// 单篇文章
struct ArticleItem {
    std::string title;
    std::string filePath;
    std::string uniqueID;
    std::string password;
    bool all = false;

    static ArticleItem fromJson(const nlohmann::json& j);
};

// 总配置
struct EncryptConfig {
    std::string generatedAt;
    int totalCount = 0;
    std::string defaultPassword;
    std::vector<EncryptedItem> encryptedAll;
    std::vector<EncryptedItem> encryptedPartial;
    std::vector<ArticleItem> articles;

    static EncryptConfig fromJson(const nlohmann::json& j);
};

// 加载加密配置文件
std::optional<EncryptConfig> loadEncryptConfig(const std::string& filePath);