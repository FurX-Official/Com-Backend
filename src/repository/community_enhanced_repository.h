#ifndef FURBBS_REPOSITORY_COMMUNITY_ENHANCED_REPOSITORY_H
#define FURBBS_REPOSITORY_COMMUNITY_ENHANCED_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <cstdint>

namespace furbbs::repository {

struct ContentReportEntity {
    int64_t id = 0;
    std::string reporter_id;
    std::string content_type;
    int64_t content_id = 0;
    std::string report_reason;
    std::string report_details;
    std::string status;
    std::string handled_by;
    int64_t handled_at = 0;
    std::string handler_notes;
    int64_t created_at = 0;
};

struct CommentReplyEntity {
    int64_t id = 0;
    int64_t comment_id = 0;
    int64_t parent_reply_id = 0;
    int64_t post_id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    std::string reply_to_user_id;
    std::string reply_to_username;
    std::string content;
    int32_t like_count = 0;
    bool is_liked = false;
    bool is_deleted = false;
    int64_t created_at = 0;
};

struct CollectionFolderEntity {
    int64_t id = 0;
    std::string user_id;
    std::string name;
    std::string description;
    bool is_public = false;
    int32_t item_count = 0;
    int64_t created_at = 0;
};

struct UserKeywordFilterEntity {
    int64_t id = 0;
    std::string user_id;
    std::string keyword;
    std::string filter_type;
    int64_t created_at = 0;
};

struct PostDraftEntity {
    int64_t id = 0;
    std::string user_id;
    std::string title;
    std::string content;
    int64_t section_id = 0;
    std::vector<std::string> tags;
    int64_t fursona_id = 0;
    bool is_auto_save = false;
    int64_t updated_at = 0;
    int64_t created_at = 0;
};

struct PostShareStatsEntity {
    int64_t post_id = 0;
    int32_t view_count = 0;
    int32_t share_count = 0;
    int32_t download_count = 0;
    int64_t last_viewed_at = 0;
};

class CommunityEnhancedRepository : protected BaseRepository {
public:
    static CommunityEnhancedRepository& Instance() {
        static CommunityEnhancedRepository instance;
        return instance;
    }

    int64_t CreateReport(const ContentReportEntity& data);
    std::vector<ContentReportEntity> GetPendingReports(int limit, int offset);
    bool HandleReport(int64_t report_id, const std::string& status,
                      const std::string& handled_by, const std::string& notes);

    int64_t CreateReply(const CommentReplyEntity& data);
    std::vector<CommentReplyEntity> GetCommentReplies(int64_t comment_id);
    bool DeleteReply(int64_t reply_id, const std::string& user_id);
    bool LikeReply(int64_t reply_id);

    bool SetSticky(int64_t post_id, int64_t section_id, int priority, const std::string& sticky_by);
    bool RemoveSticky(int64_t post_id);
    std::vector<int64_t> GetSectionStickyPosts(int64_t section_id);

    bool SetDigest(int64_t post_id, const std::string& level,
                   const std::string& recommended_by, const std::string& description);
    bool RemoveDigest(int64_t post_id);
    std::vector<int64_t> GetDigestPosts(const std::string& level, int limit, int offset);

    int64_t CreateFolder(const CollectionFolderEntity& data);
    std::vector<CollectionFolderEntity> GetUserFolders(const std::string& user_id);
    bool AddToFolder(int64_t folder_id, int64_t post_id);
    bool RemoveFromFolder(int64_t folder_id, int64_t post_id);
    std::vector<int64_t> GetFolderItems(int64_t folder_id, int limit, int offset);

    bool AddUserTag(const std::string& tagger_id, const std::string& tagged_id, const std::string& tag);
    bool RemoveUserTag(const std::string& tagger_id, const std::string& tagged_id, const std::string& tag);
    std::vector<std::string> GetUserTags(const std::string& tagger_id, const std::string& tagged_id);

    bool AddKeywordFilter(const UserKeywordFilterEntity& data);
    bool RemoveKeywordFilter(int64_t filter_id, const std::string& user_id);
    std::vector<UserKeywordFilterEntity> GetUserFilters(const std::string& user_id);

    int64_t SaveDraft(const PostDraftEntity& data);
    std::vector<PostDraftEntity> GetUserDrafts(const std::string& user_id);
    bool DeleteDraft(int64_t draft_id, const std::string& user_id);

    bool IncrementViewCount(int64_t post_id);
    bool IncrementShareCount(int64_t post_id);
    std::optional<PostShareStatsEntity> GetPostStats(int64_t post_id);

private:
    CommunityEnhancedRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_COMMUNITY_ENHANCED_REPOSITORY_H
