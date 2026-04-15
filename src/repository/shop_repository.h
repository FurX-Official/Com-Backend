#ifndef FURBBS_REPOSITORY_SHOP_REPOSITORY_H
#define FURBBS_REPOSITORY_SHOP_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <optional>

namespace furbbs::repository {

struct ShopItemEntity {
    int32_t id = 0;
    int type = 0;
    int32_t item_id = 0;
    std::string name;
    std::string description;
    int32_t price = 0;
    int32_t discount_price = 0;
    int32_t stock = -1;
    int32_t sales = 0;
    bool is_hot = false;
    bool is_new = false;
    int64_t start_time = 0;
    int64_t end_time = 0;
    std::vector<std::string> tags;
};

struct TaskProgressEntity {
    int32_t task_id = 0;
    std::string name;
    std::string description;
    int32_t target_value = 0;
    int32_t current_value = 0;
    int32_t points_reward = 0;
    bool is_completed = false;
    bool is_claimed = false;
};

class ShopRepository : protected BaseRepository {
public:
    static ShopRepository& Instance() {
        static ShopRepository instance;
        return instance;
    }

    std::vector<ShopItemEntity> GetShopItems(int type, bool only_on_sale,
                                                 const std::string& sort_by,
                                                 int limit, int offset);

    int GetTotalShopItems(int type, bool only_on_sale);

    std::optional<ShopItemEntity> GetShopItem(int32_t id);

    void DeductStock(int32_t item_id, int32_t quantity);

    void RecordPurchase(const std::string& user_id, int32_t shop_item_id,
                       int32_t price_paid, int32_t quantity);

    std::vector<TaskProgressEntity> GetUserTasks(const std::string& user_id);

    void UpdateTaskProgress(const std::string& user_id, int32_t task_id,
                           int32_t current_value, bool is_completed);

    void ClaimTaskReward(const std::string& user_id, int32_t task_id);

    bool CheckIn(const std::string& user_id, int32_t& out_continuous,
                 int32_t& out_points, bool& out_is_bonus);

    void GetCheckInStatus(const std::string& user_id, bool& out_checked_today,
                         int32_t& out_continuous, int32_t& out_total,
                         std::vector<int32_t>& out_monthly);

    void SetPostEssence(int64_t post_id, bool is_essence, int32_t level);

    void SetPostSticky(int64_t post_id, bool is_sticky, int32_t weight, int64_t expiry);

private:
    ShopRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_SHOP_REPOSITORY_H
