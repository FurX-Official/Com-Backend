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

struct SecurityConfig {
    bool enable_rate_limit;
    uint32_t rate_limit_requests;
    uint32_t rate_limit_window;
    bool enable_xss_protection;
    bool enable_sql_protection;
    uint32_t jwt_expiry_hours;
};

struct CorsConfig {
    bool enabled;
    std::vector<std::string> allowed_origins;
    std::vector<std::string> allowed_methods;
    std::vector<std::string> allowed_headers;
    std::vector<std::string> expose_headers;
    bool allow_credentials;
    uint32_t max_age;
};

struct NeteaseVerifyConfig {
    bool enabled;
    std::string secret_id;
    std::string secret_key;
    std::string business_id;
    bool mandatory_verification;
    bool require_face_verify;
    bool cache_verified;
};

class Config {
public:
    static Config& Instance();

    bool Load(const std::string& config_file);

    const ServerConfig& GetServerConfig() const { return server_; }
    const DatabaseConfig& GetDatabaseConfig() const { return database_; }
    const CasdoorConfig& GetCasdoorConfig() const { return casdoor_; }
    const LogConfig& GetLogConfig() const { return log_; }
    const SecurityConfig& GetSecurityConfig() const { return security_; }
    const CorsConfig& GetCorsConfig() const { return cors_; }
    const NeteaseVerifyConfig& GetNeteaseVerifyConfig() const { return netease_verify_; }

private:
    Config() = default;
    ~Config() = default;

    bool ParseServer(const YAML::Node& config);
    bool ParseDatabase(const YAML::Node& config);
    bool ParseCasdoor(const YAML::Node& config);
    bool ParseLog(const YAML::Node& config);
    bool ParseSecurity(const YAML::Node& config);
    bool ParseCors(const YAML::Node& config);
    bool ParseNeteaseVerify(const YAML::Node& config);

    ServerConfig server_;
    DatabaseConfig database_;
    CasdoorConfig casdoor_;
    LogConfig log_;
    SecurityConfig security_;
    CorsConfig cors_;
    NeteaseVerifyConfig netease_verify_;
};

} // namespace furbbs::config

#endif // FURBBS_CONFIG_CONFIG_H
