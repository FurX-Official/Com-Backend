#ifndef FURBBS_SERVICE_IMPL_CUSTOMIZATION_SERVICE_H
#define FURBBS_SERVICE_IMPL_CUSTOMIZATION_SERVICE_H

#include "repository/customization_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class CustomizationService {
public:
    static CustomizationService& Instance() {
        static CustomizationService instance;
        return instance;
    }

    repository::ProfileCustomEntity GetProfileCustom(const std::string& token, const std::string& user_id);
    bool UpdateProfileCustom(const std::string& token, const repository::ProfileCustomEntity& custom);

    repository::FursonaCardCustomEntity GetFursonaCardCustom(int64_t fursona_id);
    bool UpdateFursonaCardCustom(const std::string& token, const repository::FursonaCardCustomEntity& custom);

    repository::NotificationSettingsEntity GetNotificationSettings(const std::string& token);
    bool UpdateNotificationSettings(const std::string& token, const repository::NotificationSettingsEntity& settings);

    repository::FeedSettingsEntity GetFeedSettings(const std::string& token);
    bool UpdateFeedSettings(const std::string& token, const repository::FeedSettingsEntity& settings);

    int64_t CreateTheme(const std::string& token, const repository::UserThemeEntity& theme);
    std::vector<repository::UserThemeEntity> GetThemes(
        bool only_public, const std::string& user_id, int page, int page_size, int& out_total);

    repository::GroupCustomSettingsEntity GetGroupCustomSettings(const std::string& token, int64_t group_id);
    bool UpdateGroupCustomSettings(const std::string& token, const repository::GroupCustomSettingsEntity& settings);

private:
    CustomizationService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_CUSTOMIZATION_SERVICE_H
