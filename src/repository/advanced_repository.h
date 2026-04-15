#ifndef FURBBS_REPOSITORY_ADVANCED_REPOSITORY_H
#define FURBBS_REPOSITORY_ADVANCED_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

namespace furbbs::repository {

struct ModeratorEntity {
    int64_t id = 0;
    int64_t section_id = 0;
    std::string user_id;
    std::string assigned_by;
    std::string permission_level;
    bool can_manage_posts = true;
    bool can_manage_comments = true;
    bool can_manage_users = true;
    bool can_manage_reports = true;
    int64_t assigned_at = 0;
};

struct PunishmentEntity {
    int64_t id = 0;
    std::string user_id;
    std::string punishment_type;
    std::string reason;
    int64_t duration = 0;
    int32_t points_deducted = 0;
    std::string executed_by;
    int64_t executed_at = 0;
    int64_t expires_at = 0;
    bool is_active = true;
};

struct PollEntity {
    int64_t post_id = 0;
    std::string question;
    std::vector<std::string> options;
    std::vector<int32_t> vote_counts;
    bool is_multiple = false;
    int64_t end_at = 0;
    int64_t created_at = 0;
    bool has_voted = false;
};

struct HotScoreEntity {
    int64_t post_id = 0;
    double hot_score = 0;
    double view_weight = 0;
    double like_weight = 0;
    double comment_weight = 0;
    int64_t last_updated_at = 0;
};

struct WatermarkEntity {
    int64_t id = 0;
    std::string user_id;
    std::string watermark_text;
    std::string watermark_position;
    double opacity = 0.3;
    bool is_enabled = true;
    int64_t created_at = 0;
};

struct FeedSettingsEntity {
    std::string user_id;
    std::string feed_type;
    std::vector<int64_t> include_sections;
    std::vector<std::string> exclude_tags;
    int64_t updated_at = 0;
};

class AdvancedRepository : protected BaseRepository {
public:
    static AdvancedRepository& Instance() {
        static AdvancedRepository instance;
        return instance;
    }

    bool AddModerator(const ModeratorEntity& data);
    bool RemoveModerator(int64_t section_id, const std::string& user_id);
    std::vector<ModeratorEntity> GetUserModeratorRoles(const std::string& user_id);
    bool CheckModeratorPermission(const std::string& user_id, int64_t section_id,
                                    const std::string& permission = "posts");

    bool CreatePunishment(const PunishmentEntity& data);
    std::vector<PunishmentEntity> GetActivePunishments(const std::string& user_id);
    bool ExpireOldPunishments();
    void RecordPunishmentHistory(const PunishmentEntity& data);

    bool CreatePoll(const PollEntity& data);
    std::optional<PollEntity> GetPoll(int64_t post_id, const std::string& voter_id = "");
    bool VotePoll(int64_t poll_id, const std::string& user_id, int option_index);
    bool HasUserVoted(int64_t poll_id, const std::string& user_id);

    bool UpdateHotScore(const HotScoreEntity& data);
    std::vector<int64_t> GetTopHotPosts(int limit, int offset);

    std::optional<WatermarkEntity> GetUserWatermark(const std::string& user_id);
    bool SetUserWatermark(const WatermarkEntity& data);

    std::optional<FeedSettingsEntity> GetFeedSettings(const std::string& user_id);
    bool SetFeedSettings(const FeedSettingsEntity& data);

    void LogRecommendation(const std::string& user_id, int64_t post_id,
                           const std::string& algorithm, const std::string& action, double score);

private:
    AdvancedRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_ADVANCED_REPOSITORY_H
