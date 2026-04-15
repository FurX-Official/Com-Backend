#include "config.h"
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace furbbs::config {

Config& Config::Instance() {
    static Config instance;
    return instance;
}

bool Config::Load(const std::string& config_file) {
    try {
        YAML::Node config = YAML::LoadFile(config_file);
        
        if (config["server"]) {
            ParseServer(config);
        }
        if (config["database"]) {
            ParseDatabase(config);
        }
        if (config["casdoor"]) {
            ParseCasdoor(config);
        }
        if (config["log"]) {
            ParseLog(config);
        }
        
        spdlog::info("Config loaded successfully from {}", config_file);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config: {}", e.what());
        return false;
    }
}

bool Config::ParseServer(const YAML::Node& config) {
    auto server = config["server"];
    server_.listen = server["listen"].as<std::string>();
    server_.thread_num = server["thread_num"].as<uint32_t>();
    server_.idle_time = server["idle_time"].as<uint32_t>();
    return true;
}

bool Config::ParseDatabase(const YAML::Node& config) {
    auto db = config["database"];
    database_.host = db["host"].as<std::string>();
    database_.port = db["port"].as<uint16_t>();
    database_.dbname = db["dbname"].as<std::string>();
    database_.user = db["user"].as<std::string>();
    database_.password = db["password"].as<std::string>();
    database_.max_connections = db["max_connections"].as<uint32_t>();
    return true;
}

bool Config::ParseCasdoor(const YAML::Node& config) {
    auto casdoor = config["casdoor"];
    casdoor_.endpoint = casdoor["endpoint"].as<std::string>();
    casdoor_.client_id = casdoor["client_id"].as<std::string>();
    casdoor_.client_secret = casdoor["client_secret"].as<std::string>();
    casdoor_.certificate = casdoor["certificate"].as<std::string>();
    casdoor_.org_name = casdoor["org_name"].as<std::string>();
    casdoor_.app_name = casdoor["app_name"].as<std::string>();
    casdoor_.redirect_uri = casdoor["redirect_uri"].as<std::string>();
    return true;
}

bool Config::ParseLog(const YAML::Node& config) {
    auto log = config["log"];
    log_.level = log["level"].as<std::string>();
    log_.file = log["file"].as<std::string>();
    log_.max_size = log["max_size"].as<uint32_t>();
    log_.max_files = log["max_files"].as<uint32_t>();
    return true;
}

} // namespace furbbs::config
