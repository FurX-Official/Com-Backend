#ifndef FURBBS_CONFIG_CONFIG_H
#define FURBBS_CONFIG_CONFIG_H

#include <string>
#include <cstdint>

namespace YAML { class Node; }

namespace furbbs::config {

struct ServerConfig {
    std::string listen;
    uint32_t thread_num;
    uint32_t idle_time;
};

struct DatabaseConfig {
    std::string host;
    uint16_t port;
    std::string dbname;
    std::string user;
    std::string password;
    uint32_t max_connections;
};

struct CasdoorConfig {
    std::string endpoint;
    std::string client_id;
    std::string client_secret;
    std::string certificate;
    std::string org_name;
    std::string app_name;
    std::string redirect_uri;
};

struct LogConfig {
    std::string level;
    std::string file;
    uint32_t max_size;
    uint32_t max_files;
};

class Config {
public:
    static Config& Instance();

    bool Load(const std::string& config_file);

    const ServerConfig& GetServerConfig() const { return server_; }
    const DatabaseConfig& GetDatabaseConfig() const { return database_; }
    const CasdoorConfig& GetCasdoorConfig() const { return casdoor_; }
    const LogConfig& GetLogConfig() const { return log_; }

private:
    Config() = default;
    ~Config() = default;

    bool ParseServer(const YAML::Node& config);
    bool ParseDatabase(const YAML::Node& config);
    bool ParseCasdoor(const YAML::Node& config);
    bool ParseLog(const YAML::Node& config);

    ServerConfig server_;
    DatabaseConfig database_;
    CasdoorConfig casdoor_;
    LogConfig log_;
};

} // namespace furbbs::config

#endif // FURBBS_CONFIG_CONFIG_H
