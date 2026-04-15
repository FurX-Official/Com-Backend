#include "customization_service.h"

namespace furbbs::service {

repository::ProfileCustomEntity CustomizationService::GetProfileCustom(
    const std::string& token, const std::string& user_id) {

    std::string target_id = user_id;
    if (target_id.empty() && !token.empty()) {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user_opt) target_id = user_opt->id;
    }

    if (target_id.empty()) {
        return repository::ProfileCustomEntity();
    }

    return repository::CustomizationRepository::Instance().GetProfileCustom(target_id);
}

bool CustomizationService::UpdateProfileCustom(
    const std::string& token, const repository::ProfileCustomEntity& custom) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    repository::CustomizationRepository::Instance().UpdateProfileCustom(user_opt->id, custom);
    return true;
}

repository::FursonaCardCustomEntity CustomizationService::GetFursonaCardCustom(int64_t fursona_id) {
    return repository::CustomizationRepository::Instance().GetFursonaCardCustom(fursona_id);
}

bool CustomizationService::UpdateFursonaCardCustom(
    const std::string& token, const repository::FursonaCardCustomEntity& custom) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || custom.fursona_id <= 0) {
        return false;
    }

    repository::CustomizationRepository::Instance().UpdateFursonaCardCustom(
        user_opt->id, custom.fursona_id, custom);
    return true;
}

repository::NotificationSettingsEntity CustomizationService::GetNotificationSettings(
    const std::string& token) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return repository::NotificationSettingsEntity();
    }

    return repository::CustomizationRepository::Instance().GetNotificationSettings(user_opt->id);
}

bool CustomizationService::UpdateNotificationSettings(
    const std::string& token, const repository::NotificationSettingsEntity& settings) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    repository::CustomizationRepository::Instance().UpdateNotificationSettings(
        user_opt->id, settings);
    return true;
}

repository::FeedSettingsEntity CustomizationService::GetFeedSettings(const std::string& token) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return repository::FeedSettingsEntity();
    }

    return repository::CustomizationRepository::Instance().GetFeedSettings(user_opt->id);
}

bool CustomizationService::UpdateFeedSettings(
    const std::string& token, const repository::FeedSettingsEntity& settings) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    repository::CustomizationRepository::Instance().UpdateFeedSettings(
        user_opt->id, settings);
    return true;
}

int64_t CustomizationService::CreateTheme(const std::string& token,
                                           const repository::UserThemeEntity& theme) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || theme.name.empty()) {
        return 0;
    }

    return repository::CustomizationRepository::Instance().CreateTheme(user_opt->id, theme);
}

std::vector<repository::UserThemeEntity> CustomizationService::GetThemes(
    bool only_public, const std::string& user_id, int page, int page_size, int& out_total) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::CustomizationRepository::Instance().GetThemeCount(only_public, user_id);

    return repository::CustomizationRepository::Instance().GetThemes(
        only_public, user_id, page_size, offset);
}

repository::GroupCustomSettingsEntity CustomizationService::GetGroupCustomSettings(
    const std::string& token, int64_t group_id) {
    return repository::CustomizationRepository::Instance().GetGroupCustomSettings(group_id);
}

bool CustomizationService::UpdateGroupCustomSettings(
    const std::string& token, const repository::GroupCustomSettingsEntity& settings) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || settings.group_id <= 0) {
        return false;
    }

    return repository::CustomizationRepository::Instance().UpdateGroupCustomSettings(
        user_opt->id, settings.group_id, settings);
}

} // namespace furbbs::service
