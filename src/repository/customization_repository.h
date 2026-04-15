#ifndef FURBBS_REPOSITORY_CUSTOMIZATION_REPOSITORY_H
#define FURBBS_REPOSITORY_CUSTOMIZATION_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <map>
#include <optional>

namespace furbbs::repository {

struct ProfileCustomEntity {
    std::string theme;
    std::string bg_color;
    std::string bg_image;
    int32_t card_style = 0;
    int32_t layout_type = 0;
    bool show_fursona_first = true;
    bool show_badges = true;
    bool show_achievement = true;
    std::string music_url;
    std::string custom_css;
    std::vector<std::string> sidebar_widgets;
    int64_t updated_at = 0;
};

struct FursonaCardCustomEntity {
    int64_t fursona_id = 0;
    std::string card_theme;
    std::string border_color;
    int32_t bg_pattern = 0;
    std::string accent_color;
    int32_t font_style = 0;
    bool show_stats = true;
    bool show_artwork = true;
    std::map<std::string, std::string> custom_fields;
    int64_t updated_at = 0;
};

struct NotificationSettingsEntity {
    bool mention_email = true;
    bool mention_push = true;
    bool comment_email = true;
    bool comment_push = true;
    bool follow_email = true;
    bool follow_push = true;
    bool like_email = false;
    bool like_push = true;
    bool gift_email = true;
    bool gift_push = true;
    bool message_email = true;
    bool message_push = true;
    bool event_email = true;
    bool event_push = true;
    bool weekly_digest = true;
    bool marketing_email = false;
};

struct FeedSettingsEntity {
    std::string default_sort;
    bool show_avatars = true;
    bool show_signatures = true;
    bool compact_mode = false;
    int32_t posts_per_page = 20;
    bool auto_load_more = true;
    bool blur_nsfw = true;
    bool hide_nsfw = false;
    std::vector<std::string> blocked_tags;
    std::vector<std::string> blocked_users;
};

struct UserThemeEntity {
    int64_t id = 0;
    std::string name;
    std::string creator_id;
    std::string creator_name;
    std::string primary_color;
    std::string secondary_color;
    std::string accent_color;
    std::string bg_color;
    std::string card_bg_color;
    std::string text_color;
    bool is_public = false;
    int32_t use_count = 0;
    int64_t created_at = 0;
};

struct GroupCustomSettingsEntity {
    int64_t group_id = 0;
    std::string entry_message;
    bool require_approval = false;
    std::vector<std::string> approval_questions;
    std::string group_icon;
    std::string group_color;
    std::string custom_rules;
    bool allow_image_posts = true;
    bool allow_link_posts = true;
    int32_t post_cooldown = 0;
    bool mod_can_delete = true;
    bool mod_can_ban = false;
    bool visible_members = true;
};

class CustomizationRepository : protected BaseRepository {
public:
    static CustomizationRepository& Instance() {
        static CustomizationRepository instance;
        return instance;
    }

    ProfileCustomEntity GetProfileCustom(const std::string& user_id);
    void UpdateProfileCustom(const std::string& user_id, const ProfileCustomEntity& custom);

    FursonaCardCustomEntity GetFursonaCardCustom(int64_t fursona_id);
    void UpdateFursonaCardCustom(const std::string& owner_id, int64_t fursona_id,
                                 const FursonaCardCustomEntity& custom);

    NotificationSettingsEntity GetNotificationSettings(const std::string& user_id);
    void UpdateNotificationSettings(const std::string& user_id,
                                     const NotificationSettingsEntity& settings);

    FeedSettingsEntity GetFeedSettings(const std::string& user_id);
    void UpdateFeedSettings(const std::string& user_id, const FeedSettingsEntity& settings);

    int64_t CreateTheme(const std::string& creator_id, const UserThemeEntity& theme);
    std::vector<UserThemeEntity> GetThemes(bool only_public, const std::string& user_id,
                                           int limit, int offset);
    int GetThemeCount(bool only_public, const std::string& user_id);

    GroupCustomSettingsEntity GetGroupCustomSettings(int64_t group_id);
    bool UpdateGroupCustomSettings(const std::string& caller_id, int64_t group_id,
                                    const GroupCustomSettingsEntity& settings);

private:
    CustomizationRepository() = default;

    bool VerifyFursonaOwner(pqxx::work& txn, const std::string& user_id, int64_t fursona_id);
    bool VerifyGroupOwner(pqxx::work& txn, const std::string& user_id, int64_t group_id);
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_CUSTOMIZATION_REPOSITORY_H
