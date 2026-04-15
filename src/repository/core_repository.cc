#include "core_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

int64_t CoreRepository::CreateGalleryItem(const GalleryItemEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::GALLERY_CREATE_ITEM,
            data.user_id, data.fursona_id, data.title, data.description,
            data.file_url, data.thumbnail_url, data.file_type, data.file_size,
            data.image_width, data.image_height, data.artist_name, data.artist_url,
            data.tags, data.is_public, data.is_nsfw);
        return r[0][0].as<int64_t>();
    });
}

std::vector<GalleryItemEntity> CoreRepository::GetUserGallery(
        const std::string& user_id, bool is_owner, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::GALLERY_GET_BY_USER,
            user_id, is_owner, limit, offset);
        return MapResults<GalleryItemEntity>(r, [](const pqxx::row& row, GalleryItemEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.username = row["username"].as<std::string>();
            e.fursona_id = row["fursona_id"].as<int64_t>();
            e.title = row["title"].as<std::string>();
            e.description = row["description"].as<std::string>();
            e.file_url = row["file_url"].as<std::string>();
            e.thumbnail_url = row["thumbnail_url"].as<std::string>();
            e.file_type = row["file_type"].as<std::string>();
            e.is_public = row["is_public"].as<bool>();
            e.is_nsfw = row["is_nsfw"].as<bool>();
            e.view_count = row["view_count"].as<int32_t>();
            e.like_count = row["like_count"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

std::optional<GalleryItemEntity> CoreRepository::GetGalleryItem(
        int64_t id, const std::string& viewer_id) {
    return ExecuteQueryOne<GalleryItemEntity>([&](pqxx::work& tx) {
        tx.exec_params(sql::GALLERY_INC_VIEW, id);
        pqxx::result r = tx.exec_params(sql::GALLERY_GET_BY_ID, id);
        if (r.empty()) return std::optional<GalleryItemEntity>();
        GalleryItemEntity e;
        MapRow(r[0], e, [&viewer_id](const pqxx::row& row, GalleryItemEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.username = row["username"].as<std::string>();
            e.title = row["title"].as<std::string>();
            e.file_url = row["file_url"].as<std::string>();
            e.thumbnail_url = row["thumbnail_url"].as<std::string>();
            e.artist_name = row["artist_name"].as<std::string>();
            e.view_count = row["view_count"].as<int32_t>();
            e.like_count = row["like_count"].as<int32_t>();
        });
        return e;
    });
}

bool CoreRepository::LikeGalleryItem(int64_t id, const std::string& user_id, bool like) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        if (like) {
            tx.exec_params(sql::GALLERY_LIKE, id, user_id);
        } else {
            tx.exec_params(sql::GALLERY_UNLIKE, id, user_id);
        }
        return true;
    });
}

bool CoreRepository::DeleteGalleryItem(int64_t id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::GALLERY_DELETE, id, user_id);
        return r.affected_rows() > 0;
    });
}

int64_t CoreRepository::CreateAlbum(const GalleryAlbumEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::ALBUM_CREATE,
            data.user_id, data.title, data.description,
            data.cover_image, data.is_public);
        return r[0][0].as<int64_t>();
    });
}

bool CoreRepository::AddAlbumItem(int64_t album_id, int64_t item_id, int sort_order) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::ALBUM_ADD_ITEM, album_id, item_id, sort_order);
        return true;
    });
}

std::vector<GalleryAlbumEntity> CoreRepository::GetUserAlbums(
        const std::string& user_id, bool is_owner) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::ALBUM_GET_BY_USER, user_id, is_owner);
        return MapResults<GalleryAlbumEntity>(r, [](const pqxx::row& row, GalleryAlbumEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.title = row["title"].as<std::string>();
            e.description = row["description"].as<std::string>();
            e.cover_image = row["cover_image"].as<std::string>();
            e.item_count = row["item_count"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool CoreRepository::UpsertSearchIndex(const SearchResultEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SEARCH_INDEX_UPSERT,
            data.content_type, data.content_id, data.title,
            data.content_text, data.author_id, data.tags,
            data.is_public, data.weight);
        return true;
    });
}

std::vector<SearchResultEntity> CoreRepository::SearchFulltext(
        const std::string& query, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::SEARCH_FULLTEXT, query, limit, offset);
        return MapResults<SearchResultEntity>(r, [](const pqxx::row& row, SearchResultEntity& e) {
            e.content_type = row["content_type"].as<std::string>();
            e.content_id = row["content_id"].as<int64_t>();
            e.title = row["title"].as<std::string>();
            e.rank = row["rank"].as<float>();
            e.weight = row["weight"].as<int32_t>();
        });
    });
}

bool CoreRepository::UpdatePresence(const UserPresenceEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::PRESENCE_UPDATE,
            data.user_id, data.status, "", "");
        return true;
    });
}

std::optional<UserPresenceEntity> CoreRepository::GetPresence(const std::string& user_id) {
    return ExecuteQueryOne<UserPresenceEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::PRESENCE_GET, user_id);
        if (r.empty()) return std::optional<UserPresenceEntity>();
        UserPresenceEntity e;
        e.user_id = r[0]["user_id"].as<std::string>();
        e.status = r[0]["status"].as<std::string>();
        e.last_active = r[0]["last_active"].as<int64_t>();
        return e;
    });
}

std::vector<UserPresenceEntity> CoreRepository::GetOnlineUsers(int limit) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::PRESENCE_GET_ONLINE, limit);
        return MapResults<UserPresenceEntity>(r, [](const pqxx::row& row, UserPresenceEntity& e) {
            e.user_id = row["user_id"].as<std::string>();
            e.status = row["status"].as<std::string>();
            e.last_active = row["last_active"].as<int64_t>();
        });
    });
}

int64_t CoreRepository::CreateExportTask(const std::string& user_id, const std::string& task_type) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::EXPORT_CREATE_TASK, user_id, task_type);
        return r[0][0].as<int64_t>();
    });
}

std::vector<ExportTaskEntity> CoreRepository::GetUserExportTasks(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::EXPORT_GET_TASKS, user_id);
        return MapResults<ExportTaskEntity>(r, [](const pqxx::row& row, ExportTaskEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.task_type = row["task_type"].as<std::string>();
            e.status = row["status"].as<std::string>();
            e.file_url = row["file_url"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

std::vector<SystemConfigEntity> CoreRepository::GetAllConfigs() {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CONFIG_GET_ALL);
        return MapResults<SystemConfigEntity>(r, [](const pqxx::row& row, SystemConfigEntity& e) {
            e.config_key = row["config_key"].as<std::string>();
            e.config_value = row["config_value"].as<std::string>();
            e.config_type = row["config_type"].as<std::string>();
            e.is_public = row["is_public"].as<bool>();
        });
    });
}

bool CoreRepository::SetConfig(const SystemConfigEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::CONFIG_SET,
            data.config_key, data.config_value,
            data.config_type, data.description);
        return true;
    });
}

bool CoreRepository::AddAuditLog(const AuditLogEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::AUDIT_LOG,
            data.user_id, data.action, data.resource_type,
            data.resource_id, data.old_value, data.new_value,
            data.ip_address, data.user_agent);
        return true;
    });
}

std::vector<AuditLogEntity> CoreRepository::GetUserAuditLogs(
        const std::string& user_id, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::AUDIT_GET_BY_USER, user_id, limit, offset);
        return MapResults<AuditLogEntity>(r, [](const pqxx::row& row, AuditLogEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.action = row["action"].as<std::string>();
            e.resource_type = row["resource_type"].as<std::string>();
            e.resource_id = row["resource_id"].as<int64_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

} // namespace furbbs::repository
