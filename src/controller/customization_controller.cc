#include "../service_impl/customization_service.h"
#include "../common/result.h"

namespace furbbs::controller {

using ::trpc::ServerContextPtr;
using namespace furbbs::repository;

::trpc::Status GetProfileCustom(ServerContextPtr context,
                                 const ::furbbs::GetProfileCustomRequest* request,
                                 ::furbbs::GetProfileCustomResponse* response) {
    try {
        auto result = service::CustomizationService::Instance().GetProfileCustom(
            request->access_token(), request->user_id());
        auto* custom = response->mutable_custom();
        custom->set_theme(result.theme);
        custom->set_bg_color(result.bg_color);
        custom->set_bg_image(result.bg_image);
        custom->set_card_style(result.card_style);
        custom->set_layout_type(result.layout_type);
        custom->set_show_fursona_first(result.show_fursona_first);
        custom->set_show_badges(result.show_badges);
        custom->set_show_achievement(result.show_achievement);
        custom->set_music_url(result.music_url);
        custom->set_custom_css(result.custom_css);
        for (const auto& widget : result.sidebar_widgets) {
            custom->add_sidebar_widgets(widget);
        }
        custom->set_updated_at(result.updated_at);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status UpdateProfileCustom(ServerContextPtr context,
                                    const ::furbbs::UpdateProfileCustomRequest* request,
                                    ::furbbs::UpdateProfileCustomResponse* response) {
    try {
        ProfileCustomEntity entity;
        const auto& custom = request->custom();
        entity.theme = custom.theme();
        entity.bg_color = custom.bg_color();
        entity.bg_image = custom.bg_image();
        entity.card_style = custom.card_style();
        entity.layout_type = custom.layout_type();
        entity.show_fursona_first = custom.show_fursona_first();
        entity.show_badges = custom.show_badges();
        entity.show_achievement = custom.show_achievement();
        entity.music_url = custom.music_url();
        entity.custom_css = custom.custom_css();
        for (int i = 0; i < custom.sidebar_widgets_size(); i++) {
            entity.sidebar_widgets.push_back(custom.sidebar_widgets(i));
        }
        bool success = service::CustomizationService::Instance().UpdateProfileCustom(
            request->access_token(), entity);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "更新成功" : "更新失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("更新失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetFursonaCardCustom(ServerContextPtr context,
                                     const ::furbbs::GetFursonaCardCustomRequest* request,
                                     ::furbbs::GetFursonaCardCustomResponse* response) {
    try {
        auto result = service::CustomizationService::Instance().GetFursonaCardCustom(
            request->fursona_id());
        auto* custom = response->mutable_custom();
        custom->set_fursona_id(result.fursona_id);
        custom->set_card_theme(result.card_theme);
        custom->set_border_color(result.border_color);
        custom->set_bg_pattern(result.bg_pattern);
        custom->set_accent_color(result.accent_color);
        custom->set_font_style(result.font_style);
        custom->set_show_stats(result.show_stats);
        custom->set_show_artwork(result.show_artwork);
        for (const auto& field : result.custom_fields) {
            (*custom->mutable_custom_fields())[field.first] = field.second;
        }
        custom->set_updated_at(result.updated_at);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status UpdateFursonaCardCustom(ServerContextPtr context,
                                        const ::furbbs::UpdateFursonaCardCustomRequest* request,
                                        ::furbbs::UpdateFursonaCardCustomResponse* response) {
    try {
        FursonaCardCustomEntity entity;
        const auto& custom = request->custom();
        entity.fursona_id = custom.fursona_id();
        entity.card_theme = custom.card_theme();
        entity.border_color = custom.border_color();
        entity.bg_pattern = custom.bg_pattern();
        entity.accent_color = custom.accent_color();
        entity.font_style = custom.font_style();
        entity.show_stats = custom.show_stats();
        entity.show_artwork = custom.show_artwork();
        for (const auto& field : custom.custom_fields()) {
            entity.custom_fields[field.first] = field.second;
        }
        bool success = service::CustomizationService::Instance().UpdateFursonaCardCustom(
            request->access_token(), entity);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "更新成功" : "更新失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("更新失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetNotificationSettings(ServerContextPtr context,
                                        const ::furbbs::GetNotificationSettingsRequest* request,
                                        ::furbbs::GetNotificationSettingsResponse* response) {
    try {
        auto result = service::CustomizationService::Instance().GetNotificationSettings(
            request->access_token());
        auto* settings = response->mutable_settings();
        settings->set_mention_email(result.mention_email);
        settings->set_mention_push(result.mention_push);
        settings->set_comment_email(result.comment_email);
        settings->set_comment_push(result.comment_push);
        settings->set_follow_email(result.follow_email);
        settings->set_follow_push(result.follow_push);
        settings->set_like_email(result.like_email);
        settings->set_like_push(result.like_push);
        settings->set_gift_email(result.gift_email);
        settings->set_gift_push(result.gift_push);
        settings->set_message_email(result.message_email);
        settings->set_message_push(result.message_push);
        settings->set_event_email(result.event_email);
        settings->set_event_push(result.event_push);
        settings->set_weekly_digest(result.weekly_digest);
        settings->set_marketing_email(result.marketing_email);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status UpdateNotificationSettings(ServerContextPtr context,
                                           const ::furbbs::UpdateNotificationSettingsRequest* request,
                                           ::furbbs::UpdateNotificationSettingsResponse* response) {
    try {
        NotificationSettingsEntity entity;
        const auto& settings = request->settings();
        entity.mention_email = settings.mention_email();
        entity.mention_push = settings.mention_push();
        entity.comment_email = settings.comment_email();
        entity.comment_push = settings.comment_push();
        entity.follow_email = settings.follow_email();
        entity.follow_push = settings.follow_push();
        entity.like_email = settings.like_email();
        entity.like_push = settings.like_push();
        entity.gift_email = settings.gift_email();
        entity.gift_push = settings.gift_push();
        entity.message_email = settings.message_email();
        entity.message_push = settings.message_push();
        entity.event_email = settings.event_email();
        entity.event_push = settings.event_push();
        entity.weekly_digest = settings.weekly_digest();
        entity.marketing_email = settings.marketing_email();
        bool success = service::CustomizationService::Instance().UpdateNotificationSettings(
            request->access_token(), entity);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "更新成功" : "更新失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("更新失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetFeedSettings(ServerContextPtr context,
                                const ::furbbs::GetFeedSettingsRequest* request,
                                ::furbbs::GetFeedSettingsResponse* response) {
    try {
        auto result = service::CustomizationService::Instance().GetFeedSettings(
            request->access_token());
        auto* settings = response->mutable_settings();
        settings->set_default_sort(result.default_sort);
        settings->set_show_avatars(result.show_avatars);
        settings->set_show_signatures(result.show_signatures);
        settings->set_compact_mode(result.compact_mode);
        settings->set_posts_per_page(result.posts_per_page);
        settings->set_auto_load_more(result.auto_load_more);
        settings->set_blur_nsfw(result.blur_nsfw);
        settings->set_hide_nsfw(result.hide_nsfw);
        for (const auto& tag : result.blocked_tags) {
            settings->add_blocked_tags(tag);
        }
        for (const auto& user : result.blocked_users) {
            settings->add_blocked_users(user);
        }
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status UpdateFeedSettings(ServerContextPtr context,
                                   const ::furbbs::UpdateFeedSettingsRequest* request,
                                   ::furbbs::UpdateFeedSettingsResponse* response) {
    try {
        FeedSettingsEntity entity;
        const auto& settings = request->settings();
        entity.default_sort = settings.default_sort();
        entity.show_avatars = settings.show_avatars();
        entity.show_signatures = settings.show_signatures();
        entity.compact_mode = settings.compact_mode();
        entity.posts_per_page = settings.posts_per_page();
        entity.auto_load_more = settings.auto_load_more();
        entity.blur_nsfw = settings.blur_nsfw();
        entity.hide_nsfw = settings.hide_nsfw();
        for (int i = 0; i < settings.blocked_tags_size(); i++) {
            entity.blocked_tags.push_back(settings.blocked_tags(i));
        }
        for (int i = 0; i < settings.blocked_users_size(); i++) {
            entity.blocked_users.push_back(settings.blocked_users(i));
        }
        bool success = service::CustomizationService::Instance().UpdateFeedSettings(
            request->access_token(), entity);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "更新成功" : "更新失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("更新失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status CreateTheme(ServerContextPtr context,
                            const ::furbbs::CreateThemeRequest* request,
                            ::furbbs::CreateThemeResponse* response) {
    try {
        UserThemeEntity entity;
        const auto& theme = request->theme();
        entity.name = theme.name();
        entity.primary_color = theme.primary_color();
        entity.secondary_color = theme.secondary_color();
        entity.accent_color = theme.accent_color();
        entity.bg_color = theme.bg_color();
        entity.card_bg_color = theme.card_bg_color();
        entity.text_color = theme.text_color();
        entity.is_public = theme.is_public();
        int64_t theme_id = service::CustomizationService::Instance().CreateTheme(
            request->access_token(), entity);
        response->set_code(RESULT_OK);
        response->set_message("创建成功");
        response->set_theme_id(theme_id);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("创建失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetThemeList(ServerContextPtr context,
                             const ::furbbs::GetThemeListRequest* request,
                             ::furbbs::GetThemeListResponse* response) {
    try {
        int total = 0;
        auto result = service::CustomizationService::Instance().GetThemes(
            request->only_public(), request->user_id(), request->page(), request->page_size(), total);
        for (const auto& item : result) {
            auto* theme = response->add_themes();
            theme->set_id(item.id);
            theme->set_name(item.name);
            theme->set_creator_id(item.creator_id);
            theme->set_creator_name(item.creator_name);
            theme->set_primary_color(item.primary_color);
            theme->set_secondary_color(item.secondary_color);
            theme->set_accent_color(item.accent_color);
            theme->set_bg_color(item.bg_color);
            theme->set_card_bg_color(item.card_bg_color);
            theme->set_text_color(item.text_color);
            theme->set_is_public(item.is_public);
            theme->set_use_count(item.use_count);
            theme->set_created_at(item.created_at);
        }
        response->set_total(total);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

} // namespace furbbs::controller
