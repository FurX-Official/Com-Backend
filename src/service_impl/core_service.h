#ifndef FURBBS_SERVICE_IMPL_CORE_SERVICE_H
#define FURBBS_SERVICE_IMPL_CORE_SERVICE_H

#include "../repository/core_repository.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::service {

class CoreService {
public:
    static CoreService& Instance() {
        static CoreService instance;
        return instance;
    }

    int64_t CreateGalleryItem(const repository::GalleryItemEntity& data) {
        auto id = repository::CoreRepository::Instance().CreateGalleryItem(data);
        if (id > 0) {
            repository::SearchResultEntity idx;
            idx.content_type = "gallery";
            idx.content_id = id;
            idx.title = data.title;
            idx.content_text = data.description;
            idx.author_id = data.user_id;
            idx.tags = data.tags;
            idx.is_public = data.is_public;
            idx.weight = 3;
            repository::CoreRepository::Instance().UpsertSearchIndex(idx);
        }
        return id;
    }

    std::vector<repository::GalleryItemEntity> GetUserGallery(
            const std::string& user_id, const std::string& viewer_id,
            int page, int page_size) {
        bool is_owner = (user_id == viewer_id);
        return repository::CoreRepository::Instance().GetUserGallery(
            user_id, is_owner, page_size, (page - 1) * page_size);
    }

    std::optional<repository::GalleryItemEntity> GetGalleryItem(
            int64_t id, const std::string& viewer_id) {
        return repository::CoreRepository::Instance().GetGalleryItem(id, viewer_id);
    }

    bool LikeGalleryItem(int64_t id, const std::string& user_id, bool like) {
        return repository::CoreRepository::Instance().LikeGalleryItem(id, user_id, like);
    }

    bool DeleteGalleryItem(int64_t id, const std::string& user_id) {
        return repository::CoreRepository::Instance().DeleteGalleryItem(id, user_id);
    }

    int64_t CreateAlbum(const repository::GalleryAlbumEntity& data) {
        return repository::CoreRepository::Instance().CreateAlbum(data);
    }

    bool AddAlbumItem(int64_t album_id, int64_t item_id, int sort_order = 0) {
        return repository::CoreRepository::Instance().AddAlbumItem(album_id, item_id, sort_order);
    }

    std::vector<repository::GalleryAlbumEntity> GetUserAlbums(
            const std::string& user_id, const std::string& viewer_id) {
        bool is_owner = (user_id == viewer_id);
        return repository::CoreRepository::Instance().GetUserAlbums(user_id, is_owner);
    }

    bool UpsertSearchIndex(const repository::SearchResultEntity& data) {
        return repository::CoreRepository::Instance().UpsertSearchIndex(data);
    }

    std::vector<repository::SearchResultEntity> Search(
            const std::string& query, int page, int page_size) {
        if (query.empty() || query.length() < 2) {
            return {};
        }
        return repository::CoreRepository::Instance().SearchFulltext(
            query, page_size, (page - 1) * page_size);
    }

    bool UpdatePresence(const std::string& user_id, const std::string& status) {
        repository::UserPresenceEntity data;
        data.user_id = user_id;
        data.status = status;
        return repository::CoreRepository::Instance().UpdatePresence(data);
    }

    std::optional<repository::UserPresenceEntity> GetPresence(const std::string& user_id) {
        return repository::CoreRepository::Instance().GetPresence(user_id);
    }

    std::vector<repository::UserPresenceEntity> GetOnlineUsers(int limit = 50) {
        return repository::CoreRepository::Instance().GetOnlineUsers(limit);
    }

    int64_t CreateExportTask(const std::string& user_id, const std::string& task_type) {
        return repository::CoreRepository::Instance().CreateExportTask(user_id, task_type);
    }

    std::vector<repository::ExportTaskEntity> GetUserExportTasks(const std::string& user_id) {
        return repository::CoreRepository::Instance().GetUserExportTasks(user_id);
    }

    std::vector<repository::SystemConfigEntity> GetPublicConfigs() {
        auto all = repository::CoreRepository::Instance().GetAllConfigs();
        std::vector<repository::SystemConfigEntity> result;
        for (auto& c : all) {
            if (c.is_public) {
                result.push_back(c);
            }
        }
        return result;
    }

    bool SetConfig(const repository::SystemConfigEntity& data) {
        return repository::CoreRepository::Instance().SetConfig(data);
    }

    bool AddAuditLog(const repository::AuditLogEntity& data) {
        return repository::CoreRepository::Instance().AddAuditLog(data);
    }

    std::vector<repository::AuditLogEntity> GetUserAuditLogs(
            const std::string& user_id, int page, int page_size) {
        return repository::CoreRepository::Instance().GetUserAuditLogs(
            user_id, page_size, (page - 1) * page_size);
    }

    void LogAction(const std::string& user_id, const std::string& action,
                   const std::string& resource_type = "", int64_t resource_id = 0) {
        repository::AuditLogEntity log;
        log.user_id = user_id;
        log.action = action;
        log.resource_type = resource_type;
        log.resource_id = resource_id;
        AddAuditLog(log);
    }

private:
    CoreService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_CORE_SERVICE_H
