#ifndef FURBBS_REPOSITORY_FURRY_CORE_REPOSITORY_H
#define FURBBS_REPOSITORY_FURRY_CORE_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <map>

namespace furbbs::repository {

struct AIPromptEntity {
    int64_t id = 0;
    std::string user_id;
    std::string username;
    int64_t fursona_id = 0;
    std::string prompt_type;
    std::string title;
    std::string prompt;
    std::string model;
    std::vector<std::string> style_tags;
    int32_t use_count = 0;
    bool is_public = false;
    int64_t created_at = 0;
};

struct FursonaRelationEntity {
    int64_t id = 0;
    int64_t fursona_a_id = 0;
    std::string fursona_a_name;
    std::string fursona_a_avatar;
    std::string fursona_a_owner;
    int64_t fursona_b_id = 0;
    std::string fursona_b_name;
    std::string fursona_b_avatar;
    std::string fursona_b_owner;
    std::string relation_type;
    bool user_a_confirmed = false;
    bool user_b_confirmed = false;
    int64_t anniversary = 0;
    std::map<std::string, std::string> relation_data;
    int64_t created_at = 0;
};

struct WorldSettingEntity {
    int64_t id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    std::string name;
    std::string description;
    std::string cover_image;
    std::string setting_type;
    std::vector<std::string> tags;
    bool is_public = false;
    int32_t view_count = 0;
    int32_t like_count = 0;
    bool is_liked = false;
    int64_t created_at = 0;
};

struct WorldPageEntity {
    int64_t id = 0;
    int64_t world_id = 0;
    std::string title;
    std::string content;
    std::string page_type;
    int64_t parent_id = 0;
    int32_t sort_order = 0;
    int64_t created_at = 0;
};

struct EventTicketEntity {
    int64_t id = 0;
    int64_t event_id = 0;
    std::string user_id;
    std::string username;
    std::string ticket_type;
    int32_t price_paid = 0;
    bool checked_in = false;
    int64_t checked_in_at = 0;
    std::string qr_code;
    int64_t created_at = 0;
};

struct MarketItemEntity {
    int64_t id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    std::string title;
    std::string description;
    std::string category;
    int32_t price = 0;
    std::string price_type;
    std::vector<std::string> images;
    std::vector<std::string> tags;
    std::string status;
    int32_t view_count = 0;
    int32_t favorite_count = 0;
    bool is_favorited = false;
    int64_t created_at = 0;
};

struct MarketTransactionEntity {
    int64_t id = 0;
    int64_t item_id = 0;
    std::string item_title;
    std::string seller_id;
    std::string seller_name;
    std::string buyer_id;
    std::string buyer_name;
    int32_t price = 0;
    std::string price_type;
    std::string status;
    std::string buyer_contact;
    std::string seller_contact;
    int64_t created_at = 0;
};

class FurryCoreRepository : protected BaseRepository {
public:
    static FurryCoreRepository& Instance() {
        static FurryCoreRepository instance;
        return instance;
    }

    int64_t CreatePrompt(const std::string& user_id, const AIPromptEntity& data);
    std::vector<AIPromptEntity> GetPrompts(const std::string& user_id,
                                             const std::string& viewer_id,
                                             bool only_public, int limit, int offset);
    int GetPromptCount(const std::string& user_id, bool only_public);
    bool DeletePrompt(const std::string& user_id, int64_t prompt_id);

    int64_t CreateRelation(const std::string& user_id, const FursonaRelationEntity& data);
    bool ConfirmRelation(const std::string& user_id, int64_t relation_id);
    std::vector<FursonaRelationEntity> GetFursonaRelations(int64_t fursona_id);
    bool DeleteRelation(const std::string& user_id, int64_t relation_id);

    int64_t CreateWorld(const std::string& user_id, const WorldSettingEntity& data);
    bool UpdateWorld(const std::string& user_id, int64_t world_id, const WorldSettingEntity& data);
    std::vector<WorldSettingEntity> GetWorlds(const std::string& user_id,
                                               const std::string& viewer_id,
                                               bool only_public, int limit, int offset);
    int GetWorldCount(const std::string& user_id, bool only_public);
    std::optional<WorldSettingEntity> GetWorld(int64_t world_id, const std::string& viewer_id);
    bool LikeWorld(const std::string& user_id, int64_t world_id, bool like);
    bool DeleteWorld(const std::string& user_id, int64_t world_id);

    int64_t AddWorldPage(int64_t world_id, const std::string& user_id, const WorldPageEntity& page);
    std::vector<WorldPageEntity> GetWorldPages(int64_t world_id);
    bool DeleteWorldPage(int64_t world_id, const std::string& user_id, int64_t page_id);

    int64_t PurchaseTicket(const std::string& user_id, int64_t event_id,
                           const std::string& ticket_type, int32_t price);
    std::vector<EventTicketEntity> GetUserTickets(const std::string& user_id);
    bool CheckInTicket(const std::string& checker_id, int64_t ticket_id, bool is_admin);
    std::vector<EventTicketEntity> GetEventAttendees(int64_t event_id, bool only_checked_in);

    int64_t CreateMarketItem(const std::string& user_id, const MarketItemEntity& item);
    bool UpdateMarketItem(const std::string& user_id, int64_t item_id, const MarketItemEntity& item);
    std::vector<MarketItemEntity> GetMarketItems(const std::string& category,
                                                  const std::string& keyword,
                                                  const std::string& user_id,
                                                  int limit, int offset);
    int GetMarketItemCount(const std::string& category, const std::string& keyword);
    std::optional<MarketItemEntity> GetMarketItem(int64_t item_id, const std::string& viewer_id);
    bool FavoriteMarketItem(const std::string& user_id, int64_t item_id, bool favorite);
    bool DeleteMarketItem(const std::string& user_id, int64_t item_id);

    int64_t CreateTransaction(int64_t item_id, const std::string& buyer_id,
                              const std::string& buyer_contact);
    bool ConfirmTransaction(const std::string& user_id, int64_t transaction_id, const std::string& status);
    std::vector<MarketTransactionEntity> GetUserTransactions(const std::string& user_id);

private:
    FurryCoreRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_FURRY_CORE_REPOSITORY_H
