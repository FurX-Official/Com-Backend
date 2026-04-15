#ifndef FURBBS_CONTROLLER_BASE_CONTROLLER_H
#define FURBBS_CONTROLLER_BASE_CONTROLLER_H

#include <trpc/trpc.h>
#include <string>
#include <optional>
#include "../service_impl/security_service.h"

namespace furbbs::controller {

struct AuthResult {
    bool valid = false;
    std::string user_id;
    std::string username;
    bool is_admin = false;
};

class BaseController {
public:
    static bool CheckIpSecurity(const std::string& ip_address) {
        if (ip_address.empty()) return true;
        if (service::SecurityService::Instance().IsIpBlacklisted(ip_address)) {
            return false;
        }
        return service::SecurityService::Instance().CheckRateLimit(
            ip_address, "global_api", 1000, 60);
    }

    static AuthResult VerifyToken(const std::string& token) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) {
            return {false, "", "", false};
        }
        return {true, user->user_id, user->username, user->is_admin};
    }

    template<typename ResponseT>
    static void SetResponse(ResponseT* response, int code, const std::string& message) {
        response->set_code(code);
        response->set_message(message);
    }
};

} // namespace furbbs::controller

#endif // FURBBS_CONTROLLER_BASE_CONTROLLER_H
