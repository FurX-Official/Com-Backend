#ifndef FURBBS_SERVICE_IMPL_REDEEM_SERVICE_H
#define FURBBS_SERVICE_IMPL_REDEEM_SERVICE_H

#include "repository/redeem_repository.h"
#include "repository/user_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::service {

struct RedeemResult {
    bool success = false;
    std::string message;
    int reward_type = 0;
    int reward_value = 0;
    std::string reward_name;
};

struct GenerateResult {
    bool success = false;
    std::string message;
    std::vector<std::string> codes;
};

class RedeemService {
public:
    static RedeemService& Instance() {
        static RedeemService instance;
        return instance;
    }

    GenerateResult GenerateCards(const std::string& token, int type, int value,
                           const std::string& item_id, const std::string& item_name,
                           int quantity, int max_uses, int64_t expiry_days,
                           const std::string& batch_no, int code_length);

    RedeemResult RedeemCard(const std::string& token, const std::string& code);

    std::vector<repository::RedeemCardEntity> GetCardList(
        const std::string& token, int status, int type,
        const std::string& batch_no, int page, int page_size, int& out_total);

private:
    RedeemService() = default;

    std::string GenerateSingleCode(int length);
    int64_t GetCurrentTimestamp();
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_REDEEM_SERVICE_H
