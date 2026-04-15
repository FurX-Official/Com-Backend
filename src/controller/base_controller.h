#ifndef FURBBS_CONTROLLER_BASE_CONTROLLER_H
#define FURBBS_CONTROLLER_BASE_CONTROLLER_H

#include <trpc/trpc.h>
#include <string>
#include <optional>

namespace furbbs::controller {

struct AuthResult {
    bool valid = false;
    std::string user_id;
    std::string username;
    bool is_admin = false;
};

class BaseController {
public:
    static AuthResult VerifyToken(const std::string& token) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) {
            return {false, "", "", false};
        }
        return {true, user->user_id, user->username, user->is_admin};
    }

    static template<typename ResponseT>
    void SetResponse(ResponseT* response, int code, const std::string& message) {
        response->set_code(code);
        response->set_message(message);
    }
};

} // namespace furbbs::controller

#endif // FURBBS_CONTROLLER_BASE_CONTROLLER_H
