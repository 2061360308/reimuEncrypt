#include <fstream>
#include <nlohmann/json.hpp>

#include "encryptConfig.h"

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