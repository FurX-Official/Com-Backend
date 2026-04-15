#ifndef FURBBS_SERVICE_IMPL_FURRY_CORE_SERVICE_H
#define FURBBS_SERVICE_IMPL_FURRY_CORE_SERVICE_H

#include "../repository/furry_core_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <random>

namespace furbbs::service {

class FurryCoreService {
public:
    static FurryCoreService& Instance() {
        static FurryCoreService instance;
        return instance;
    }

    std::string GenerateFursonaPrompt(const std::map<std::string, std::string>& params) {
        std::vector<std::string> species = {"狼", "狐狸", "龙", "虎", "狮", "兔", "猫", "狗", "鹿", "熊"};
        std::vector<std::string> colors = {"蓝色", "红色", "金色", "紫色", "绿色", "橙色", "粉色", "黑白"};
        std::vector<std::string> styles = {"卡通", "写实", "水彩", "像素", "赛博朋克", "日式动漫"};
        
        std::string species_v = params.count("species") ? params.at("species") : species[rand() % species.size()];
        std::string color_v = params.count("color") ? params.at("color") : colors[rand() % colors.size()];
        std::string style_v = params.count("style") ? params.at("style") : styles[rand() % styles.size()];
        std::string gender = params.count("gender") ? params.at("gender") : "中性";
        
        return "毛茸茸的" + color_v + species_v + "兽人，" + gender + "，" + style_v + "风格，高清细节，柔软毛发";
    }

    int64_t SavePrompt(const std::string& user_id, const repository::AIPromptEntity& data) {
        return repository::FurryCoreRepository::Instance().CreatePrompt(user_id, data);
    }

    std::vector<repository::AIPromptEntity> GetPrompts(
            const std::string& user_id, const std::string& viewer_id,
            bool only_public, int page, int page_size) {
        return repository::FurryCoreRepository::Instance().GetPrompts(
            user_id, viewer_id, only_public, page_size, (page - 1) * page_size);
    }

    bool DeletePrompt(const std::string& user_id, int64_t prompt_id) {
        return repository::FurryCoreRepository::Instance().DeletePrompt(user_id, prompt_id);
    }

    int64_t CreateRelation(const std::string& user_id, const repository::FursonaRelationEntity& data) {
        auto relations = repository::FurryCoreRepository::Instance().GetFursonaRelations(data.fursona_a_id);
        for (auto& r : relations) {
            if ((r.fursona_a_id == data.fursona_a_id && r.fursona_b_id == data.fursona_b_id) ||
                (r.fursona_a_id == data.fursona_b_id && r.fursona_b_id == data.fursona_a_id)) {
                return 0;
            }
        }
        return repository::FurryCoreRepository::Instance().CreateRelation(user_id, data);
    }

    bool ConfirmRelation(const std::string& user_id, int64_t relation_id) {
        return repository::FurryCoreRepository::Instance().ConfirmRelation(user_id, relation_id);
    }

    std::vector<repository::FursonaRelationEntity> GetFursonaRelations(int64_t fursona_id) {
        return repository::FurryCoreRepository::Instance().GetFursonaRelations(fursona_id);
    }

    bool DeleteRelation(const std::string& user_id, int64_t relation_id) {
        return repository::FurryCoreRepository::Instance().DeleteRelation(user_id, relation_id);
    }

    int64_t CreateWorld(const std::string& user_id, const repository::WorldSettingEntity& data) {
        return repository::FurryCoreRepository::Instance().CreateWorld(user_id, data);
    }

    bool UpdateWorld(const std::string& user_id, int64_t world_id, const repository::WorldSettingEntity& data) {
        return repository::FurryCoreRepository::Instance().UpdateWorld(user_id, world_id, data);
    }

    std::vector<repository::WorldSettingEntity> GetWorlds(
            const std::string& user_id, const std::string& viewer_id,
            bool only_public, int page, int page_size) {
        return repository::FurryCoreRepository::Instance().GetWorlds(
            user_id, viewer_id, only_public, page_size, (page - 1) * page_size);
    }

    std::optional<repository::WorldSettingEntity> GetWorld(int64_t world_id, const std::string& viewer_id) {
        return repository::FurryCoreRepository::Instance().GetWorld(world_id, viewer_id);
    }

    bool LikeWorld(const std::string& user_id, int64_t world_id, bool like) {
        return repository::FurryCoreRepository::Instance().LikeWorld(user_id, world_id, like);
    }

    bool DeleteWorld(const std::string& user_id, int64_t world_id) {
        return repository::FurryCoreRepository::Instance().DeleteWorld(user_id, world_id);
    }

    int64_t AddWorldPage(int64_t world_id, const std::string& user_id, const repository::WorldPageEntity& page) {
        return repository::FurryCoreRepository::Instance().AddWorldPage(world_id, user_id, page);
    }

    std::vector<repository::WorldPageEntity> GetWorldPages(int64_t world_id) {
        return repository::FurryCoreRepository::Instance().GetWorldPages(world_id);
    }

    bool DeleteWorldPage(int64_t world_id, const std::string& user_id, int64_t page_id) {
        return repository::FurryCoreRepository::Instance().DeleteWorldPage(world_id, user_id, page_id);
    }

    int64_t PurchaseTicket(const std::string& user_id, int64_t event_id,
                           const std::string& ticket_type, int32_t price) {
        if (price < 0) return 0;
        return repository::FurryCoreRepository::Instance().PurchaseTicket(
            user_id, event_id, ticket_type, price);
    }

    std::vector<repository::EventTicketEntity> GetUserTickets(const std::string& user_id) {
        return repository::FurryCoreRepository::Instance().GetUserTickets(user_id);
    }

    bool CheckInTicket(const std::string& checker_id, int64_t ticket_id, bool is_admin) {
        return repository::FurryCoreRepository::Instance().CheckInTicket(checker_id, ticket_id, is_admin);
    }

    std::vector<repository::EventTicketEntity> GetEventAttendees(int64_t event_id, bool only_checked_in) {
        return repository::FurryCoreRepository::Instance().GetEventAttendees(event_id, only_checked_in);
    }

    int64_t CreateMarketItem(const std::string& user_id, const repository::MarketItemEntity& item) {
        if (item.price < 0 || item.title.empty()) return 0;
        return repository::FurryCoreRepository::Instance().CreateMarketItem(user_id, item);
    }

    bool UpdateMarketItem(const std::string& user_id, int64_t item_id, const repository::MarketItemEntity& item) {
        return repository::FurryCoreRepository::Instance().UpdateMarketItem(user_id, item_id, item);
    }

    std::vector<repository::MarketItemEntity> GetMarketItems(
            const std::string& category, const std::string& keyword,
            const std::string& user_id, int page, int page_size) {
        return repository::FurryCoreRepository::Instance().GetMarketItems(
            category, keyword, user_id, page_size, (page - 1) * page_size);
    }

    std::optional<repository::MarketItemEntity> GetMarketItem(int64_t item_id, const std::string& viewer_id) {
        return repository::FurryCoreRepository::Instance().GetMarketItem(item_id, viewer_id);
    }

    bool FavoriteMarketItem(const std::string& user_id, int64_t item_id, bool favorite) {
        return repository::FurryCoreRepository::Instance().FavoriteMarketItem(user_id, item_id, favorite);
    }

    bool DeleteMarketItem(const std::string& user_id, int64_t item_id) {
        return repository::FurryCoreRepository::Instance().DeleteMarketItem(user_id, item_id);
    }

    int64_t CreateTransaction(int64_t item_id, const std::string& buyer_id,
                              const std::string& buyer_contact) {
        if (buyer_contact.empty()) return 0;
        return repository::FurryCoreRepository::Instance().CreateTransaction(
            item_id, buyer_id, buyer_contact);
    }

    bool ConfirmTransaction(const std::string& user_id, int64_t transaction_id, const std::string& status) {
        return repository::FurryCoreRepository::Instance().ConfirmTransaction(user_id, transaction_id, status);
    }

    std::vector<repository::MarketTransactionEntity> GetUserTransactions(const std::string& user_id) {
        return repository::FurryCoreRepository::Instance().GetUserTransactions(user_id);
    }

private:
    FurryCoreService() {
        srand(time(0));
    }
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_FURRY_CORE_SERVICE_H
