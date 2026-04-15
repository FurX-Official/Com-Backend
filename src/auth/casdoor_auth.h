#ifndef FURBBS_AUTH_CASDOOR_AUTH_H
#define FURBBS_AUTH_CASDOOR_AUTH_H

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace furbbs::auth {

struct CasdoorUser {
    std::string id;
    std::string name;
    std::string display_name;
    std::string avatar;
    std::string email;
    std::string bio;
};

class CasdoorAuth {
public:
    static CasdoorAuth& Instance();

    bool Init(const std::string& endpoint, const std::string& client_id,
              const std::string& client_secret, const std::string& certificate,
              const std::string& org_name, const std::string& app_name);

    std::optional<CasdoorUser> VerifyToken(const std::string& token);
    
    std::optional<CasdoorUser> GetUserInfo(const std::string& user_id);
    
    bool SyncUserToDB(const CasdoorUser& user);
    
    bool IsHealthy() const;

private:
    CasdoorAuth() = default;
    ~CasdoorAuth() = default;

    std::optional<nlohmann::json> ParseJwtToken(const std::string& token);

    std::string endpoint_;
    std::string client_id_;
    std::string client_secret_;
    std::string certificate_;
    std::string org_name_;
    std::string app_name_;
};

} // namespace furbbs::auth

#endif // FURBBS_AUTH_CASDOOR_AUTH_H
