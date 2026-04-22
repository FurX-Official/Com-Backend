#include "../service_impl/shop_service.h"
#include "../common/result.h"

namespace furbbs::controller {

using ::trpc::ServerContextPtr;
using namespace furbbs::repository;

::trpc::Status GetShopItems(ServerContextPtr context,
                             const ::furbbs::GetShopItemsRequest* request,
                             ::furbbs::GetShopItemsResponse* response) {
    try {
        int total = 0;
        auto items = service::ShopService::Instance().GetShopItems(
            request->type(), request->only_on_sale(),
            request->sort_by(), request->page(), request->page_size(), total);
        for (const auto& item : items) {
            auto* shop_item = response->add_items();
            shop_item->set_id(item.id);
            shop_item->set_name(item.name);
            shop_item->set_description(item.description);
            shop_item->set_icon(item.icon);
            shop_item->set_price(item.price);
            shop_item->set_type(item.type);
            shop_item->set_quantity(item.quantity);
            shop_item->set_is_on_sale(item.is_on_sale);
            shop_item->set_sale_price(item.sale_price);
        }
        response->set_total(total);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status PurchaseItem(ServerContextPtr context,
                             const ::furbbs::PurchaseItemRequest* request,
                             ::furbbs::PurchaseItemResponse* response) {
    try {
        auto result = service::ShopService::Instance().PurchaseItem(
            request->access_token(), request->item_id(), request->quantity());
        response->set_success(result.success);
        response->set_message(result.message);
        response->set_points_spent(result.points_spent);
        response->set_code(result.success ? RESULT_OK : RESULT_ERROR);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("购买失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetUserInventory(ServerContextPtr context,
                                 const ::furbbs::GetUserInventoryRequest* request,
                                 ::furbbs::GetUserInventoryResponse* response) {
    try {
        std::vector<repository::TaskProgressEntity> tasks;
        int points_earned = 0;
        tasks = service::ShopService::Instance().GetUserTasks(
            request->access_token(), points_earned);
        for (const auto& task : tasks) {
            auto* item = response->add_items();
            item->set_item_id(task.task_id);
            item->set_item_name(task.task_name);
            item->set_quantity(task.is_claimed ? 1 : 0);
        }
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status DailyCheckIn(ServerContextPtr context,
                             const ::furbbs::DailyCheckInRequest* request,
                             ::furbbs::DailyCheckInResponse* response) {
    try {
        auto result = service::ShopService::Instance().CheckIn(request->access_token());
        response->set_success(result.success);
        response->set_message(result.message);
        response->set_points_earned(result.points_earned);
        response->set_continuous_days(result.continuous_days);
        response->set_is_bonus(result.is_bonus);
        response->set_code(result.success ? RESULT_OK : RESULT_ERROR);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("签到失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetCheckInStatus(ServerContextPtr context,
                                  const ::furbbs::GetCheckInStatusRequest* request,
                                  ::furbbs::GetCheckInStatusResponse* response) {
    try {
        auto status = service::ShopService::Instance().GetCheckInStatus(request->access_token());
        response->set_checked_in_today(status.checked_in_today);
        response->set_continuous_days(status.continuous_days);
        response->set_total_check_ins(status.total_check_ins);
        for (int day : status.monthly_days) {
            response->add_monthly_days(day);
        }
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetUserLevel(ServerContextPtr context,
                             const ::furbbs::GetUserLevelRequest* request,
                             ::furbbs::GetUserLevelResponse* response) {
    try {
        int points_earned = 0;
        auto tasks = service::ShopService::Instance().GetUserTasks(
            request->access_token(), points_earned);
        response->set_level(points_earned / 100 + 1);
        response->set_current_exp(points_earned % 100);
        response->set_exp_to_next_level(100 - (points_earned % 100));
        response->set_total_exp(points_earned);
        response->set_rank(points_earned / 500 + 1);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

} // namespace furbbs::controller
