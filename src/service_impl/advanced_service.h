#ifndef FURBBS_SERVICE_IMPL_ADVANCED_SERVICE_H
#define FURBBS_SERVICE_IMPL_ADVANCED_SERVICE_H

#include "repository/advanced_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace furbbs::service {

struct PunishmentRule {
    std::string offense_type;
    int points_threshold;
    std::string punishment_type;
    int64_t duration_ms;
    int points_deduction;
};

class AdvancedService {
public:
    static AdvancedService& Instance() {
        static AdvancedService instance;
        return instance;
    }

    void InitializePunishmentRules();

    bool AddModerator(const std::string& token, int64_t section_id,
                      const std::string& target_user_id,
                      const std::string& permission_level,
                      bool can_manage_posts, bool can_manage_comments,
                      bool can_manage_users, bool can_manage_reports);

    bool RemoveModerator(const std::string& token, int64_t section_id,
                         const std::string& target_user_id);

    std::vector<repository::ModeratorEntity> GetUserModeratorRoles(
        const std::string& user_id);

    bool CheckModeratorPermission(const std::string& user_id,
                                  int64_t section_id,
                                  const std::string& permission);

    bool PunishUser(const std::string& token, const std::string& target_user_id,
                    const std::string& punishment_type, const std::string& reason,
                    int64_t duration_ms, int points_deducted);

    bool AutoPunishUser(const std::string& user_id, const std::string& offense_type,
                        const std::string& reason);

    std::vector<repository::PunishmentEntity> GetUserPunishments(
        const std::string& token, const std::string& target_user_id);

    bool IsUserPunished(const std::string& user_id, const std::string& punishment_type);

    void ExpireOldPunishments();

    bool CreatePoll(const std::string& token, int64_t post_id,
                    const std::string& question,
                    const std::vector<std::string>& options,
                    bool is_multiple, int64_t end_at);

    std::optional<repository::PollEntity> GetPoll(
        const std::string& token, int64_t post_id);

    bool VotePoll(const std::string& token, int64_t post_id,
                  const std::vector<int32_t>& option_indices);

    void CalculateHotScores();

    std::vector<int64_t> GetHotPosts(int limit, int offset);

    std::vector<int64_t> GetPersonalizedFeed(const std::string& user_id,
                                             int limit, int offset);

    bool SetUserWatermark(const std::string& token, const std::string& text,
                          const std::string& position, double opacity,
                          bool is_enabled);

    std::optional<repository::WatermarkEntity> GetUserWatermark(
        const std::string& token);

    bool SetFeedSettings(const std::string& token, const std::string& feed_type,
                         const std::vector<int64_t>& include_sections,
                         const std::vector<std::string>& exclude_tags);

    std::optional<repository::FeedSettingsEntity> GetFeedSettings(
        const std::string& token);

    void LogRecommendation(const std::string& user_id, int64_t post_id,
                           const std::string& algorithm, const std::string& action,
                           double score);

    int64_t CreateGroup(const std::string& token, const std::string& name,
                        const std::string& description, bool is_public,
                        bool allow_join_request, const std::vector<std::string>& tags);

    std::vector<repository::GroupEntity> GetGroups(
        const std::string& token, const std::string& user_id,
        const std::string& tag, const std::string& keyword,
        int page, int page_size, int& out_total);

    std::optional<repository::GroupEntity> GetGroupDetail(
        const std::string& token, int64_t group_id);

    bool ManageGroupMember(const std::string& token, int64_t group_id,
                           const std::string& target_user_id, int action, int new_role);

    int64_t CreateGroupPost(const std::string& token, int64_t group_id,
                            const std::string& title, const std::string& content,
                            const std::vector<int64_t>& tag_ids);

    int64_t RegisterEvent(const std::string& token, int64_t event_id,
                          int32_t ticket_type, int32_t guest_count,
                          const std::string& contact);

    std::vector<repository::EventRegEntity> GetEventRegistrations(
        const std::string& token, int64_t event_id,
        int page, int page_size, int& out_total, int& out_confirmed);

    bool UpdateRegistrationStatus(const std::string& token, int64_t reg_id, int32_t new_status);

    std::vector<repository::MentionEntity> GetMentions(
        const std::string& token, bool only_unread,
        int page, int page_size, int& out_total, int& out_unread);

    void MarkMentionRead(const std::string& token, int64_t mention_id, bool mark_all);

    bool FavoritePost(const std::string& token, int64_t post_id, bool favorite);

    std::vector<PostEntity> GetFavoritePosts(
        const std::string& token, int page, int page_size, int& out_total);

    int64_t SaveDraft(const std::string& token, int64_t draft_id,
                      const std::string& title, const std::string& content,
                      int32_t section_id, const std::vector<int64_t>& tag_ids,
                      int64_t group_id);

    std::vector<repository::DraftEntity> GetDrafts(
        const std::string& token, int page, int page_size, int& out_total);

    bool DeleteDraft(const std::string& token, int64_t draft_id);

    void ProcessMentions(const std::string& user_id, const std::string& content,
                         int64_t post_id, int64_t comment_id);

private:
    AdvancedService() {
        InitializePunishmentRules();
    }

    std::string ValidateAndGetUserId(const std::string& token);

    bool IsAdmin(const std::string& user_id);

    std::unordered_map<std::string, std::vector<PunishmentRule>> punishment_rules_;
    std::unordered_map<std::string, int> user_offense_counts_;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_ADVANCED_SERVICE_H
