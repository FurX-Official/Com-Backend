#include "redeem_service.h"
#include <chrono>

namespace furbbs::service {

GenerateResult RedeemService::GenerateCards(const std::string& token, int type, int value,
                                      const std::string& item_id, const std::string& item_name,
                                      int quantity, int max_uses, int64_t expiry_days,
                                      const std::string& batch_no, int code_length) {
    GenerateResult result;

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || user_opt->role != "admin") {
        result.success = false;
        result.message = "Permission denied";
        return result;
    }

    quantity = std::min(1000, std::max(1, quantity));
    code_length = std::min(32, std::max(8, code_length > 0 ? code_length : 16));

    int64_t timestamp = GetCurrentTimestamp();
    int64_t expiry_date = 0;
    if (expiry_days > 0) {
        expiry_date = timestamp + expiry_days * 86400000LL;
    }

    auto& repo = repository::RedeemRepository::Instance();
    for (int i = 0; i < quantity; ++i) {
        std::string code = GenerateSingleCode(code_length);
        repo.InsertCard(code, type, value, item_id, item_name,
                        max_uses > 0 ? max_uses : 1, expiry_date, user_opt->id, batch_no);
        result.codes.push_back(code);
    }

    result.success = true;
    result.message = "Cards generated successfully";
    return result;
}

RedeemResult RedeemService::RedeemCard(const std::string& token, const std::string& code) {
    RedeemResult result;

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    int64_t timestamp = GetCurrentTimestamp();
    auto& redeem_repo = repository::RedeemRepository::Instance();
    auto& user_repo = repository::UserRepository::Instance();

    auto card_opt = redeem_repo.GetCardByCode(code);
    if (!card_opt) {
        result.success = false;
        result.message = "Invalid redeem code";
        return result;
    }

    auto card = *card_opt;
    if (card.status != 0) {
        result.success = false;
        result.message = "Card is not active";
        return result;
    }
    if (card.expiry_date > 0 && card.expiry_date < timestamp) {
        result.success = false;
        result.message = "Card has expired";
        return result;
    }
    if (card.used_count >= card.max_uses) {
        result.success = false;
        result.message = "Card has been fully used";
        return result;
    }

    switch (card.type) {
        case 0:
            user_repo.AddPoints(user_opt->id, card.value);
            result.reward_name = std::to_string(card.value) + " Points";
            break;
        case 1: {
            int64_t new_expiry = timestamp + card.value * 86400000LL;
            user_repo.AddMembership(user_opt->id, 2, new_expiry);
            result.reward_name = "Membership " + std::to_string(card.value) + " Days";
            break;
        }
        case 2:
            user_repo.UnlockTitle(user_opt->id, card.value);
            result.reward_name = card.item_name.empty() ? "Exclusive Title" : card.item_name;
            break;
        case 3:
            user_repo.UnlockAvatarFrame(user_opt->id, card.value);
            result.reward_name = card.item_name.empty() ? "Avatar Frame" : card.item_name;
            break;
        default:
            result.reward_name = card.item_name.empty() ? "Custom Reward" : card.item_name;
    }

    int new_used_count = card.used_count + 1;
    int new_status = (new_used_count >= card.max_uses) ? 1 : 0;
    redeem_repo.UpdateCardStatus(card.id, new_used_count, new_status, user_opt->id, timestamp);

    result.success = true;
    result.reward_type = card.type;
    result.reward_value = card.value;
    result.message = "Card redeemed successfully!";
    return result;
}

std::vector<repository::RedeemCardEntity> RedeemService::GetCardList(
    const std::string& token, int status, int type,
    const std::string& batch_no, int page, int page_size, int& out_total) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || user_opt->role != "admin") {
        out_total = 0;
        return {};
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::RedeemRepository::Instance().GetTotalCards();
    return repository::RedeemRepository::Instance().GetCardList(
        status, type, batch_no, page_size, offset);
}

std::string RedeemService::GenerateSingleCode(int length) {
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string code;
    for (int i = 0; i < length; ++i) {
        code += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return code;
}

int64_t RedeemService::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

} // namespace furbbs::service
