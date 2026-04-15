#ifndef FURBBS_SERVICE_IMPL_CUSTOMIZATION_SERVICE_H
#define FURBBS_SERVICE_IMPL_CUSTOMIZATION_SERVICE_H

#include "repository/customization_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

struct CustomizationResult {
    bool success = false;
    std::string message;
};

class CustomizationService {
public:
    static CustomizationService& Instance() {
        static CustomizationService instance;
        return instance;
    }

    std::vector<repository::TitleEntity> GetAllTitles();
    CustomizationResult SetActiveTitle(const std::string& token, int32_t title_id);

    std::vector<repository::AvatarFrameEntity> GetAllAvatarFrames();
    CustomizationResult SetActiveFrame(const std::string& token, int32_t frame_id);

    std::vector<repository::NameplateEntity> GetAllNameplates();
    std::vector<repository::ThemeEntity> GetAllThemes();
    CustomizationResult SetCustomization(const std::string& token,
                                          int32_t nameplate_id, int32_t theme_id);

private:
    CustomizationService() = default;
    int64_t GetCurrentTimestamp();
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_CUSTOMIZATION_SERVICE_H
