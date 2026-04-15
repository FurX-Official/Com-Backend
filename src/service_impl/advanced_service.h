#ifndef FURBBS_SERVICE_IMPL_ADVANCED_SERVICE_H
#define FURBBS_SERVICE_IMPL_ADVANCED_SERVICE_H

#include "repository/advanced_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class AdvancedService {
public:
    static AdvancedService& Instance() {
        static AdvancedService instance;
        return instance;
    }

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
    AdvancedService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_ADVANCED_SERVICE_H
