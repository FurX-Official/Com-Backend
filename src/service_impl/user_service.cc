#include "user_service.h"

namespace furbbs::service {

int32_t UserService::DailySignIn(const std::string& token, int32_t& out_streak,
                                  bool& out_is_continuous, int32_t& out_new_level) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return 0;

    int32_t points = repository::UserRepository::Instance().DailySignIn(
        *user_id, out_streak, out_is_continuous);

    auto stats = repository::UserRepository::Instance().GetUserStats(*user_id);
    out_new_level = stats ? stats->level : 1;

    return points;
}

std::vector<repository::AchievementEntity> UserService::GetAchievements(const std::string& user_id) {
    return repository::UserRepository::Instance().GetAchievements(user_id);
}

bool UserService::UnlockAchievement(const std::string& token, int32_t achievement_id,
                                     int32_t& out_points_reward) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return false;

    auto achievements = repository::UserRepository::Instance().GetAchievements(*user_id);
    for (const auto& a : achievements) {
        if (a.id == achievement_id && a.is_unlocked) {
            out_points_reward = a.points_reward;
            return repository::UserRepository::Instance().CheckAndUnlockAchievement(*user_id, achievement_id);
        }
    }
    return false;
}

repository::UserStatsEntity UserService::GetUserStats(const std::string& user_id) {
    auto stats = repository::UserRepository::Instance().GetUserStats(user_id);
    return stats ? *stats : repository::UserStatsEntity{};
}

void UserService::AddActivityPoints(const std::string& user_id, int32_t amount) {
    repository::UserRepository::Instance().AddPoints(user_id, amount);
    repository::UserRepository::Instance().UpdateLevel(user_id);
}

} // namespace furbbs::service
