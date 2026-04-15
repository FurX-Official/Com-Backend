#ifndef FURBBS_SERVICE_IMPL_ENHANCED_SERVICE_H
#define FURBBS_SERVICE_IMPL_ENHANCED_SERVICE_H

#include "repository/enhanced_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class EnhancedService {
public:
    static EnhancedService& Instance() {
        static EnhancedService instance;
        return instance;
    }

    bool LikeComment(const std::string& token, int64_t comment_id);
    bool UnlikeComment(const std::string& token, int64_t comment_id);
    int32_t GetCommentLikeCount(int64_t comment_id);
    bool HasLikedComment(const std::string& token, int64_t comment_id);

    bool RatePost(const std::string& token, int64_t post_id,
                  int32_t score, const std::string& comment);
    std::vector<repository::AppreciationEntity> GetPostAppreciations(
        int64_t post_id, int page, int page_size);
    std::pair<int32_t, double> GetPostRatingStats(int64_t post_id);

    bool RecordShare(const std::string& token, int64_t post_id,
                     const std::string& platform);
    int32_t GetShareCount(int64_t post_id);
    std::vector<std::pair<std::string, int32_t>> GetSharePlatformStats(
        int64_t post_id);

    bool RecordVisit(const std::string& visitor_id,
                     const std::string& target_user_id, int64_t post_id,
                     const std::string& ip, const std::string& ua, int32_t duration);
    std::vector<repository::VisitRecordEntity> GetProfileVisitors(
        const std::string& token, int page, int page_size);

    bool AddUserTag(const std::string& token, const std::string& tagged_user_id,
                    const std::string& tag_name, const std::string& tag_color);
    bool RemoveUserTag(const std::string& token, const std::string& tagged_user_id,
                       const std::string& tag_name);
    std::vector<repository::UserTagEntity> GetUserTags(
        const std::string& token, const std::string& tagged_user_id);

    bool SetReadingProgress(const std::string& token, int64_t post_id,
                            int32_t progress_percent, int32_t position,
                            int32_t total_words);
    std::optional<repository::ReadingProgressEntity> GetReadingProgress(
        const std::string& token, int64_t post_id);
    std::vector<repository::ReadingProgressEntity> GetReadingHistory(
        const std::string& token, int page, int page_size);

    int64_t AddNote(const std::string& token, const std::string& target_type,
                    int64_t target_id, const std::string& content,
                    const std::string& color, bool is_pinned);
    bool UpdateNote(const std::string& token, int64_t note_id,
                    const std::string& content, const std::string& color,
                    bool is_pinned);
    bool DeleteNote(const std::string& token, int64_t note_id);
    std::vector<repository::UserNoteEntity> GetNotes(
        const std::string& token, const std::string& target_type,
        int64_t target_id);

    void RecordContentEdit(int64_t post_id, int64_t comment_id,
                           const std::string& edited_by,
                           const std::string& old_title,
                           const std::string& old_content,
                           const std::string& new_title,
                           const std::string& new_content,
                           const std::string& reason);
    std::vector<repository::ContentHistoryEntity> GetContentHistory(
        const std::string& token, int64_t post_id, int64_t comment_id,
        int page, int page_size);

    bool AddQuickAccess(const std::string& token, const std::string& item_type,
                        int64_t item_id, const std::string& item_name,
                        const std::string& item_icon, int32_t sort_order);
    bool RemoveQuickAccess(const std::string& token, const std::string& item_type,
                           int64_t item_id);
    std::vector<repository::QuickAccessEntity> GetQuickAccess(
        const std::string& token);

    std::vector<repository::FilterWordEntity> GetAllFilterWords();
    bool AddFilterWord(const std::string& token, const std::string& word,
                       const std::string& replacement, int32_t level,
                       bool is_regex);
    bool RemoveFilterWord(const std::string& token, int64_t id);
    std::string FilterContent(const std::string& content);

    int64_t CreateSeries(const std::string& token, const std::string& title,
                         const std::string& description,
                         const std::string& cover_image, bool is_public);
    bool UpdateSeries(const std::string& token, int64_t series_id,
                      const std::string& title, const std::string& description,
                      const std::string& cover_image, bool is_public);
    bool DeleteSeries(const std::string& token, int64_t series_id);
    std::vector<repository::SeriesEntity> GetUserSeries(
        const std::string& token, const std::string& user_id,
        int page, int page_size);
    bool AddPostToSeries(const std::string& token, int64_t series_id,
                         int64_t post_id, int32_t sort_order);
    bool RemovePostFromSeries(const std::string& token, int64_t series_id,
                              int64_t post_id);
    std::vector<repository::SeriesPostEntity> GetSeriesPosts(
        int64_t series_id);

    std::optional<repository::UserPreferencesEntity> GetUserPreferences(
        const std::string& token);
    bool SetUserPreferences(const std::string& token,
                            const repository::UserPreferencesEntity& prefs);

    void CalculateContributorRankings();
    std::vector<repository::ContributorRankingEntity> GetTopContributors(
        int limit, int offset);

private:
    EnhancedService() = default;

    std::string ValidateAndGetUserId(const std::string& token);

    bool IsAdmin(const std::string& user_id);
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_ENHANCED_SERVICE_H
