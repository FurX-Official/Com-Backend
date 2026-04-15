#include "auth_interceptor.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include "../auth/casdoor_auth.h"
#include "../common/security.h"

namespace furbbs::middleware {

void AuthInterceptor::Invoke(::trpc::ServerContextPtr context, ::trpc::InterceptorTransportType type,
                            ::trpc::InterceptorPoint point, ::trpc::InterceptorDoneFunction done) {
    if (point != ::trpc::InterceptorPoint::PRE_HANDLE) {
        done();
        return;
    }

    auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    const auto& method = context->GetFunc();
    
    const std::unordered_set<std::string> PUBLIC_METHODS = {
        "GetUser", "GetPost", "ListPosts", "ListComments", "ListCategories",
        "ListGallery", "GetUserFursonas", "GetUserCommissions", "ListEvents"
    };

    bool is_public = false;
    for (const auto& pub : PUBLIC_METHODS) {
        if (method.find(pub) != std::string::npos) {
            is_public = true;
            break;
        }
    }

    std::string client_ip = context->GetRemoteIp();
    
    if (!CheckRateLimit(client_ip)) {
        context->SetStatus(::trpc::Status(429, "Rate limit exceeded"));
        done();
        return;
    }

    if (!is_public) {
        if (!VerifyRequest(context)) {
            LogRequest(context, start_time);
            done();
            return;
        }
    }

    done();
    
    LogRequest(context, start_time);
}

bool AuthInterceptor::VerifyRequest(::trpc::ServerContextPtr context) {
    auto metadata = context->GetRequestMetadata();
    auto it = metadata.find("authorization");
    if (it == metadata.end()) {
        context->SetStatus(::trpc::Status(401, "Missing authorization token"));
        return false;
    }

    std::string token = it->second;
    if (token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        context->SetStatus(::trpc::Status(401, "Invalid token"));
        spdlog::warn("Invalid token from IP: {}", context->GetRemoteIp());
        return false;
    }

    context->SetReqContext("user_id", user_opt->id);
    return true;
}

bool AuthInterceptor::CheckRateLimit(const std::string& client_ip) {
    return common::RateLimiter::Instance().Allow(client_ip, 100, 60);
}

bool AuthInterceptor::ValidateContent(const std::string& content) {
    if (auto sql_attempt = common::Security::DetectSqlInjection(content)) {
        spdlog::warn("SQL injection attempt detected: {}", *sql_attempt);
        return false;
    }

    if (auto xss_attempt = common::Security::DetectXss(content)) {
        spdlog::warn("XSS attempt detected: {}", *xss_attempt);
        return false;
    }

    return true;
}

void AuthInterceptor::LogRequest(::trpc::ServerContextPtr context, int64_t start_time) {
    auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    int64_t latency = end_time - start_time;
    auto status = context->GetStatus();

    spdlog::info("Request: method={}, ip={}, latency={}ms, code={}",
                 context->GetFunc(), context->GetRemoteIp(), latency,
                 static_cast<int>(status.GetRetCode()));
}

} // namespace furbbs::middleware
