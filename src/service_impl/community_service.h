#ifndef FURBBS_SERVICE_IMPL_COMMUNITY_SERVICE_H
#define FURBBS_SERVICE_IMPL_COMMUNITY_SERVICE_H

#include "../repository/community_repository.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::service {

class CommunityService {
public:
    static CommunityService& Instance() {
        static CommunityService instance;
        return instance;
    }

    int64_t SaveCard(const std::string& user_id, const repository::FursonaCardEntity& data) {
        return repository::CommunityRepository::Instance().SaveCard(user_id, data);
    }

    std::optional<repository::FursonaCardEntity> GetCard(int64_t fursona_id) {
        repository::CommunityRepository::Instance().IncrementCardView(fursona_id);
        return repository::CommunityRepository::Instance().GetCard(fursona_id);
    }

    bool SetContentRating(const repository::ContentRatingEntity& data) {
        return repository::CommunityRepository::Instance().SetContentRating(data);
    }

    std::optional<repository::ContentRatingEntity> GetContentRating(
            const std::string& content_type, int64_t content_id) {
        return repository::CommunityRepository::Instance().GetContentRating(content_type, content_id);
    }

    bool UpdateContentPrefs(const repository::ContentPrefsEntity& data) {
        return repository::CommunityRepository::Instance().UpdateContentPrefs(data);
    }

    std::optional<repository::ContentPrefsEntity> GetContentPrefs(const std::string& user_id) {
        auto result = repository::CommunityRepository::Instance().GetContentPrefs(user_id);
        if (!result) {
            repository::ContentPrefsEntity default_prefs;
            default_prefs.user_id = user_id;
            return default_prefs;
        }
        return result;
    }

    int64_t RequestPermission(const repository::CreationPermissionEntity& data) {
        if (data.author_user_id == data.authorized_user_id) {
            return 0;
        }
        return repository::CommunityRepository::Instance().RequestPermission(data);
    }

    bool ApprovePermission(const std::string& author_id, int64_t permission_id) {
        return repository::CommunityRepository::Instance().ApprovePermission(author_id, permission_id);
    }

    std::vector<repository::CreationPermissionEntity> GetUserPermissions(const std::string& user_id) {
        return repository::CommunityRepository::Instance().GetUserPermissions(user_id);
    }

    bool AddInteraction(const repository::FursonaInteractionEntity& data) {
        if (data.from_fursona_id == data.to_fursona_id) {
            return false;
        }
        return repository::CommunityRepository::Instance().AddInteraction(data);
    }

    std::vector<repository::FursonaInteractionEntity> GetFursonaInteractions(int64_t fursona_id) {
        return repository::CommunityRepository::Instance().GetFursonaInteractions(fursona_id);
    }

    bool SubmitToModeration(const repository::ModerationItemEntity& data) {
        return repository::CommunityRepository::Instance().SubmitToModeration(data);
    }

    bool ReviewModeration(const repository::ModerationItemEntity& data) {
        return repository::CommunityRepository::Instance().ReviewModeration(data);
    }

    std::vector<repository::ModerationItemEntity> GetModerationQueue(
            const std::string& status, int page, int page_size) {
        return repository::CommunityRepository::Instance().GetModerationQueue(
            status, page_size, (page - 1) * page_size);
    }

private:
    CommunityService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_COMMUNITY_SERVICE_H
