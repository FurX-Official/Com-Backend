#ifndef FURBBS_REPOSITORY_USER_REPOSITORY_H
#define FURBBS_REPOSITORY_USER_REPOSITORY_USER_REPOSITORY_H

#include "base_repository.h"
#include "db/sql_queries.h"
#include <optional>
#include <string>

namespace furbbs::repository {

struct UserStatsEntity {
    std::string user_id;
    int32_t points = 0;
    int32_t posts_count = 0;
    int32_t comments_count = 0;
    int32_t level = 1;
};

struct UserMembershipEntity {
    std::string user_id;
    int32_t tier = 0;
    int64_t expiry_date = 0;
    bool is_active = false;
};

class UserRepository : protected BaseRepository {
public:
    static UserRepository& Instance() {
        static UserRepository instance;
        return instance;
    }

    std::optional<UserStatsEntity> GetUserStats(const std::string& user_id);
    void AddPoints(const std::string& user_id, int32_t amount);

    void AddMembership(const std::string& user_id, int tier, int64_t expiry_date);

    void UnlockTitle(const std::string& user_id, int32_t title_id);
    void UnlockAvatarFrame(const std::string& user_id, int32_t frame_id);

private:
    UserRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_USER_REPOSITORY_H
