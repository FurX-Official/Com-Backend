#ifndef FURBBS_REPOSITORY_ENHANCED_REPOSITORY_H
#define FURBBS_REPOSITORY_ENHANCED_REPOSITORY_H

#include "repository/base_repository.h"
#include "db/sql_queries.h"
#include <vector>
#include <string>
#include <optional>
#include <utility>

namespace furbbs::repository {

struct CommentLikeEntity {
    int64_t id = 0;
    int64_t comment_id = 0;
    std::string user_id;
    int64_t created_at = 0;
};

struct AppreciationEntity {
    int64_t id = 0;
    int64_t post_id = 0;
    std::string user_id;
    std::string username;
    int32_t score = 5;
    std::string comment;
    int64_t created_at = 0;
};

struct ShareRecordEntity {
    int64_t id = 0;
    int64_t post_id = 0;
    std::string user_id;
    std::string platform;
    int64_t shared_at = 0;
};

struct VisitRecordEntity {
    int64_t id = 0;
    std::string visitor_id;
    std::string target_user_id;
    int64_t post_id = 0;
    std::string ip_address;
    std::string user_agent;
    int32_t duration = 0;
    int64_t visited_at = 0;
};

struct UserTagEntity {
    int64_t id = 0;
    std::string user_id;
    std::string tagged_user_id;
    std::string tag_name;
    std::string tag_color;
    int64_t created_at = 0;
};

struct ReadingProgressEntity {
    std::string user_id;
    int64_t post_id = 0;
    int32_t progress_percent = 0;
    int32_t last_read_position = 0;
    int32_t total_words = 0;
    int64_t last_read_at = 0;
    bool is_completed = false;
};

struct UserNoteEntity {
    int64_t id = 0;
    std::string user_id;
    std::string target_type;
    int64_t target_id = 0;
    std::string note_content;
    std::string color;
    bool is_pinned = false;
    int64_t created_at = 0;
    int64_t updated_at = 0;
};

struct ContentHistoryEntity {
    int64_t id = 0;
    int64_t post_id = 0;
    int64_t comment_id = 0;
    std::string edited_by;
    std::string old_title;
    std::string old_content;
    std::string new_title;
    std::string new_content;
    std::string edit_reason;
    int64_t edited_at = 0;
};

struct QuickAccessEntity {
    int64_t id = 0;
    std::string user_id;
    std::string item_type;
    int64_t item_id = 0;
    std::string item_name;
    std::string item_icon;
    int32_t sort_order = 0;
    int64_t created_at = 0;
};

struct FilterWordEntity {
    int64_t id = 0;
    std::string word;
    std::string replacement;
    int32_t filter_level = 1;
    bool is_regex = false;
    std::string created_by;
    int64_t created_at = 0;
};

struct SeriesEntity {
    int64_t id = 0;
    std::string user_id;
    std::string title;
    std::string description;
    std::string cover_image;
    bool is_public = true;
    int32_t post_count = 0;
    int32_t view_count = 0;
    int64_t created_at = 0;
    int64_t updated_at = 0;
};

struct SeriesPostEntity {
    int64_t series_id = 0;
    int64_t post_id = 0;
    std::string post_title;
    int32_t sort_order = 0;
    int64_t added_at = 0;
};

struct UserPreferencesEntity {
    std::string user_id;
    std::string content_language = "zh";
    std::string default_sort = "hot";
    bool show_nsfw = false;
    bool blur_nsfw = true;
    bool auto_play_video = true;
    bool infinite_scroll = true;
    bool compact_mode = false;
    std::string night_mode = "auto";
    std::string font_size = "medium";
    bool notify_on_like = true;
    bool notify_on_comment = true;
    bool notify_on_follow = true;
    bool notify_on_mention = true;
    bool notify_on_message = true;
    bool email_digest = true;
    bool show_online_status = true;
    bool show_read_status = true;
    int64_t updated_at = 0;
};

struct ContributorRankingEntity {
    std::string user_id;
    std::string username;
    std::string avatar;
    double contribution_score = 0;
    int32_t posts_weight = 0;
    int32_t comments_weight = 0;
    int32_t likes_weight = 0;
    int32_t uploads_weight = 0;
    int32_t help_weight = 0;
    int64_t last_updated = 0;
};

class EnhancedRepository : protected BaseRepository {
public:
    static EnhancedRepository& Instance() {
        static EnhancedRepository instance;
        return instance;
    }

    bool LikeComment(int64_t comment_id, const std::string& user_id);
    bool UnlikeComment(int64_t comment_id, const std::string& user_id);
    int32_t GetCommentLikeCount(int64_t comment_id);
    bool HasLikedComment(int64_t comment_id, const std::string& user_id);

    bool RatePost(int64_t post_id, const std::string& user_id,
                  int32_t score, const std::string& comment);
    std::vector<AppreciationEntity> GetPostAppreciations(
        int64_t post_id, int page, int page_size);
    std::pair<int32_t, double> GetPostRatingStats(int64_t post_id);

    bool RecordShare(int64_t post_id, const std::string& user_id,
                     const std::string& platform);
    int32_t GetShareCount(int64_t post_id);
    std::vector<std::pair<std::string, int32_t>> GetSharePlatformStats(
        int64_t post_id);

    bool RecordVisit(const std::string& visitor_id,
                     const std::string& target_user_id, int64_t post_id,
                     const std::string& ip, const std::string& ua, int32_t duration);
    std::vector<VisitRecordEntity> GetProfileVisitors(
        const std::string& user_id, int page, int page_size);

    bool AddUserTag(const std::string& user_id, const std::string& tagged_user_id,
                    const std::string& tag_name, const std::string& tag_color);
    bool RemoveUserTag(const std::string& user_id, const std::string& tagged_user_id,
                       const std::string& tag_name);
    std::vector<UserTagEntity> GetUserTags(
        const std::string& user_id, const std::string& tagged_user_id);

    bool SetReadingProgress(const ReadingProgressEntity& data);
    std::optional<ReadingProgressEntity> GetReadingProgress(
        const std::string& user_id, int64_t post_id);
    std::vector<ReadingProgressEntity> GetReadingHistory(
        const std::string& user_id, int page, int page_size);

    int64_t AddNote(const UserNoteEntity& data);
    bool UpdateNote(int64_t note_id, const UserNoteEntity& data);
    bool DeleteNote(int64_t note_id, const std::string& user_id);
    std::vector<UserNoteEntity> GetNotes(
        const std::string& user_id, const std::string& target_type,
        int64_t target_id);

    bool RecordContentEdit(const ContentHistoryEntity& data);
    std::vector<ContentHistoryEntity> GetContentHistory(
        int64_t post_id, int64_t comment_id, int page, int page_size);

    bool AddQuickAccess(const QuickAccessEntity& data);
    bool RemoveQuickAccess(const std::string& user_id,
                           const std::string& item_type, int64_t item_id);
    std::vector<QuickAccessEntity> GetQuickAccess(const std::string& user_id);

    std::vector<FilterWordEntity> GetAllFilterWords();
    bool AddFilterWord(const FilterWordEntity& data);
    bool RemoveFilterWord(int64_t id);
    std::string FilterContent(const std::string& content);

    int64_t CreateSeries(const SeriesEntity& data);
    bool UpdateSeries(int64_t series_id, const SeriesEntity& data);
    bool DeleteSeries(int64_t series_id, const std::string& user_id);
    std::vector<SeriesEntity> GetUserSeries(
        const std::string& user_id, int page, int page_size);
    bool AddPostToSeries(int64_t series_id, int64_t post_id, int32_t sort_order);
    bool RemovePostFromSeries(int64_t series_id, int64_t post_id);
    std::vector<SeriesPostEntity> GetSeriesPosts(int64_t series_id);

    std::optional<UserPreferencesEntity> GetUserPreferences(
        const std::string& user_id);
    bool SetUserPreferences(const UserPreferencesEntity& data);

    bool UpdateContributorRanking(const ContributorRankingEntity& data);
    std::vector<ContributorRankingEntity> GetTopContributors(
        int limit, int offset);

private:
    EnhancedRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_ENHANCED_REPOSITORY_H
