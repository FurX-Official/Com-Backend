#include "enhanced_service.h"
#include "repository/user_repository.h"
#include "repository/post_repository.h"
#include <chrono>

namespace furbbs::service {

std::string EnhancedService::ValidateAndGetUserId(const std::string& token) {
    auto auth_result = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!auth_result) {
        return "";
    }
    return auth_result->user_id;
}

bool EnhancedService::IsAdmin(const std::string& user_id) {
    auto user = repository::UserRepository::Instance().GetUserInfo(user_id);
    if (!user) {
        return false;
    }
    return user->role == "admin" || user->role == "super_admin";
}

bool EnhancedService::LikeComment(const std::string& token, int64_t comment_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().LikeComment(comment_id, user_id);
}

bool EnhancedService::UnlikeComment(const std::string& token, int64_t comment_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().UnlikeComment(comment_id, user_id);
}

int32_t EnhancedService::GetCommentLikeCount(int64_t comment_id) {
    return repository::EnhancedRepository::Instance().GetCommentLikeCount(comment_id);
}

bool EnhancedService::HasLikedComment(const std::string& token, int64_t comment_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().HasLikedComment(comment_id, user_id);
}

bool EnhancedService::RatePost(const std::string& token, int64_t post_id,
                               int32_t score, const std::string& comment) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    if (score < 1 || score > 10) {
        return false;
    }
    return repository::EnhancedRepository::Instance().RatePost(
        post_id, user_id, score, comment);
}

std::vector<repository::AppreciationEntity> EnhancedService::GetPostAppreciations(
    int64_t post_id, int page, int page_size) {
    return repository::EnhancedRepository::Instance().GetPostAppreciations(
        post_id, page, page_size);
}

std::pair<int32_t, double> EnhancedService::GetPostRatingStats(int64_t post_id) {
    return repository::EnhancedRepository::Instance().GetPostRatingStats(post_id);
}

bool EnhancedService::RecordShare(const std::string& token, int64_t post_id,
                                  const std::string& platform) {
    std::string user_id = ValidateAndGetUserId(token);
    return repository::EnhancedRepository::Instance().RecordShare(
        post_id, user_id, platform);
}

int32_t EnhancedService::GetShareCount(int64_t post_id) {
    return repository::EnhancedRepository::Instance().GetShareCount(post_id);
}

std::vector<std::pair<std::string, int32_t>> EnhancedService::GetSharePlatformStats(
    int64_t post_id) {
    return repository::EnhancedRepository::Instance().GetSharePlatformStats(post_id);
}

bool EnhancedService::RecordVisit(const std::string& visitor_id,
                                  const std::string& target_user_id, int64_t post_id,
                                  const std::string& ip, const std::string& ua, int32_t duration) {
    return repository::EnhancedRepository::Instance().RecordVisit(
        visitor_id, target_user_id, post_id, ip, ua, duration);
}

std::vector<repository::VisitRecordEntity> EnhancedService::GetProfileVisitors(
    const std::string& token, int page, int page_size) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::EnhancedRepository::Instance().GetProfileVisitors(
        user_id, page, page_size);
}

bool EnhancedService::AddUserTag(const std::string& token, const std::string& tagged_user_id,
                                 const std::string& tag_name, const std::string& tag_color) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().AddUserTag(
        user_id, tagged_user_id, tag_name, tag_color);
}

bool EnhancedService::RemoveUserTag(const std::string& token, const std::string& tagged_user_id,
                                    const std::string& tag_name) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().RemoveUserTag(
        user_id, tagged_user_id, tag_name);
}

std::vector<repository::UserTagEntity> EnhancedService::GetUserTags(
    const std::string& token, const std::string& tagged_user_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::EnhancedRepository::Instance().GetUserTags(
        user_id, tagged_user_id);
}

bool EnhancedService::SetReadingProgress(const std::string& token, int64_t post_id,
                                         int32_t progress_percent, int32_t position,
                                         int32_t total_words) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::ReadingProgressEntity data;
    data.user_id = user_id;
    data.post_id = post_id;
    data.progress_percent = progress_percent;
    data.last_read_position = position;
    data.total_words = total_words;
    data.last_read_at = now_ms;
    data.is_completed = progress_percent >= 95;
    return repository::EnhancedRepository::Instance().SetReadingProgress(data);
}

std::optional<repository::ReadingProgressEntity> EnhancedService::GetReadingProgress(
    const std::string& token, int64_t post_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return std::nullopt;
    }
    return repository::EnhancedRepository::Instance().GetReadingProgress(
        user_id, post_id);
}

std::vector<repository::ReadingProgressEntity> EnhancedService::GetReadingHistory(
    const std::string& token, int page, int page_size) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::EnhancedRepository::Instance().GetReadingHistory(
        user_id, page, page_size);
}

int64_t EnhancedService::AddNote(const std::string& token, const std::string& target_type,
                                 int64_t target_id, const std::string& content,
                                 const std::string& color, bool is_pinned) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return 0;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::UserNoteEntity note;
    note.user_id = user_id;
    note.target_type = target_type;
    note.target_id = target_id;
    note.note_content = content;
    note.color = color;
    note.is_pinned = is_pinned;
    note.updated_at = now_ms;
    return repository::EnhancedRepository::Instance().AddNote(note);
}

bool EnhancedService::UpdateNote(const std::string& token, int64_t note_id,
                                 const std::string& content, const std::string& color,
                                 bool is_pinned) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::UserNoteEntity note;
    note.user_id = user_id;
    note.note_content = content;
    note.color = color;
    note.is_pinned = is_pinned;
    note.updated_at = now_ms;
    return repository::EnhancedRepository::Instance().UpdateNote(note_id, note);
}

bool EnhancedService::DeleteNote(const std::string& token, int64_t note_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().DeleteNote(note_id, user_id);
}

std::vector<repository::UserNoteEntity> EnhancedService::GetNotes(
    const std::string& token, const std::string& target_type, int64_t target_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::EnhancedRepository::Instance().GetNotes(
        user_id, target_type, target_id);
}

void EnhancedService::RecordContentEdit(int64_t post_id, int64_t comment_id,
                                        const std::string& edited_by,
                                        const std::string& old_title,
                                        const std::string& old_content,
                                        const std::string& new_title,
                                        const std::string& new_content,
                                        const std::string& reason) {
    repository::ContentHistoryEntity history;
    history.post_id = post_id;
    history.comment_id = comment_id;
    history.edited_by = edited_by;
    history.old_title = old_title;
    history.old_content = old_content;
    history.new_title = new_title;
    history.new_content = new_content;
    history.edit_reason = reason;
    repository::EnhancedRepository::Instance().RecordContentEdit(history);
}

std::vector<repository::ContentHistoryEntity> EnhancedService::GetContentHistory(
    const std::string& token, int64_t post_id, int64_t comment_id,
    int page, int page_size) {
    std::string user_id = ValidateAndGetUserId(token);
    if (!IsAdmin(user_id)) {
        return {};
    }
    return repository::EnhancedRepository::Instance().GetContentHistory(
        post_id, comment_id, page, page_size);
}

bool EnhancedService::AddQuickAccess(const std::string& token, const std::string& item_type,
                                     int64_t item_id, const std::string& item_name,
                                     const std::string& item_icon, int32_t sort_order) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    repository::QuickAccessEntity item;
    item.user_id = user_id;
    item.item_type = item_type;
    item.item_id = item_id;
    item.item_name = item_name;
    item.item_icon = item_icon;
    item.sort_order = sort_order;
    return repository::EnhancedRepository::Instance().AddQuickAccess(item);
}

bool EnhancedService::RemoveQuickAccess(const std::string& token, const std::string& item_type,
                                        int64_t item_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().RemoveQuickAccess(
        user_id, item_type, item_id);
}

std::vector<repository::QuickAccessEntity> EnhancedService::GetQuickAccess(
    const std::string& token) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::EnhancedRepository::Instance().GetQuickAccess(user_id);
}

std::vector<repository::FilterWordEntity> EnhancedService::GetAllFilterWords() {
    return repository::EnhancedRepository::Instance().GetAllFilterWords();
}

bool EnhancedService::AddFilterWord(const std::string& token, const std::string& word,
                                    const std::string& replacement, int32_t level,
                                    bool is_regex) {
    std::string user_id = ValidateAndGetUserId(token);
    if (!IsAdmin(user_id)) {
        return false;
    }
    repository::FilterWordEntity item;
    item.word = word;
    item.replacement = replacement;
    item.filter_level = level;
    item.is_regex = is_regex;
    item.created_by = user_id;
    return repository::EnhancedRepository::Instance().AddFilterWord(item);
}

bool EnhancedService::RemoveFilterWord(const std::string& token, int64_t id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (!IsAdmin(user_id)) {
        return false;
    }
    return repository::EnhancedRepository::Instance().RemoveFilterWord(id);
}

std::string EnhancedService::FilterContent(const std::string& content) {
    return repository::EnhancedRepository::Instance().FilterContent(content);
}

int64_t EnhancedService::CreateSeries(const std::string& token, const std::string& title,
                                      const std::string& description,
                                      const std::string& cover_image, bool is_public) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return 0;
    }
    repository::SeriesEntity series;
    series.user_id = user_id;
    series.title = title;
    series.description = description;
    series.cover_image = cover_image;
    series.is_public = is_public;
    return repository::EnhancedRepository::Instance().CreateSeries(series);
}

bool EnhancedService::UpdateSeries(const std::string& token, int64_t series_id,
                                   const std::string& title, const std::string& description,
                                   const std::string& cover_image, bool is_public) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::SeriesEntity series;
    series.user_id = user_id;
    series.title = title;
    series.description = description;
    series.cover_image = cover_image;
    series.is_public = is_public;
    series.updated_at = now_ms;
    return repository::EnhancedRepository::Instance().UpdateSeries(series_id, series);
}

bool EnhancedService::DeleteSeries(const std::string& token, int64_t series_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::EnhancedRepository::Instance().DeleteSeries(series_id, user_id);
}

std::vector<repository::SeriesEntity> EnhancedService::GetUserSeries(
    const std::string& token, const std::string& user_id, int page, int page_size) {
    std::string requester_id = ValidateAndGetUserId(token);
    return repository::EnhancedRepository::Instance().GetUserSeries(
        user_id, page, page_size);
}

bool EnhancedService::AddPostToSeries(const std::string& token, int64_t series_id,
                                      int64_t post_id, int32_t sort_order) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto series_list = repository::EnhancedRepository::Instance().GetUserSeries(
        user_id, 0, 1000);
    bool owns_series = false;
    for (auto& s : series_list) {
        if (s.id == series_id) {
            owns_series = true;
            break;
        }
    }
    if (!owns_series) {
        return false;
    }
    return repository::EnhancedRepository::Instance().AddPostToSeries(
        series_id, post_id, sort_order);
}

bool EnhancedService::RemovePostFromSeries(const std::string& token, int64_t series_id,
                                           int64_t post_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto series_list = repository::EnhancedRepository::Instance().GetUserSeries(
        user_id, 0, 1000);
    bool owns_series = false;
    for (auto& s : series_list) {
        if (s.id == series_id) {
            owns_series = true;
            break;
        }
    }
    if (!owns_series) {
        return false;
    }
    return repository::EnhancedRepository::Instance().RemovePostFromSeries(
        series_id, post_id);
}

std::vector<repository::SeriesPostEntity> EnhancedService::GetSeriesPosts(
    int64_t series_id) {
    return repository::EnhancedRepository::Instance().GetSeriesPosts(series_id);
}

std::optional<repository::UserPreferencesEntity> EnhancedService::GetUserPreferences(
    const std::string& token) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return std::nullopt;
    }
    auto prefs = repository::EnhancedRepository::Instance().GetUserPreferences(user_id);
    if (!prefs) {
        repository::UserPreferencesEntity default_prefs;
        default_prefs.user_id = user_id;
        return default_prefs;
    }
    return prefs;
}

bool EnhancedService::SetUserPreferences(const std::string& token,
                                         const repository::UserPreferencesEntity& prefs) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::UserPreferencesEntity data = prefs;
    data.user_id = user_id;
    data.updated_at = now_ms;
    return repository::EnhancedRepository::Instance().SetUserPreferences(data);
}

void EnhancedService::CalculateContributorRankings() {
    auto users = repository::UserRepository::Instance().GetRecentActiveUsers(1000);
    for (auto& uid : users) {
        auto post_stats = repository::PostRepository::Instance().GetUserPostStats(uid);
        double score = post_stats.first * 10.0 + post_stats.second * 2.0;
        repository::ContributorRankingEntity ranking;
        ranking.user_id = uid;
        ranking.contribution_score = score;
        ranking.posts_weight = post_stats.first;
        ranking.comments_weight = post_stats.second;
        auto now = std::chrono::system_clock::now();
        ranking.last_updated = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        repository::EnhancedRepository::Instance().UpdateContributorRanking(ranking);
    }
}

std::vector<repository::ContributorRankingEntity> EnhancedService::GetTopContributors(
    int limit, int offset) {
    return repository::EnhancedRepository::Instance().GetTopContributors(limit, offset);
}

} // namespace furbbs::service
