#include "controller/common.h"
#include "service_impl/user_service.h"

namespace furbbs::controller {

trpc::Status UserController::DailySignIn(::trpc::ServerContextPtr,
                                          const ::furbbs::DailySignInRequest* request,
                                          ::furbbs::DailySignInResponse* response) {
    int32_t streak = 0;
    bool is_continuous = false;
    int32_t new_level = 0;

    int32_t points = service::UserService::Instance().DailySignIn(
        request->token(), streak, is_continuous, new_level);

    response->set_earned_points(points);
    response->set_current_streak(streak);
    response->set_is_continuous(is_continuous);
    response->set_new_level(new_level);
    response->set_success(points > 0);
    return trpc::kSuccStatus;
}

trpc::Status UserController::GetAchievements(::trpc::ServerContextPtr,
                                              const ::furbbs::GetAchievementsRequest* request,
                                              ::furbbs::GetAchievementsResponse* response) {
    auto list = service::UserService::Instance().GetAchievements(request->user_id());
    for (const auto& item : list) {
        auto* a = response->add_achievements();
        a->set_id(item.id);
        a->set_name(item.name);
        a->set_icon(item.icon);
        a->set_description(item.description);
        a->set_points_reward(item.points_reward);
        a->set_is_unlocked(item.is_unlocked);
        a->set_unlocked_at(item.unlocked_at);
    }
    return trpc::kSuccStatus;
}

trpc::Status UserController::UnlockAchievement(::trpc::ServerContextPtr,
                                                const ::furbbs::UnlockAchievementRequest* request,
                                                ::furbbs::UnlockAchievementResponse* response) {
    int32_t points_reward = 0;
    bool success = service::UserService::Instance().UnlockAchievement(
        request->token(), request->achievement_id(), points_reward);

    response->set_success(success);
    response->set_points_reward(points_reward);
    return trpc::kSuccStatus;
}

trpc::Status UserController::GetUserStats(::trpc::ServerContextPtr,
                                           const ::furbbs::GetUserStatsRequest* request,
                                           ::furbbs::GetUserStatsResponse* response) {
    auto stats = service::UserService::Instance().GetUserStats(request->user_id());
    response->set_points(stats.points);
    response->set_level(stats.level);
    response->set_posts_count(stats.posts_count);
    response->set_comments_count(stats.comments_count);
    response->set_sign_in_streak(stats.sign_in_streak);
    response->set_last_sign_in(stats.last_sign_in);
    return trpc::kSuccStatus;
}

} // namespace furbbs::controller
