#include "furry_core_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

int64_t FurryCoreRepository::CreatePrompt(const std::string& user_id, const AIPromptEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::AI_CREATE_PROMPT,
            user_id, data.fursona_id, data.prompt_type, data.title,
            data.prompt, data.model, data.style_tags, data.is_public);
        return r[0][0].as<int64_t>();
    });
}

std::vector<AIPromptEntity> FurryCoreRepository::GetPrompts(
        const std::string& user_id, const std::string& viewer_id,
        bool only_public, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r;
        if (!user_id.empty()) {
            bool can_view_private = (user_id == viewer_id);
            r = tx.exec_params(sql::AI_GET_PROMPTS_USER, user_id, can_view_private, limit, offset);
        } else {
            r = tx.exec_params(sql::AI_GET_PROMPTS_PUBLIC, limit, offset);
        }
        return MapResults<AIPromptEntity>(r, [](const pqxx::row& row, AIPromptEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.fursona_id = row["fursona_id"].as<int64_t>();
            e.prompt_type = row["prompt_type"].as<std::string>();
            e.title = row["title"].as<std::string>();
            e.prompt = row["prompt"].as<std::string>();
            e.model = row["model"].as<std::string>();
            e.style_tags = ParseArray(row["style_tags"]);
            e.use_count = row["use_count"].as<int32_t>();
            e.is_public = row["is_public"].as<bool>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

int FurryCoreRepository::GetPromptCount(const std::string& user_id, bool only_public) {
    return ExecuteScalar<int>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::AI_COUNT_PROMPTS, user_id, only_public);
        return r[0][0].as<int>();
    });
}

bool FurryCoreRepository::DeletePrompt(const std::string& user_id, int64_t prompt_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::AI_DELETE_PROMPT, prompt_id, user_id);
        return r.affected_rows() > 0;
    });
}

int64_t FurryCoreRepository::CreateRelation(const std::string& user_id, const FursonaRelationEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CP_CREATE_RELATION,
            data.fursona_a_id, data.fursona_b_id, data.relation_type, data.anniversary);
        return r[0][0].as<int64_t>();
    });
}

bool FurryCoreRepository::ConfirmRelation(const std::string& user_id, int64_t relation_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CP_CONFIRM_RELATION, relation_id, user_id);
        return r.affected_rows() > 0;
    });
}

std::vector<FursonaRelationEntity> FurryCoreRepository::GetFursonaRelations(int64_t fursona_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CP_GET_RELATIONS, fursona_id);
        return MapResults<FursonaRelationEntity>(r, [](const pqxx::row& row, FursonaRelationEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.fursona_a_id = row["fursona_a_id"].as<int64_t>();
            e.fursona_a_name = row["fursona_a_name"].as<std::string>();
            e.fursona_a_owner = row["owner_a"].as<std::string>();
            e.fursona_b_id = row["fursona_b_id"].as<int64_t>();
            e.fursona_b_name = row["fursona_b_name"].as<std::string>();
            e.fursona_b_owner = row["owner_b"].as<std::string>();
            e.relation_type = row["relation_type"].as<std::string>();
            e.user_a_confirmed = row["user_a_confirmed"].as<bool>();
            e.user_b_confirmed = row["user_b_confirmed"].as<bool>();
            e.anniversary = row["anniversary"].as<int64_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool FurryCoreRepository::DeleteRelation(const std::string& user_id, int64_t relation_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CP_DELETE_RELATION, relation_id, user_id);
        return r.affected_rows() > 0;
    });
}

int64_t FurryCoreRepository::CreateWorld(const std::string& user_id, const WorldSettingEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_CREATE,
            user_id, data.name, data.description, data.cover_image,
            data.setting_type, data.tags, data.is_public);
        return r[0][0].as<int64_t>();
    });
}

bool FurryCoreRepository::UpdateWorld(const std::string& user_id, int64_t world_id, const WorldSettingEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_UPDATE,
            world_id, user_id, data.name, data.description,
            data.cover_image, data.setting_type, data.tags, data.is_public);
        return r.affected_rows() > 0;
    });
}

std::vector<WorldSettingEntity> FurryCoreRepository::GetWorlds(
        const std::string& user_id, const std::string& viewer_id,
        bool only_public, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r;
        bool can_view_private = (user_id == viewer_id);
        if (!user_id.empty()) {
            r = tx.exec_params(sql::WORLD_GET_BY_USER, user_id, can_view_private, limit, offset);
        } else {
            r = tx.exec_params(sql::WORLD_GET_PUBLIC, limit, offset);
        }
        return MapResults<WorldSettingEntity>(r, [&](const pqxx::row& row, WorldSettingEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.name = row["name"].as<std::string>();
            e.description = row["description"].as<std::string>();
            e.cover_image = row["cover_image"].as<std::string>();
            e.setting_type = row["setting_type"].as<std::string>();
            e.tags = ParseArray(row["tags"]);
            e.is_public = row["is_public"].as<bool>();
            e.view_count = row["view_count"].as<int32_t>();
            e.like_count = row["like_count"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

int FurryCoreRepository::GetWorldCount(const std::string& user_id, bool only_public) {
    return ExecuteScalar<int>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_COUNT, user_id, only_public);
        return r[0][0].as<int>();
    });
}

std::optional<WorldSettingEntity> FurryCoreRepository::GetWorld(int64_t world_id, const std::string& viewer_id) {
    return ExecuteQueryOptional<WorldSettingEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_GET_BY_ID, world_id);
        if (r.empty()) return std::optional<WorldSettingEntity>();
        const auto& row = r[0];
        WorldSettingEntity e;
        e.id = row["id"].as<int64_t>();
        e.user_id = row["user_id"].as<std::string>();
        e.name = row["name"].as<std::string>();
        e.description = row["description"].as<std::string>();
        e.cover_image = row["cover_image"].as<std::string>();
        e.setting_type = row["setting_type"].as<std::string>();
        e.tags = ParseArray(row["tags"]);
        e.is_public = row["is_public"].as<bool>();
        e.view_count = row["view_count"].as<int32_t>();
        e.like_count = row["like_count"].as<int32_t>();
        e.created_at = row["created_at"].as<int64_t>();
        
        if (!viewer_id.empty()) {
            pqxx::result lr = tx.exec_params(sql::WORLD_CHECK_LIKE, world_id, viewer_id);
            e.is_liked = !lr.empty();
        }
        return e;
    });
}

bool FurryCoreRepository::LikeWorld(const std::string& user_id, int64_t world_id, bool like) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        if (like) {
            tx.exec_params(sql::WORLD_LIKE, world_id, user_id);
        } else {
            tx.exec_params(sql::WORLD_UNLIKE, world_id, user_id);
        }
        return true;
    });
}

bool FurryCoreRepository::DeleteWorld(const std::string& user_id, int64_t world_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_DELETE, world_id, user_id);
        return r.affected_rows() > 0;
    });
}

int64_t FurryCoreRepository::AddWorldPage(int64_t world_id, const std::string& user_id, const WorldPageEntity& page) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_ADD_PAGE,
            world_id, page.title, page.content, page.page_type,
            page.parent_id, page.sort_order);
        return r[0][0].as<int64_t>();
    });
}

std::vector<WorldPageEntity> FurryCoreRepository::GetWorldPages(int64_t world_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_GET_PAGES, world_id);
        return MapResults<WorldPageEntity>(r, [](const pqxx::row& row, WorldPageEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.world_id = row["world_id"].as<int64_t>();
            e.title = row["title"].as<std::string>();
            e.content = row["content"].as<std::string>();
            e.page_type = row["page_type"].as<std::string>();
            e.parent_id = row["parent_id"].as<int64_t>();
            e.sort_order = row["sort_order"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool FurryCoreRepository::DeleteWorldPage(int64_t world_id, const std::string& user_id, int64_t page_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WORLD_DELETE_PAGE, page_id, world_id, user_id);
        return r.affected_rows() > 0;
    });
}

int64_t FurryCoreRepository::PurchaseTicket(const std::string& user_id, int64_t event_id,
                                             const std::string& ticket_type, int32_t price) {
    return ExecuteInsert([&](pqxx::work& tx) {
        tx.exec_params(sql::POINT_DEDUCT, user_id, price);
        std::string qr = "TICKET_" + std::to_string(event_id) + "_" + user_id;
        pqxx::result r = tx.exec_params(sql::EVENT_BUY_TICKET,
            event_id, user_id, ticket_type, price, qr);
        return r[0][0].as<int64_t>();
    });
}

std::vector<EventTicketEntity> FurryCoreRepository::GetUserTickets(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::EVENT_GET_USER_TICKETS, user_id);
        return MapResults<EventTicketEntity>(r, [](const pqxx::row& row, EventTicketEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.event_id = row["event_id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.ticket_type = row["ticket_type"].as<std::string>();
            e.price_paid = row["price_paid"].as<int32_t>();
            e.checked_in = row["checked_in"].as<bool>();
            e.checked_in_at = row["checked_in_at"].as<int64_t>();
            e.qr_code = row["qr_code"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool FurryCoreRepository::CheckInTicket(const std::string& checker_id, int64_t ticket_id, bool is_admin) {
    if (!is_admin) return false;
    return ExecuteUpdate([&](pqxx::work& tx) {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        pqxx::result r = tx.exec_params(sql::EVENT_CHECKIN, ticket_id, ms);
        return r.affected_rows() > 0;
    });
}

std::vector<EventTicketEntity> FurryCoreRepository::GetEventAttendees(int64_t event_id, bool only_checked_in) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::EVENT_GET_ATTENDEES, event_id, only_checked_in);
        return MapResults<EventTicketEntity>(r, [](const pqxx::row& row, EventTicketEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.event_id = row["event_id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.username = row["username"].as<std::string>();
            e.ticket_type = row["ticket_type"].as<std::string>();
            e.checked_in = row["checked_in"].as<bool>();
            e.checked_in_at = row["checked_in_at"].as<int64_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

int64_t FurryCoreRepository::CreateMarketItem(const std::string& user_id, const MarketItemEntity& item) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MARKET_CREATE_ITEM,
            user_id, item.title, item.description, item.category,
            item.price, item.price_type, item.images, item.tags);
        return r[0][0].as<int64_t>();
    });
}

bool FurryCoreRepository::UpdateMarketItem(const std::string& user_id, int64_t item_id, const MarketItemEntity& item) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MARKET_UPDATE_ITEM,
            item_id, user_id, item.title, item.description, item.category,
            item.price, item.price_type, item.images, item.tags, item.status);
        return r.affected_rows() > 0;
    });
}

std::vector<MarketItemEntity> FurryCoreRepository::GetMarketItems(
        const std::string& category, const std::string& keyword,
        const std::string& user_id, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        std::string search = keyword.empty() ? "" : "%" + keyword + "%";
        pqxx::result r = tx.exec_params(sql::MARKET_GET_ITEMS,
            category, search, user_id, limit, offset);
        return MapResults<MarketItemEntity>(r, [&](const pqxx::row& row, MarketItemEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.username = row["username"].as<std::string>();
            e.avatar = row["avatar"].as<std::string>();
            e.title = row["title"].as<std::string>();
            e.description = row["description"].as<std::string>();
            e.category = row["category"].as<std::string>();
            e.price = row["price"].as<int32_t>();
            e.price_type = row["price_type"].as<std::string>();
            e.images = ParseArray(row["images"]);
            e.tags = ParseArray(row["tags"]);
            e.status = row["status"].as<std::string>();
            e.view_count = row["view_count"].as<int32_t>();
            e.favorite_count = row["favorite_count"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

int FurryCoreRepository::GetMarketItemCount(const std::string& category, const std::string& keyword) {
    return ExecuteScalar<int>([&](pqxx::work& tx) {
        std::string search = keyword.empty() ? "" : "%" + keyword + "%";
        pqxx::result r = tx.exec_params(sql::MARKET_COUNT_ITEMS, category, search);
        return r[0][0].as<int>();
    });
}

std::optional<MarketItemEntity> FurryCoreRepository::GetMarketItem(int64_t item_id, const std::string& viewer_id) {
    return ExecuteQueryOptional<MarketItemEntity>([&](pqxx::work& tx) {
        tx.exec_params(sql::MARKET_INC_VIEW, item_id);
        pqxx::result r = tx.exec_params(sql::MARKET_GET_ITEM, item_id);
        if (r.empty()) return std::optional<MarketItemEntity>();
        const auto& row = r[0];
        MarketItemEntity e;
        e.id = row["id"].as<int64_t>();
        e.user_id = row["user_id"].as<std::string>();
        e.username = row["username"].as<std::string>();
        e.avatar = row["avatar"].as<std::string>();
        e.title = row["title"].as<std::string>();
        e.description = row["description"].as<std::string>();
        e.category = row["category"].as<std::string>();
        e.price = row["price"].as<int32_t>();
        e.price_type = row["price_type"].as<std::string>();
        e.images = ParseArray(row["images"]);
        e.tags = ParseArray(row["tags"]);
        e.status = row["status"].as<std::string>();
        e.view_count = row["view_count"].as<int32_t>();
        e.favorite_count = row["favorite_count"].as<int32_t>();
        e.created_at = row["created_at"].as<int64_t>();
        
        if (!viewer_id.empty()) {
            pqxx::result fr = tx.exec_params(sql::MARKET_CHECK_FAVORITE, item_id, viewer_id);
            e.is_favorited = !fr.empty();
        }
        return e;
    });
}

bool FurryCoreRepository::FavoriteMarketItem(const std::string& user_id, int64_t item_id, bool favorite) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        if (favorite) {
            tx.exec_params(sql::MARKET_ADD_FAVORITE, item_id, user_id);
        } else {
            tx.exec_params(sql::MARKET_DEL_FAVORITE, item_id, user_id);
        }
        return true;
    });
}

bool FurryCoreRepository::DeleteMarketItem(const std::string& user_id, int64_t item_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MARKET_DELETE_ITEM, item_id, user_id);
        return r.affected_rows() > 0;
    });
}

int64_t FurryCoreRepository::CreateTransaction(int64_t item_id, const std::string& buyer_id,
                                                const std::string& buyer_contact) {
    return ExecuteInsert([&](pqxx::work& tx) {
        auto seller = tx.exec_params(sql::MARKET_GET_SELLER, item_id);
        if (seller.empty()) return int64_t(0);
        std::string seller_id = seller[0][0].as<std::string>();
        int32_t price = seller[0][1].as<int32_t>();
        std::string price_type = seller[0][2].as<std::string>();
        
        if (price_type == "points") {
            tx.exec_params(sql::POINT_DEDUCT, buyer_id, price);
            tx.exec_params(sql::POINT_ADD, seller_id, price);
        }
        
        pqxx::result r = tx.exec_params(sql::MARKET_CREATE_TRANSACTION,
            item_id, seller_id, buyer_id, price, price_type, buyer_contact);
        tx.exec_params(sql::MARKET_SET_SOLD, item_id);
        return r[0][0].as<int64_t>();
    });
}

bool FurryCoreRepository::ConfirmTransaction(const std::string& user_id, int64_t transaction_id, const std::string& status) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MARKET_CONFIRM_TRANS, transaction_id, user_id, status);
        return r.affected_rows() > 0;
    });
}

std::vector<MarketTransactionEntity> FurryCoreRepository::GetUserTransactions(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MARKET_GET_TRANSACTIONS, user_id);
        return MapResults<MarketTransactionEntity>(r, [](const pqxx::row& row, MarketTransactionEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.item_id = row["item_id"].as<int64_t>();
            e.item_title = row["title"].as<std::string>();
            e.seller_id = row["seller_id"].as<std::string>();
            e.buyer_id = row["buyer_id"].as<std::string>();
            e.price = row["price"].as<int32_t>();
            e.price_type = row["price_type"].as<std::string>();
            e.status = row["status"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

} // namespace furbbs::repository
