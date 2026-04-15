#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "trpc/server/server.h"
#include "trpc/server/trpc_server.h"
#include "config/config.h"
#include "db/database.h"
#include "auth/casdoor_auth.h"
#include "service/furbbs_service.h"
#include "middleware/auth_interceptor.h"
#include "common/security.h"

int main(int argc, char* argv[]) {
    std::string config_file = "./config/config.yaml";
    if (argc > 1) {
        config_file = argv[1];
    }

    if (!furbbs::config::Config::Instance().Load(config_file)) {
        std::cerr << "Failed to load config file: " << config_file << std::endl;
        return 1;
    }

    const auto& log_config = furbbs::config::Config::Instance().GetLogConfig();
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_config.file, true);
    
    spdlog::logger logger("furbbs", {console_sink, file_sink});
    logger.set_level(spdlog::level::from_str(log_config.level));
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

    spdlog::info("Starting FurBBS Server...");

    const auto& db_config = furbbs::config::Config::Instance().GetDatabaseConfig();
    if (!furbbs::db::Database::Instance().Init(
        db_config.host, db_config.port, db_config.dbname,
        db_config.user, db_config.password, db_config.max_connections
    )) {
        spdlog::error("Failed to initialize database");
        return 1;
    }

    const auto& casdoor_config = furbbs::config::Config::Instance().GetCasdoorConfig();
    if (!furbbs::auth::CasdoorAuth::Instance().Init(
        casdoor_config.endpoint, casdoor_config.client_id,
        casdoor_config.client_secret, casdoor_config.certificate,
        casdoor_config.org_name, casdoor_config.app_name
    )) {
        spdlog::error("Failed to initialize casdoor auth");
        return 1;
    }

    const auto& server_config = furbbs::config::Config::Instance().GetServerConfig();
    
    ::trpc::TrpcServerConfig trpc_config;
    trpc_config.ip = "0.0.0.0";
    trpc_config.port = static_cast<uint16_t>(std::stoul(server_config.listen.substr(server_config.listen.find(':') + 1)));
    trpc_config.io_thread_num = server_config.thread_num;
    trpc_config.idle_time = server_config.idle_time;

    ::trpc::TrpcServer server(trpc_config);
    
    auto service = std::make_shared<furbbs::service::FurBBSServiceImpl>();
    if (!server.RegisterService(service)) {
        spdlog::error("Failed to register service");
        return 1;
    }

    auto auth_interceptor = std::make_shared<furbbs::middleware::AuthInterceptor>();
    if (!server.RegisterInterceptor(auth_interceptor)) {
        spdlog::warn("Failed to register auth interceptor");
    } else {
        spdlog::info("Auth interceptor registered successfully");
    }

    spdlog::info("FurBBS Server started on {}", server_config.listen);
    spdlog::info("Furry Community Backend is running! 🐺🦊🐲");

    server.Start();
    server.Wait();

    spdlog::info("FurBBS Server stopped");
    return 0;
}
