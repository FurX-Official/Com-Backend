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
        if (config["security"]) {
            ParseSecurity(config);
        }
        if (config["cors"]) {
            ParseCors(config);
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

bool Config::ParseSecurity(const YAML::Node& config) {
    auto sec = config["security"];
    security_.enable_rate_limit = sec["enable_rate_limit"].as<bool>(true);
    security_.rate_limit_requests = sec["rate_limit_requests"].as<uint32_t>(100);
    security_.rate_limit_window = sec["rate_limit_window"].as<uint32_t>(60);
    security_.enable_xss_protection = sec["enable_xss_protection"].as<bool>(true);
    security_.enable_sql_protection = sec["enable_sql_protection"].as<bool>(true);
    security_.jwt_expiry_hours = sec["jwt_expiry_hours"].as<uint32_t>(24);
    return true;
}

bool Config::ParseCors(const YAML::Node& config) {
    auto cors = config["cors"];
    cors_.enabled = cors["enabled"].as<bool>(true);
    
    if (cors["allowed_origins"] && cors["allowed_origins"].IsSequence()) {
        for (const auto& origin : cors["allowed_origins"]) {
            cors_.allowed_origins.push_back(origin.as<std::string>());
        }
    }
    
    if (cors["allowed_methods"] && cors["allowed_methods"].IsSequence()) {
        for (const auto& method : cors["allowed_methods"]) {
            cors_.allowed_methods.push_back(method.as<std::string>());
        }
    }
    
    if (cors["allowed_headers"] && cors["allowed_headers"].IsSequence()) {
        for (const auto& header : cors["allowed_headers"]) {
            cors_.allowed_headers.push_back(header.as<std::string>());
        }
    }
    
    if (cors["expose_headers"] && cors["expose_headers"].IsSequence()) {
        for (const auto& header : cors["expose_headers"]) {
            cors_.expose_headers.push_back(header.as<std::string>());
        }
    }
    
    cors_.allow_credentials = cors["allow_credentials"].as<bool>(true);
    cors_.max_age = cors["max_age"].as<uint32_t>(86400);
    return true;
}

} // namespace furbbs::config
