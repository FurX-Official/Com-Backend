#ifndef FURBBS_SERVICE_IMPL_SHOP_SERVICE_H
#define FURBBS_SERVICE_IMPL_SHOP_SERVICE_H

#include "repository/shop_repository.h"
#include "repository/user_repository.h"
#include "repository/customization_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

struct PurchaseResult {
    bool success = false;
    std::string message;
    int32_t points_spent = 0;
    std::string item_name;
};

struct CheckInResult {
    bool success = false;
    std::string message;
    int32_t points_earned = 0;
    int32_t continuous_days = 0;
    bool is_bonus = false;
};

struct CheckInStatus {
    bool checked_in_today = false;
    int32_t continuous_days = 0;
    int32_t total_check_ins = 0;
    std::vector<int32_t> monthly_days;
};

struct ClaimResult {
    bool success = false;
    std::string message;
    int32_t points_claimed = 0;
};

class ShopService {
public:
    static ShopService& Instance() {
        static ShopService instance;
        return instance;
    }

    std::vector<repository::ShopItemEntity> GetShopItems(
        int type, bool only_on_sale, const std::string& sort_by,
        int page, int page_size, int& out_total);

    PurchaseResult PurchaseItem(const std::string& token, int32_t item_id, int32_t quantity);

    std::vector<repository::TaskProgressEntity> GetUserTasks(
        const std::string& token, int32_t& out_points_earned);

    ClaimResult ClaimTaskReward(const std::string& token, int32_t task_id);

    CheckInResult CheckIn(const std::string& token);

    CheckInStatus GetCheckInStatus(const std::string& token);

    bool SetEssence(const std::string& token, int64_t post_id,
                    bool is_essence, int32_t level);

    bool SetSticky(const std::string& token, int64_t post_id,
                   bool is_sticky, int32_t weight, int64_t expiry);

private:
    ShopService() = default;

    void DeliverItem(const std::string& user_id, int item_type, int32_t item_id);
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_SHOP_SERVICE_H
