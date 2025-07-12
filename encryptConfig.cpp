#include <fstream>
#include <nlohmann/json.hpp>

#include "tool.h"
#include "encryptConfig.h"

std::optional<ReplaceConfig> ReplaceConfig::fromJson(const nlohmann::json& j) {
    if (j.is_null()) return std::nullopt;
    ReplaceConfig rc;
    if (j.contains("innerHTML")) rc.innerHTML = j["innerHTML"].get<bool>();
    if (j.contains("content")) rc.content = j["content"].get<std::string>();
    return rc;
}

EncryptedItem EncryptedItem::fromJson(const nlohmann::json& j) {
    EncryptedItem item;
    if (j.contains("name")) item.name = j["name"].get<std::string>();
    if (j.contains("selector")) item.selector = j["selector"].get<std::string>();
    if (j.contains("selectAll")) item.selectAll = j["selectAll"].get<bool>();
    if (j.contains("replace")) item.replace = ReplaceConfig::fromJson(j["replace"]);
    if (j.contains("password")) item.password = j["password"].get<std::string>();
    return item;
}

ArticleItem ArticleItem::fromJson(const nlohmann::json& j) {
    ArticleItem item;
    if (j.contains("title")) item.title = j["title"].get<std::string>();
    if (j.contains("filePath")) item.filePath = j["filePath"].get<std::string>();
    item.filePath = decodeUrl(item.filePath); // 文件路径可能需要进行 URL 解码
    if (j.contains("uniqueID")) item.uniqueID = j["uniqueID"].get<std::string>();
    if (j.contains("password")) item.password = j["password"].get<std::string>();
    if (j.contains("all")) item.all = j["all"].get<bool>();
    return item;
}

EncryptConfig EncryptConfig::fromJson(const nlohmann::json& j) {
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

std::optional<EncryptConfig> loadEncryptConfig(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        return std::nullopt;
    }
    nlohmann::json j;
    try {
        in >> j;
    } catch (...) {
        return std::nullopt;
    }
    return EncryptConfig::fromJson(j);
}