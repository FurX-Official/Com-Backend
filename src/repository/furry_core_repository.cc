#include "furry_core_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

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

} // namespace furbbs::repository
