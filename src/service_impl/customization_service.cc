#include "customization_service.h"
#include <chrono>

namespace furbbs::service {

std::vector<repository::TitleEntity> CustomizationService::GetAllTitles() {
    return repository::CustomizationRepository::Instance().GetAllTitles();
}

CustomizationResult CustomizationService::SetActiveTitle(const std::string& token, int32_t title_id) {
    CustomizationResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    repository::CustomizationRepository::Instance().SetActiveTitle(
        user_opt->id, title_id, GetCurrentTimestamp());

    result.success = true;
    result.message = "Active title set successfully";
    return result;
}

std::vector<repository::AvatarFrameEntity> CustomizationService::GetAllAvatarFrames() {
    return repository::CustomizationRepository::Instance().GetAllAvatarFrames();
}

CustomizationResult CustomizationService::SetActiveFrame(const std::string& token, int32_t frame_id) {
    CustomizationResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    repository::CustomizationRepository::Instance().SetActiveFrame(
        user_opt->id, frame_id, GetCurrentTimestamp());

    result.success = true;
    result.message = "Active avatar frame set successfully";
    return result;
}

std::vector<repository::NameplateEntity> CustomizationService::GetAllNameplates() {
    return repository::CustomizationRepository::Instance().GetAllNameplates();
}

std::vector<repository::ThemeEntity> CustomizationService::GetAllThemes() {
    return repository::CustomizationRepository::Instance().GetAllThemes();
}

CustomizationResult CustomizationService::SetCustomization(const std::string& token,
                                                           int32_t nameplate_id, int32_t theme_id) {
    CustomizationResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "Invalid token";
        return result;
    }

    repository::CustomizationRepository::Instance().SetCustomization(
        user_opt->id, nameplate_id, theme_id, GetCurrentTimestamp());

    result.success = true;
    result.message = "Customization updated successfully";
    return result;
}

int64_t CustomizationService::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

} // namespace furbbs::service
