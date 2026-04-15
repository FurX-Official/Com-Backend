#ifndef FURBBS_REPOSITORY_COMMUNITY_REPOSITORY_H
#define FURBBS_REPOSITORY_COMMUNITY_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <map>

namespace furbbs::repository {

struct FursonaCardEntity {
    int64_t id = 0;
    int64_t fursona_id = 0;
    std::string user_id;
    std::string fursona_name;
    std::string species;
    std::string gender;
    std::string template_id = "default";
    std::string theme_color = "#6B9EFF";
    std::string background_image;
    bool show_stats = true;
    bool show_artist = true;
    std::string card_layout = "classic";
    std::string font_family;
    std::map<std::string, std::string> custom_fields;
    int32_t view_count = 0;
    int64_t created_at = 0;
    int64_t updated_at = 0;
};

struct ContentRatingEntity {
    int64_t id = 0;
    std::string content_type;
    int64_t content_id = 0;
    std::string user_id;
    std::string rating_level;
    std::vector<std::string> content_warnings;
    bool is_age_verified = false;
    std::string rated_by;
    int64_t rated_at = 0;
};

struct ContentPrefsEntity {
    std::string user_id;
    bool show_safe = true;
    bool show_questionable = false;
    bool show_explicit = false;
    std::vector<std::string> enabled_warnings;
    bool blur_sensitive = true;
    bool age_verified = false;
    int64_t updated_at = 0;
};

struct CreationPermissionEntity {
    int64_t id = 0;
    std::string author_user_id;
    std::string authorized_user_id;
    int64_t fursona_id = 0;
    std::string fursona_name;
    std::string permission_type;
    std::string terms;
    bool is_approved = false;
    int64_t expires_at = 0;
    int64_t created_at = 0;
};

struct FursonaInteractionEntity {
    int64_t id = 0;
    int64_t from_fursona_id = 0;
    int64_t to_fursona_id = 0;
    std::string fursona_name;
    std::string interaction_type;
    std::string user_note;
    int32_t intimacy_score = 0;
    int64_t created_at = 0;
};

struct ModerationItemEntity {
    int64_t id = 0;
    std::string content_type;
    int64_t content_id = 0;
    std::string submitter_id;
    std::string status = "pending";
    std::string moderator_id;
    std::string moderator_note;
    std::string violation_type;
    int64_t submitted_at = 0;
    int64_t reviewed_at = 0;
};

class CommunityRepository : protected BaseRepository {
public:
    static CommunityRepository& Instance() {
        static CommunityRepository instance;
        return instance;
    }

    int64_t SaveCard(const std::string& user_id, const FursonaCardEntity& data);
    std::optional<FursonaCardEntity> GetCard(int64_t fursona_id);
    bool IncrementCardView(int64_t fursona_id);

    bool SetContentRating(const ContentRatingEntity& data);
    std::optional<ContentRatingEntity> GetContentRating(
        const std::string& content_type, int64_t content_id);

    bool UpdateContentPrefs(const ContentPrefsEntity& data);
    std::optional<ContentPrefsEntity> GetContentPrefs(const std::string& user_id);

    int64_t RequestPermission(const CreationPermissionEntity& data);
    bool ApprovePermission(const std::string& author_id, int64_t permission_id);
    std::vector<CreationPermissionEntity> GetUserPermissions(const std::string& user_id);

    bool AddInteraction(const FursonaInteractionEntity& data);
    std::vector<FursonaInteractionEntity> GetFursonaInteractions(int64_t fursona_id);

    bool SubmitToModeration(const ModerationItemEntity& data);
    bool ReviewModeration(const ModerationItemEntity& data);
    std::vector<ModerationItemEntity> GetModerationQueue(
        const std::string& status, int limit, int offset);

private:
    CommunityRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_COMMUNITY_REPOSITORY_H
