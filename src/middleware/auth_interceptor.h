#ifndef FURBBS_MIDDLEWARE_AUTH_INTERCEPTOR_H
#define FURBBS_MIDDLEWARE_AUTH_INTERCEPTOR_H

#include "trpc/server/server_interceptor.h"
#include <string>

namespace furbbs::middleware {

class AuthInterceptor : public ::trpc::ServerInterceptor {
public:
    AuthInterceptor() = default;
    ~AuthInterceptor() override = default;

    const char* Name() const override { return "AuthInterceptor"; }

    void Invoke(::trpc::ServerContextPtr context, ::trpc::InterceptorTransportType type,
                ::trpc::InterceptorPoint point, ::trpc::InterceptorDoneFunction done) override;

private:
    bool VerifyRequest(::trpc::ServerContextPtr context);
    bool CheckRateLimit(const std::string& client_ip);
    bool ValidateContent(const std::string& content);
    void LogRequest(::trpc::ServerContextPtr context, int64_t start_time);
};

} // namespace furbbs::middleware

#endif // FURBBS_MIDDLEWARE_AUTH_INTERCEPTOR_H
