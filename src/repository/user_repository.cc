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

} // namespace furbbs::repository
