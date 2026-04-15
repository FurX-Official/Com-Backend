#include "user_repository.h"

namespace furbbs::repository {

std::optional<UserStatsEntity> UserRepository::GetUserStats(const std::string& user_id) {
    return Execute<std::optional<UserStatsEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::USER_GET_STATS, user_id);
        if (result.empty()) return std::nullopt;

        UserStatsEntity stats;
        stats.user_id = user_id;
        stats.points = result[0][24].as<int32_t>();
        stats.posts_count = result[0][21].as<int32_t>();
        stats.comments_count = result[0][22].as<int32_t>();
        stats.level = result[0][26].as<int32_t>();
        return stats;
    });
}

void UserRepository::AddPoints(const std::string& user_id, int32_t amount) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(R"(
            UPDATE user_stats SET points = points + $1 WHERE user_id = $2
        )", amount, user_id);
    });
}

void UserRepository::AddMembership(const std::string& user_id, int tier, int64_t expiry_date) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(R"(
            INSERT INTO user_memberships (user_id, tier, start_date, expiry_date)
            VALUES ($1, $2, $3, $4)
            ON CONFLICT (user_id) DO UPDATE SET
            tier = GREATEST(user_memberships.tier, $2),
            expiry_date = CASE WHEN user_memberships.expiry_date < $3 THEN $4
                               ELSE user_memberships.expiry_date + $4 - $3 END
        )", user_id, tier, timestamp, expiry_date);
    });
}

void UserRepository::UnlockTitle(const std::string& user_id, int32_t title_id) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(R"(
            INSERT INTO user_owned_titles (user_id, title_id) VALUES ($1, $2)
            ON CONFLICT DO NOTHING
        )", user_id, title_id);
    });
}

void UserRepository::UnlockAvatarFrame(const std::string& user_id, int32_t frame_id) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(R"(
            INSERT INTO user_owned_frames (user_id, frame_id) VALUES ($1, $2)
            ON CONFLICT DO NOTHING
        )", user_id, frame_id);
    });
}

int32_t UserRepository::DailySignIn(const std::string& user_id, int32_t& out_streak, bool& out_is_continuous) {
    return Execute<int32_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto today = std::chrono::system_clock::to_time_t(now);
        today = today - (today % 86400);
        auto yesterday = today - 86400;
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        int32_t streak_bonus[] = {10, 15, 20, 25, 30, 40, 50};
        int32_t day = out_streak % 7;
        int32_t points = streak_bonus[day];

        auto result = txn.exec_params(sql::USER_SIGN_IN, user_id, points, timestamp, yesterday);
        out_streak = result[0][0].as<int32_t>();
        out_is_continuous = result[0][1].as<bool>();

        UpdateLevel(user_id);
        return points;
    });
}

std::vector<AchievementEntity> UserRepository::GetAchievements(const std::string& user_id) {
    return Execute<std::vector<AchievementEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::USER_GET_ACHIEVEMENTS, user_id);
        std::vector<AchievementEntity> list;
        for (const auto& row : result) {
            AchievementEntity e;
            e.id = row[0].as<int32_t>();
            e.name = row[1].as<std::string>();
            e.icon = row[2].as<std::string>();
            e.description = row[3].as<std::string>();
            e.points_reward = row[4].as<int32_t>();
            e.is_unlocked = row[5].as<bool>();
            e.unlocked_at = row[6].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

bool UserRepository::CheckAndUnlockAchievement(const std::string& user_id, int32_t achievement_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(sql::USER_UNLOCK_ACHIEVEMENT, user_id, achievement_id, timestamp);
        if (!result.empty()) {
            txn.exec_params(R"(
                UPDATE user_stats SET points = points + (
                    SELECT points_reward FROM achievements WHERE id = $1
                ) WHERE user_id = $2
            )", achievement_id, user_id);
            UpdateLevel(user_id);
            return true;
        }
        return false;
    });
}

void UserRepository::UpdateLevel(const std::string& user_id) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::USER_UPDATE_LEVEL, user_id);
    });
}

} // namespace furbbs::repository
