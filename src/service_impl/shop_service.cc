#include "shop_service.h"

namespace furbbs::service {

std::vector<repository::ShopItemEntity> ShopService::GetShopItems(
    int type, bool only_on_sale, const std::string& sort_by,
    int page, int page_size, int& out_total) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::ShopRepository::Instance().GetTotalShopItems(type, only_on_sale);
    return repository::ShopRepository::Instance().GetShopItems(
        type, only_on_sale, sort_by, page_size, offset);
}

PurchaseResult ShopService::PurchaseItem(const std::string& token, int32_t item_id, int32_t quantity) {
    PurchaseResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    auto item_opt = repository::ShopRepository::Instance().GetShopItem(item_id);
    if (!item_opt) {
        result.success = false;
        result.message = "Item not found";
        return result;
    }

    auto item = *item_opt;
    int32_t actual_price = item.discount_price > 0 ? item.discount_price : item.price;
    int32_t total_price = actual_price * quantity;

    auto stats = repository::UserRepository::Instance().GetUserStats(user_opt->id);
    if (!stats || stats->points < total_price) {
        result.success = false;
        result.message = "Insufficient points";
        return result;
    }

    if (item.stock >= 0 && item.stock < quantity) {
        result.success = false;
        result.message = "Out of stock";
        return result;
    }

    repository::UserRepository::Instance().AddPoints(user_opt->id, -total_price);
    repository::ShopRepository::Instance().DeductStock(item_id, quantity);
    repository::ShopRepository::Instance().RecordPurchase(
        user_opt->id, item_id, total_price, quantity);

    DeliverItem(user_opt->id, item.type, item.item_id);

    result.success = true;
    result.points_spent = total_price;
    result.item_name = item.name;
    result.message = "Purchase successful";
    return result;
}

std::vector<repository::TaskProgressEntity> ShopService::GetUserTasks(
    const std::string& token, int32_t& out_points_earned) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        out_points_earned = 0;
        return {};
    }

    out_points_earned = 0;
    auto tasks = repository::ShopRepository::Instance().GetUserTasks(user_opt->id);
    for (const auto& t : tasks) {
        if (t.is_claimed) {
            out_points_earned += t.points_reward;
        }
    }
    return tasks;
}

ClaimResult ShopService::ClaimTaskReward(const std::string& token, int32_t task_id) {
    ClaimResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    auto tasks = repository::ShopRepository::Instance().GetUserTasks(user_opt->id);
    for (const auto& t : tasks) {
        if (t.task_id == task_id) {
            if (!t.is_completed) {
                result.success = false;
                result.message = "Task not completed";
                return result;
            }
            if (t.is_claimed) {
                result.success = false;
                result.message = "Already claimed";
                return result;
            }

            repository::ShopRepository::Instance().ClaimTaskReward(user_opt->id, task_id);
            repository::UserRepository::Instance().AddPoints(user_opt->id, t.points_reward);

            result.success = true;
            result.points_claimed = t.points_reward;
            result.message = "Reward claimed successfully";
            return result;
        }
    }

    result.success = false;
    result.message = "Task not found";
    return result;
}

CheckInResult ShopService::CheckIn(const std::string& token) {
    CheckInResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    result.success = repository::ShopRepository::Instance().CheckIn(
        user_opt->id, result.continuous_days, result.points_earned, result.is_bonus);

    if (!result.success) {
        result.message = "Already checked in today";
    } else {
        result.message = result.is_bonus ? "Bonus check-in!" : "Check-in successful";
    }
    return result;
}

CheckInStatus ShopService::GetCheckInStatus(const std::string& token) {
    CheckInStatus status;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return status;
    }

    repository::ShopRepository::Instance().GetCheckInStatus(
        user_opt->id, status.checked_in_today,
        status.continuous_days, status.total_check_ins,
        status.monthly_days);

    return status;
}

bool ShopService::SetEssence(const std::string& token, int64_t post_id,
                              bool is_essence, int32_t level) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
        return false;
    }

    repository::ShopRepository::Instance().SetPostEssence(post_id, is_essence, level);
    return true;
}

bool ShopService::SetSticky(const std::string& token, int64_t post_id,
                             bool is_sticky, int32_t weight, int64_t expiry) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || (user_opt->role != "admin" && user_opt->role != "moderator")) {
        return false;
    }

    repository::ShopRepository::Instance().SetPostSticky(post_id, is_sticky, weight, expiry);
    return true;
}

void ShopService::DeliverItem(const std::string& user_id, int item_type, int32_t item_id) {
    switch (item_type) {
        case 0:
        case 2:
            repository::UserRepository::Instance().UnlockTitle(user_id, item_id);
            break;
        case 1:
            repository::UserRepository::Instance().UnlockAvatarFrame(user_id, item_id);
            break;
    }
}

} // namespace furbbs::service
