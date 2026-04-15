#include "shop_repository.h"
#include "user_repository.h"
#include <chrono>

namespace furbbs::repository {

std::vector<ShopItemEntity> ShopRepository::GetShopItems(int type, bool only_on_sale,
                                                           const std::string& sort_by,
                                                           int limit, int offset) {
    return Execute<std::vector<ShopItemEntity>>([&](pqxx::work& txn) {
        pqxx::result result;
        if (type >= 0) {
            const std::string& query = only_on_sale
                ? sql::SHOP_ITEMS_BY_TYPE_ON_SALE
                : sql::SHOP_ITEMS_BY_TYPE;
            result = txn.exec_params(query, type, limit, offset);
        } else {
            const std::string& query = only_on_sale
                ? sql::SHOP_ITEMS_ON_SALE
                : sql::SHOP_ITEMS_BASE;
            result = txn.exec_params(query, limit, offset);
        }

        std::vector<ShopItemEntity> items;
        for (const auto& row : result) {
            ShopItemEntity item;
            item.id = row[0].as<int32_t>();
            item.type = row[1].as<int>();
            item.item_id = row[2].as<int32_t>();
            item.name = row[3].as<std::string>();
            if (!row[4].is_null()) item.description = row[4].as<std::string>();
            item.price = row[5].as<int32_t>();
            if (!row[6].is_null()) item.discount_price = row[6].as<int32_t>();
            item.stock = row[7].as<int32_t>();
            item.sales = row[8].as<int32_t>();
            item.is_hot = row[9].as<bool>();
            item.is_new = row[10].as<bool>();
            if (!row[11].is_null()) item.start_time = row[11].as<int64_t>();
            if (!row[12].is_null()) item.end_time = row[12].as<int64_t>();
            if (!row[13].is_null()) {
                pqxx::array_parser<std::vector<std::string>> parser(row[13]);
                parser.get(item.tags);
            }
            items.push_back(item);
        }
        return items;
    });
}

int ShopRepository::GetTotalShopItems(int type, bool only_on_sale) {
    return Execute<int>([&](pqxx::work& txn) {
        pqxx::result result;
        if (type >= 0) {
            const std::string& query = only_on_sale ? sql::SHOP_ITEMS_COUNT_BY_TYPE_ON_SALE : sql::SHOP_ITEMS_COUNT_BY_TYPE;
            result = txn.exec_params(query, type);
        } else {
            const std::string& query = only_on_sale ? sql::SHOP_ITEMS_COUNT_ON_SALE : sql::SHOP_ITEMS_COUNT_ALL;
            result = txn.exec(query);
        }
        return result[0][0].as<int>();
    });
}

std::optional<ShopItemEntity> ShopRepository::GetShopItem(int32_t id) {
    return Execute<std::optional<ShopItemEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::SHOP_GET_ITEM, id);
        if (result.empty()) return std::nullopt;

        ShopItemEntity item;
        item.id = result[0][0].as<int32_t>();
        item.type = result[0][1].as<int>();
        item.item_id = result[0][2].as<int32_t>();
        item.name = result[0][3].as<std::string>();
        item.price = result[0][4].as<int32_t>();
        if (!result[0][5].is_null()) item.discount_price = result[0][5].as<int32_t>();
        item.stock = result[0][6].as<int32_t>();
        return item;
    });
}

void ShopRepository::DeductStock(int32_t item_id, int32_t quantity) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::SHOP_DEDUCT_STOCK, quantity, item_id);
    });
}

void ShopRepository::RecordPurchase(const std::string& user_id, int32_t shop_item_id,
                                     int32_t price_paid, int32_t quantity) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(sql::SHOP_RECORD_PURCHASE, user_id, shop_item_id, price_paid, quantity, timestamp);
    });
}

std::vector<TaskProgressEntity> ShopRepository::GetUserTasks(const std::string& user_id) {
    return Execute<std::vector<TaskProgressEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::SHOP_GET_USER_TASKS, user_id);

        std::vector<TaskProgressEntity> tasks;
        for (const auto& row : result) {
            TaskProgressEntity t;
            t.task_id = row[0].as<int32_t>();
            t.name = row[1].as<std::string>();
            t.description = row[2].as<std::string>();
            t.target_value = row[3].as<int32_t>();
            t.points_reward = row[4].as<int32_t>();
            t.current_value = row[5].as<int32_t>();
            t.is_completed = row[6].as<bool>();
            t.is_claimed = row[7].as<bool>();
            tasks.push_back(t);
        }
        return tasks;
    });
}

void ShopRepository::UpdateTaskProgress(const std::string& user_id, int32_t task_id,
                                         int32_t current_value, bool is_completed) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::SHOP_UPDATE_TASK_PROGRESS, user_id, task_id, current_value, is_completed);
    });
}

void ShopRepository::ClaimTaskReward(const std::string& user_id, int32_t task_id) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::SHOP_CLAIM_TASK_REWARD, user_id, task_id);
    });
}

bool ShopRepository::CheckIn(const std::string& user_id, int32_t& out_continuous,
                              int32_t& out_points, bool& out_is_bonus) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto today_result = txn.exec_params(sql::SHOP_CHECKIN_CHECK_TODAY, user_id);

        if (!today_result.empty()) {
            return false;
        }

        auto last_result = txn.exec_params(sql::SHOP_CHECKIN_GET_LAST, user_id);

        out_continuous = 1;
        if (!last_result.empty()) {
            out_continuous = last_result[0][0].as<int32_t>() + 1;
        }

        out_points = 10;
        out_is_bonus = false;
        if (out_continuous % 7 == 0) {
            out_points = out_continuous * 2;
            out_is_bonus = true;
        }

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(sql::SHOP_CHECKIN_INSERT, user_id, out_continuous, out_points, out_is_bonus, timestamp);

        UserRepository::Instance().AddPoints(user_id, out_points);
        return true;
    });
}

void ShopRepository::GetCheckInStatus(const std::string& user_id, bool& out_checked_today,
                                       int32_t& out_continuous, int32_t& out_total,
                                       std::vector<int32_t>& out_monthly) {
    Execute([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::SHOP_GET_CHECKIN_STATUS, user_id);

        out_checked_today = false;
        out_continuous = 0;
        out_total = result.size();
        out_monthly.clear();

        int expected_day = 0;
        for (const auto& row : result) {
            auto date = row[0].as<std::string>();
            int day = std::stoi(date.substr(8, 2));

            if (expected_day == 0) {
                if (day == std::chrono::system_clock::to_time_t(
                    std::chrono::system_clock::now()) / 86400 % 31 + 1) {
                    out_checked_today = true;
                }
                expected_day = day - 1;
                out_continuous = 1;
            } else if (day == expected_day) {
                out_continuous++;
                expected_day--;
            } else {
                break;
            }

            out_monthly.push_back(day);
        }
    });
}

void ShopRepository::SetPostEssence(int64_t post_id, bool is_essence, int32_t level) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::SHOP_SET_POST_ESSENCE, is_essence, level, post_id);
    });
}

void ShopRepository::SetPostSticky(int64_t post_id, bool is_sticky, int32_t weight, int64_t expiry) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::SHOP_SET_POST_STICKY, is_sticky, weight, expiry > 0 ? expiry : nullptr, post_id);
    });
}

} // namespace furbbs::repository
