#ifndef FURBBS_REPOSITORY_ADVANCED_REPOSITORY_H
#define FURBBS_REPOSITORY_ADVANCED_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::repository {

struct GroupEntity {
    int64_t id = 0;
    std::string name;
    std::string description;
    std::string avatar;
    std::string banner;
    std::string owner_id;
    std::string owner_name;
    int32_t member_count = 0;
    int32_t post_count = 0;
    bool is_public = true;
    bool allow_join_request = true;
    std::vector<std::string> tags;
    int64_t created_at = 0;
    bool is_member = false;
    bool is_owner = false;
    int32_t member_role = 0;
};

struct EventRegEntity {
    int64_t id = 0;
    int64_t event_id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    int32_t ticket_type = 0;
    int32_t guest_count = 0;
    std::string contact;
    int32_t status = 0;
    int64_t registered_at = 0;
};

struct MentionEntity {
    int64_t id = 0;
    std::string from_user_id;
    std::string from_username;
    std::string from_avatar;
    int64_t post_id = 0;
    std::string post_title;
    int64_t comment_id = 0;
    std::string content_preview;
    bool is_read = false;
    int64_t created_at = 0;
};

struct DraftEntity {
    int64_t id = 0;
    std::string title;
    std::string content;
    int32_t section_id = 0;
    std::vector<int64_t> tag_ids;
    int64_t group_id = 0;
    int64_t created_at = 0;
    int64_t updated_at = 0;
};

class AdvancedRepository : protected BaseRepository {
public:
    static AdvancedRepository& Instance() {
        static AdvancedRepository instance;
        return instance;
    }

    int64_t CreateGroup(const std::string& owner_id, const std::string& name,
                        const std::string& description, bool is_public,
                        bool allow_join_request, const std::vector<std::string>& tags);

    std::vector<GroupEntity> GetGroups(
        const std::string& user_id, const std::string& viewer_id,
        const std::string& tag, const std::string& keyword,
        int limit, int offset);

    int GetGroupCount(const std::string& user_id, const std::string& tag,
                       const std::string& keyword);

    std::optional<GroupEntity> GetGroup(int64_t group_id, const std::string& viewer_id);

    bool ManageGroupMember(int64_t group_id, const std::string& owner_id,
                           const std::string& target_user_id, int action, int new_role);

    int64_t CreateGroupPost(int64_t group_id, const std::string& user_id,
                            const std::string& title, const std::string& content,
                            const std::vector<int64_t>& tag_ids);

    int64_t RegisterEvent(int64_t event_id, const std::string& user_id,
                          int32_t ticket_type, int32_t guest_count,
                          const std::string& contact);

    std::vector<EventRegEntity> GetEventRegistrations(
        int64_t event_id, const std::string& owner_id, int limit, int offset);

    int GetRegistrationCount(int64_t event_id);
    int GetConfirmedCount(int64_t event_id);

    bool UpdateRegistrationStatus(int64_t reg_id, const std::string& owner_id, int32_t new_status);

    void CreateMention(const std::string& from_id, const std::vector<std::string>& to_ids,
                       int64_t post_id, int64_t comment_id, const std::string& preview);

    std::vector<MentionEntity> GetUserMentions(
        const std::string& user_id, bool only_unread, int limit, int offset);

    int GetMentionCount(const std::string& user_id, bool only_unread);
    int GetUnreadMentionCount(const std::string& user_id);

    void MarkMentionRead(const std::string& user_id, int64_t mention_id, bool mark_all);

    bool SetPostFavorite(const std::string& user_id, int64_t post_id, bool favorite);

    std::vector<int64_t> GetFavoritePostIds(
        const std::string& user_id, int limit, int offset);

    int GetFavoritePostCount(const std::string& user_id);

    int64_t SaveDraft(const std::string& user_id, int64_t draft_id,
                      const std::string& title, const std::string& content,
                      int32_t section_id, const std::vector<int64_t>& tag_ids,
                      int64_t group_id);

    std::vector<DraftEntity> GetUserDrafts(
        const std::string& user_id, int limit, int offset);

    int GetDraftCount(const std::string& user_id);

    bool DeleteDraft(const std::string& user_id, int64_t draft_id);

    std::vector<std::string> ExtractMentions(const std::string& content);

private:
    AdvancedRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_ADVANCED_REPOSITORY_H
