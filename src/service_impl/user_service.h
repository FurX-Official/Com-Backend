#ifndef FURBBS_SERVICE_IMPL_USER_SERVICE_H
#define FURBBS_SERVICE_IMPL_USER_SERVICE_H

#include "repository/user_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class UserService {
public:
    static UserService& Instance() {
        static UserService instance;
        return instance;
    }

    int32_t DailySignIn(const std::string& token, int32_t& out_streak,
                        bool& out_is_continuous, int32_t& out_new_level);

    std::vector<repository::AchievementEntity> GetAchievements(const std::string& user_id);

    bool UnlockAchievement(const std::string& token, int32_t achievement_id,
                           int32_t& out_points_reward);

    repository::UserStatsEntity GetUserStats(const std::string& user_id);

    void AddActivityPoints(const std::string& user_id, int32_t amount);

private:
    UserService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_USER_SERVICE_H
