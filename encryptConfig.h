#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

// 替换配置
struct ReplaceConfig {
    bool innerHTML = true;
    std::string content;

    static std::optional<ReplaceConfig> fromJson(const nlohmann::json& j) {
        if (j.is_null()) return std::nullopt;
        ReplaceConfig rc;
        if (j.contains("innerHTML")) rc.innerHTML = j["innerHTML"].get<bool>();
        if (j.contains("content")) rc.content = j["content"].get<std::string>();
        return rc;
    }
};

// 单个加密项
struct EncryptedItem {
    std::string name;
    std::string selector;
    bool selectAll = false;
    std::optional<ReplaceConfig> replace;
    std::string password;

    static EncryptedItem fromJson(const nlohmann::json& j) {
        EncryptedItem item;
        if (j.contains("name")) item.name = j["name"].get<std::string>();
        if (j.contains("selector")) item.selector = j["selector"].get<std::string>();
        if (j.contains("selectAll")) item.selectAll = j["selectAll"].get<bool>();
        if (j.contains("replace")) item.replace = ReplaceConfig::fromJson(j["replace"]);
        if (j.contains("password")) item.password = j["password"].get<std::string>();
        return item;
    }
};

// 单篇文章
struct ArticleItem {
    std::string title;
    std::string filePath;
    std::string uniqueID;
    std::string password;
    bool all = false;

    static ArticleItem fromJson(const nlohmann::json& j) {
        ArticleItem item;
        if (j.contains("title")) item.title = j["title"].get<std::string>();
        if (j.contains("filePath")) item.filePath = j["filePath"].get<std::string>();
        if (j.contains("uniqueID")) item.uniqueID = j["uniqueID"].get<std::string>();
        if (j.contains("password")) item.password = j["password"].get<std::string>();
        if (j.contains("all")) item.all = j["all"].get<bool>();
        return item;
    }
};

// 总配置
struct EncryptConfig {
    std::string generatedAt;
    int totalCount = 0;
    std::string defaultPassword;
    std::vector<EncryptedItem> encryptedAll;  // 文章整体加密配置项
    std::vector<EncryptedItem> encryptedPartial;  // 文章局部加密配置项
    std::vector<ArticleItem> articles;

    static EncryptConfig fromJson(const nlohmann::json& j) {
        EncryptConfig cfg;
        if (j.contains("generatedAt")) cfg.generatedAt = j["generatedAt"].get<std::string>();
        if (j.contains("totalCount")) cfg.totalCount = j["totalCount"].get<int>();
        if (j.contains("defaultPassword")) cfg.defaultPassword = j["defaultPassword"].get<std::string>();
        
        if (j.contains("encrypted-all")) {
            for (const auto& item : j["encrypted-all"]) {
                cfg.encryptedAll.push_back(EncryptedItem::fromJson(item));
            }
        }
        if (j.contains("encrypted-partial")) {
            for (const auto& item : j["encrypted-partial"]) {
                cfg.encryptedPartial.push_back(EncryptedItem::fromJson(item));
            }
        }
        if (j.contains("articles")) {
            for (const auto& item : j["articles"]) {
                cfg.articles.push_back(ArticleItem::fromJson(item));
            }
        }
        return cfg;
    }
};

// 加载加密配置文件
std::optional<EncryptConfig> loadEncryptConfig(const std::string& filePath);