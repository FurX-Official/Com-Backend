#include "customization_repository.h"
#include <chrono>
#include <nlohmann/json.hpp>

namespace furbbs::repository {

using json = nlohmann::json;

ProfileCustomEntity CustomizationRepository::GetProfileCustom(const std::string& user_id) {
    return Execute<ProfileCustomEntity>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT theme, bg_color, bg_image, card_style, layout_type,
                   show_fursona_first, show_badges, show_achievement,
                   music_url, custom_css, sidebar_widgets, updated_at
            FROM user_profile_custom WHERE user_id = $1
        )", user_id);

        ProfileCustomEntity custom;
        if (result.empty()) {
            custom.theme = "default";
            return custom;
        }

        const auto& row = result[0];
        if (!row[0].is_null()) custom.theme = row[0].as<std::string>();
        if (!row[1].is_null()) custom.bg_color = row[1].as<std::string>();
        if (!row[2].is_null()) custom.bg_image = row[2].as<std::string>();
        custom.card_style = row[3].as<int32_t>();
        custom.layout_type = row[4].as<int32_t>();
        custom.show_fursona_first = row[5].as<bool>();
        custom.show_badges = row[6].as<bool>();
        custom.show_achievement = row[7].as<bool>();
        if (!row[8].is_null()) custom.music_url = row[8].as<std::string>();
        if (!row[9].is_null()) custom.custom_css = row[9].as<std::string>();

        if (!row[10].is_null()) {
            pqxx::array_parser<std::string> parser(row[10].as<std::string>());
            std::vector<std::string> widgets;
            std::string widget;
            bool more;
            while ((more = parser.get_next(widget))) {
                if (!widget.empty()) widgets.push_back(widget);
            }
            custom.sidebar_widgets = widgets;
        }
        custom.updated_at = row[11].as<int64_t>();
        return custom;
    });
}

void CustomizationRepository::UpdateProfileCustom(const std::string& user_id,
                                                  const ProfileCustomEntity& custom) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        pqxx::array<std::string> widget_arr(custom.sidebar_widgets);
        txn.exec_params(R"(
            INSERT INTO user_profile_custom (user_id, theme, bg_color, bg_image,
                card_style, layout_type, show_fursona_first, show_badges,
                show_achievement, music_url, custom_css, sidebar_widgets, updated_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
            ON CONFLICT (user_id) DO UPDATE SET
                theme = $2, bg_color = $3, bg_image = $4,
                card_style = $5, layout_type = $6,
                show_fursona_first = $7, show_badges = $8,
                show_achievement = $9, music_url = $10, custom_css = $11,
                sidebar_widgets = $12, updated_at = $13
        )", user_id,
           custom.theme.empty() ? nullptr : &custom.theme,
           custom.bg_color.empty() ? nullptr : &custom.bg_color,
           custom.bg_image.empty() ? nullptr : &custom.bg_image,
           custom.card_style, custom.layout_type,
           custom.show_fursona_first, custom.show_badges,
           custom.show_achievement,
           custom.music_url.empty() ? nullptr : &custom.music_url,
           custom.custom_css.empty() ? nullptr : &custom.custom_css,
           custom.sidebar_widgets.empty() ? nullptr : &widget_arr,
           timestamp);
    });
}

FursonaCardCustomEntity CustomizationRepository::GetFursonaCardCustom(int64_t fursona_id) {
    return Execute<FursonaCardCustomEntity>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT card_theme, border_color, bg_pattern, accent_color,
                   font_style, show_stats, show_artwork, custom_fields, updated_at
            FROM fursona_card_custom WHERE fursona_id = $1
        )", fursona_id);

        FursonaCardCustomEntity custom;
        custom.fursona_id = fursona_id;
        if (result.empty()) {
            custom.card_theme = "classic";
            return custom;
        }

        const auto& row = result[0];
        if (!row[0].is_null()) custom.card_theme = row[0].as<std::string>();
        if (!row[1].is_null()) custom.border_color = row[1].as<std::string>();
        custom.bg_pattern = row[2].as<int32_t>();
        if (!row[3].is_null()) custom.accent_color = row[3].as<std::string>();
        custom.font_style = row[4].as<int32_t>();
        custom.show_stats = row[5].as<bool>();
        custom.show_artwork = row[6].as<bool>();

        if (!row[7].is_null()) {
            auto j = json::parse(row[7].as<std::string>());
            for (auto& [key, val] : j.items()) {
                custom.custom_fields[key] = val;
            }
        }
        custom.updated_at = row[8].as<int64_t>();
        return custom;
    });
}

bool CustomizationRepository::VerifyFursonaOwner(pqxx::work& txn,
                                                 const std::string& user_id,
                                                 int64_t fursona_id) {
    auto r = txn.exec_params(R"(
        SELECT 1 FROM fursonas WHERE id = $1 AND user_id = $2
    )", fursona_id, user_id);
    return !r.empty();
}

void CustomizationRepository::UpdateFursonaCardCustom(const std::string& owner_id,
                                                      int64_t fursona_id,
                                                      const FursonaCardCustomEntity& custom) {
    Execute([&](pqxx::work& txn) {
        if (!VerifyFursonaOwner(txn, owner_id, fursona_id)) return;

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        json j = custom.custom_fields;
        std::string json_str = j.dump();

        txn.exec_params(R"(
            INSERT INTO fursona_card_custom (fursona_id, card_theme, border_color,
                bg_pattern, accent_color, font_style, show_stats, show_artwork,
                custom_fields, updated_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
            ON CONFLICT (fursona_id) DO UPDATE SET
                card_theme = $2, border_color = $3, bg_pattern = $4,
                accent_color = $5, font_style = $6, show_stats = $7,
                show_artwork = $8, custom_fields = $9, updated_at = $10
        )", fursona_id,
           custom.card_theme.empty() ? "classic" : custom.card_theme,
           custom.border_color.empty() ? nullptr : &custom.border_color,
           custom.bg_pattern,
           custom.accent_color.empty() ? nullptr : &custom.accent_color,
           custom.font_style, custom.show_stats, custom.show_artwork,
           custom.custom_fields.empty() ? nullptr : &json_str,
           timestamp);
    });
}

NotificationSettingsEntity CustomizationRepository::GetNotificationSettings(
    const std::string& user_id) {
    return Execute<NotificationSettingsEntity>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT mention_email, mention_push, comment_email, comment_push,
                   follow_email, follow_push, like_email, like_push,
                   gift_email, gift_push, message_email, message_push,
                   event_email, event_push, weekly_digest, marketing_email
            FROM notification_settings WHERE user_id = $1
        )", user_id);

        NotificationSettingsEntity settings;
        if (result.empty()) return settings;

        const auto& row = result[0];
        settings.mention_email = row[0].as<bool>();
        settings.mention_push = row[1].as<bool>();
        settings.comment_email = row[2].as<bool>();
        settings.comment_push = row[3].as<bool>();
        settings.follow_email = row[4].as<bool>();
        settings.follow_push = row[5].as<bool>();
        settings.like_email = row[6].as<bool>();
        settings.like_push = row[7].as<bool>();
        settings.gift_email = row[8].as<bool>();
        settings.gift_push = row[9].as<bool>();
        settings.message_email = row[10].as<bool>();
        settings.message_push = row[11].as<bool>();
        settings.event_email = row[12].as<bool>();
        settings.event_push = row[13].as<bool>();
        settings.weekly_digest = row[14].as<bool>();
        settings.marketing_email = row[15].as<bool>();
        return settings;
    });
}

void CustomizationRepository::UpdateNotificationSettings(
    const std::string& user_id, const NotificationSettingsEntity& settings) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(R"(
            INSERT INTO notification_settings (user_id, mention_email, mention_push,
                comment_email, comment_push, follow_email, follow_push,
                like_email, like_push, gift_email, gift_push,
                message_email, message_push, event_email, event_push,
                weekly_digest, marketing_email)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12,
                    $13, $14, $15, $16, $17)
            ON CONFLICT (user_id) DO UPDATE SET
                mention_email = $2, mention_push = $3,
                comment_email = $4, comment_push = $5,
                follow_email = $6, follow_push = $7,
                like_email = $8, like_push = $9,
                gift_email = $10, gift_push = $11,
                message_email = $12, message_push = $13,
                event_email = $14, event_push = $15,
                weekly_digest = $16, marketing_email = $17
        )", user_id,
           settings.mention_email, settings.mention_push,
           settings.comment_email, settings.comment_push,
           settings.follow_email, settings.follow_push,
           settings.like_email, settings.like_push,
           settings.gift_email, settings.gift_push,
           settings.message_email, settings.message_push,
           settings.event_email, settings.event_push,
           settings.weekly_digest, settings.marketing_email);
    });
}

FeedSettingsEntity CustomizationRepository::GetFeedSettings(const std::string& user_id) {
    return Execute<FeedSettingsEntity>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT default_sort, show_avatars, show_signatures, compact_mode,
                   posts_per_page, auto_load_more, blur_nsfw, hide_nsfw,
                   blocked_tags, blocked_users
            FROM feed_settings WHERE user_id = $1
        )", user_id);

        FeedSettingsEntity settings;
        settings.default_sort = "hot";
        if (result.empty()) return settings;

        const auto& row = result[0];
        if (!row[0].is_null()) settings.default_sort = row[0].as<std::string>();
        settings.show_avatars = row[1].as<bool>();
        settings.show_signatures = row[2].as<bool>();
        settings.compact_mode = row[3].as<bool>();
        settings.posts_per_page = row[4].as<int32_t>();
        settings.auto_load_more = row[5].as<bool>();
        settings.blur_nsfw = row[6].as<bool>();
        settings.hide_nsfw = row[7].as<bool>();
        return settings;
    });
}

void CustomizationRepository::UpdateFeedSettings(const std::string& user_id,
                                                 const FeedSettingsEntity& settings) {
    Execute([&](pqxx::work& txn) {
        pqxx::array<std::string> tag_arr(settings.blocked_tags);
        pqxx::array<std::string> user_arr(settings.blocked_users);

        txn.exec_params(R"(
            INSERT INTO feed_settings (user_id, default_sort, show_avatars,
                show_signatures, compact_mode, posts_per_page, auto_load_more,
                blur_nsfw, hide_nsfw, blocked_tags, blocked_users)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
            ON CONFLICT (user_id) DO UPDATE SET
                default_sort = $2, show_avatars = $3, show_signatures = $4,
                compact_mode = $5, posts_per_page = $6, auto_load_more = $7,
                blur_nsfw = $8, hide_nsfw = $9, blocked_tags = $10,
                blocked_users = $11
        )", user_id,
           settings.default_sort.empty() ? "hot" : settings.default_sort,
           settings.show_avatars, settings.show_signatures,
           settings.compact_mode, settings.posts_per_page,
           settings.auto_load_more, settings.blur_nsfw, settings.hide_nsfw,
           settings.blocked_tags.empty() ? nullptr : &tag_arr,
           settings.blocked_users.empty() ? nullptr : &user_arr);
    });
}

int64_t CustomizationRepository::CreateTheme(const std::string& creator_id,
                                             const UserThemeEntity& theme) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO user_themes (name, creator_id, primary_color,
                secondary_color, accent_color, bg_color, card_bg_color,
                text_color, is_public, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
            RETURNING id
        )", theme.name, creator_id,
           theme.primary_color.empty() ? nullptr : &theme.primary_color,
           theme.secondary_color.empty() ? nullptr : &theme.secondary_color,
           theme.accent_color.empty() ? nullptr : &theme.accent_color,
           theme.bg_color.empty() ? nullptr : &theme.bg_color,
           theme.card_bg_color.empty() ? nullptr : &theme.card_bg_color,
           theme.text_color.empty() ? nullptr : &theme.text_color,
           theme.is_public, timestamp);

        return result[0][0].as<int64_t>();
    });
}

std::vector<UserThemeEntity> CustomizationRepository::GetThemes(
    bool only_public, const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<UserThemeEntity>>([&](pqxx::work& txn) {
        pqxx::result result;
        if (!user_id.empty()) {
            const std::string& query = only_public
                ? sql::THEMES_LIST_PUBLIC_BY_USER
                : sql::THEMES_LIST_BY_USER;
            result = txn.exec_params(query, user_id, limit, offset);
        } else {
            const std::string& query = only_public
                ? sql::THEMES_LIST_PUBLIC
                : sql::THEMES_LIST_ALL;
            result = txn.exec_params(query, limit, offset);
        }

        std::vector<UserThemeEntity> themes;
        for (const auto& row : result) {
            UserThemeEntity t;
            t.id = row[0].as<int64_t>();
            t.name = row[1].as<std::string>();
            if (!row[2].is_null()) t.creator_id = row[2].as<std::string>();
            if (!row[3].is_null()) t.creator_name = row[3].as<std::string>();
            if (!row[4].is_null()) t.primary_color = row[4].as<std::string>();
            if (!row[5].is_null()) t.secondary_color = row[5].as<std::string>();
            t.is_public = row[10].as<bool>();
            t.use_count = row[11].as<int32_t>();
            t.created_at = row[12].as<int64_t>();
            themes.push_back(t);
        }
        return themes;
    });
}

int CustomizationRepository::GetThemeCount(bool only_public, const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        pqxx::result r;
        if (!user_id.empty()) {
            const std::string& query = only_public ? sql::THEME_COUNT_PUBLIC_BY_USER : sql::THEME_COUNT_BY_USER;
            r = txn.exec_params(query, user_id);
        } else {
            const std::string& query = only_public ? sql::THEME_COUNT_PUBLIC : sql::THEME_COUNT_ALL;
            r = txn.exec(query);
        }
        return r[0][0].as<int>();
    });
}

GroupCustomSettingsEntity CustomizationRepository::GetGroupCustomSettings(int64_t group_id) {
    return Execute<GroupCustomSettingsEntity>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT entry_message, require_approval, approval_questions,
                   group_icon, group_color, custom_rules,
                   allow_image_posts, allow_link_posts, post_cooldown,
                   mod_can_delete, mod_can_ban, visible_members
            FROM group_custom_settings WHERE group_id = $1
        )", group_id);

        GroupCustomSettingsEntity settings;
        settings.group_id = group_id;
        if (result.empty()) return settings;

        const auto& row = result[0];
        if (!row[0].is_null()) settings.entry_message = row[0].as<std::string>();
        settings.require_approval = row[1].as<bool>();
        if (!row[2].is_null()) {
            pqxx::array_parser<std::string> parser(row[2].as<std::string>());
            std::vector<std::string> questions;
            std::string q;
            bool more;
            while ((more = parser.get_next(q))) {
                if (!q.empty()) questions.push_back(q);
            }
            settings.approval_questions = questions;
        }
        if (!row[3].is_null()) settings.group_icon = row[3].as<std::string>();
        if (!row[4].is_null()) settings.group_color = row[4].as<std::string>();
        if (!row[5].is_null()) settings.custom_rules = row[5].as<std::string>();
        settings.allow_image_posts = row[6].as<bool>();
        settings.allow_link_posts = row[7].as<bool>();
        settings.post_cooldown = row[8].as<int32_t>();
        settings.mod_can_delete = row[9].as<bool>();
        settings.mod_can_ban = row[10].as<bool>();
        settings.visible_members = row[11].as<bool>();
        return settings;
    });
}

bool CustomizationRepository::VerifyGroupOwner(pqxx::work& txn, const std::string& user_id,
                                               int64_t group_id) {
    auto r = txn.exec_params(R"(
        SELECT 1 FROM user_groups WHERE id = $1 AND owner_id = $2
    )", group_id, user_id);
    return !r.empty();
}

bool CustomizationRepository::UpdateGroupCustomSettings(const std::string& caller_id,
                                                        int64_t group_id,
                                                        const GroupCustomSettingsEntity& settings) {
    return Execute<bool>([&](pqxx::work& txn) {
        if (!VerifyGroupOwner(txn, caller_id, group_id)) {
            return false;
        }

        pqxx::array<std::string> q_arr(settings.approval_questions);

        txn.exec_params(R"(
            INSERT INTO group_custom_settings (group_id, entry_message,
                require_approval, approval_questions, group_icon, group_color,
                custom_rules, allow_image_posts, allow_link_posts,
                post_cooldown, mod_can_delete, mod_can_ban, visible_members)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
            ON CONFLICT (group_id) DO UPDATE SET
                entry_message = $2, require_approval = $3,
                approval_questions = $4, group_icon = $5, group_color = $6,
                custom_rules = $7, allow_image_posts = $8, allow_link_posts = $9,
                post_cooldown = $10, mod_can_delete = $11, mod_can_ban = $12,
                visible_members = $13
        )", group_id,
           settings.entry_message.empty() ? nullptr : &settings.entry_message,
           settings.require_approval,
           settings.approval_questions.empty() ? nullptr : &q_arr,
           settings.group_icon.empty() ? nullptr : &settings.group_icon,
           settings.group_color.empty() ? nullptr : &settings.group_color,
           settings.custom_rules.empty() ? nullptr : &settings.custom_rules,
           settings.allow_image_posts, settings.allow_link_posts,
           settings.post_cooldown, settings.mod_can_delete,
           settings.mod_can_ban, settings.visible_members);

        return true;
    });
}

} // namespace furbbs::repository
