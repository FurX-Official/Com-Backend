#ifndef FURBBS_REPOSITORY_CORE_REPOSITORY_H
#define FURBBS_REPOSITORY_CORE_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <map>

namespace furbbs::repository {

struct GalleryItemEntity {
    int64_t id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    int64_t fursona_id = 0;
    std::string title;
    std::string description;
    std::string file_url;
    std::string thumbnail_url;
    std::string file_type;
    int64_t file_size = 0;
    int32_t image_width = 0;
    int32_t image_height = 0;
    std::string artist_name;
    std::string artist_url;
    std::vector<std::string> tags;
    bool is_public = true;
    bool is_nsfw = false;
    bool is_liked = false;
    int32_t view_count = 0;
    int32_t like_count = 0;
    int32_t comment_count = 0;
    int64_t created_at = 0;
};

struct GalleryAlbumEntity {
    int64_t id = 0;
    std::string user_id;
    std::string title;
    std::string description;
    std::string cover_image;
    bool is_public = true;
    int32_t item_count = 0;
    int64_t created_at = 0;
};

struct SearchResultEntity {
    int64_t id = 0;
    std::string content_type;
    int64_t content_id = 0;
    std::string title;
    std::string content_text;
    std::string author_id;
    std::vector<std::string> tags;
    float rank = 0;
    int32_t weight = 0;
    int64_t updated_at = 0;
};

struct UserPresenceEntity {
    std::string user_id;
    std::string status;
    int64_t last_active = 0;
    bool is_invisible = false;
};

struct ExportTaskEntity {
    int64_t id = 0;
    std::string user_id;
    std::string task_type;
    std::string status;
    std::string file_url;
    int64_t file_size = 0;
    int64_t expires_at = 0;
    int64_t created_at = 0;
    int64_t completed_at = 0;
};

struct SystemConfigEntity {
    std::string config_key;
    std::string config_value;
    std::string config_type;
    std::string description;
    bool is_public = false;
    int64_t updated_at = 0;
};

struct AuditLogEntity {
    int64_t id = 0;
    std::string user_id;
    std::string action;
    std::string resource_type;
    int64_t resource_id = 0;
    std::string old_value;
    std::string new_value;
    std::string ip_address;
    std::string user_agent;
    int64_t created_at = 0;
};

class CoreRepository : protected BaseRepository {
public:
    static CoreRepository& Instance() {
        static CoreRepository instance;
        return instance;
    }

    int64_t CreateGalleryItem(const GalleryItemEntity& data);
    std::vector<GalleryItemEntity> GetUserGallery(
        const std::string& user_id, bool is_owner, int limit, int offset);
    std::optional<GalleryItemEntity> GetGalleryItem(int64_t id, const std::string& viewer_id);
    bool LikeGalleryItem(int64_t id, const std::string& user_id, bool like);
    bool DeleteGalleryItem(int64_t id, const std::string& user_id);

    int64_t CreateAlbum(const GalleryAlbumEntity& data);
    bool AddAlbumItem(int64_t album_id, int64_t item_id, int sort_order);
    std::vector<GalleryAlbumEntity> GetUserAlbums(
        const std::string& user_id, bool is_owner);

    bool UpsertSearchIndex(const SearchResultEntity& data);
    std::vector<SearchResultEntity> SearchFulltext(
        const std::string& query, int limit, int offset);

    bool UpdatePresence(const UserPresenceEntity& data);
    std::optional<UserPresenceEntity> GetPresence(const std::string& user_id);
    std::vector<UserPresenceEntity> GetOnlineUsers(int limit);

    int64_t CreateExportTask(const std::string& user_id, const std::string& task_type);
    std::vector<ExportTaskEntity> GetUserExportTasks(const std::string& user_id);

    std::vector<SystemConfigEntity> GetAllConfigs();
    bool SetConfig(const SystemConfigEntity& data);

    bool AddAuditLog(const AuditLogEntity& data);
    std::vector<AuditLogEntity> GetUserAuditLogs(
        const std::string& user_id, int limit, int offset);

private:
    CoreRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_CORE_REPOSITORY_H
