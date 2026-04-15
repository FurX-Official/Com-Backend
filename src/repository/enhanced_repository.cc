#include "enhanced_repository.h"
#include "db/sql_queries.h"
#include <pqxx/pqxx>
#include <regex>

namespace furbbs::repository {

bool EnhancedRepository::LikeComment(int64_t comment_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::COMMENT_LIKE_ADD, comment_id, user_id);
        return true;
    });
}

bool EnhancedRepository::UnlikeComment(int64_t comment_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::COMMENT_LIKE_REMOVE, comment_id, user_id);
        return true;
    });
}

int32_t EnhancedRepository::GetCommentLikeCount(int64_t comment_id) {
    return ExecuteQuery<int32_t>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::COMMENT_LIKE_COUNT, comment_id);
        return result[0][0].as<int32_t>();
    });
}

bool EnhancedRepository::HasLikedComment(int64_t comment_id, const std::string& user_id) {
    return ExecuteQuery<bool>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::COMMENT_LIKE_HAS, comment_id, user_id);
        return !result.empty();
    });
}

bool EnhancedRepository::RatePost(int64_t post_id, const std::string& user_id,
                                  int32_t score, const std::string& comment) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::POST_APPRECIATION_ADD,
            post_id, user_id, score, comment);
        return true;
    });
}

std::vector<AppreciationEntity> EnhancedRepository::GetPostAppreciations(
    int64_t post_id, int page, int page_size) {
    return ExecuteQuery<std::vector<AppreciationEntity>>([&](pqxx::work& tx) {
        std::vector<AppreciationEntity> result;
        auto res = tx.exec_params(sql::POST_APPRECIATION_GET,
            post_id, page_size, page * page_size);
        for (auto& row : res) {
            AppreciationEntity item;
            item.id = row[0].as<int64_t>();
            item.post_id = row[1].as<int64_t>();
            item.user_id = row[2].as<std::string>();
            item.username = row[3].as<std::string>();
            item.score = row[4].as<int32_t>();
            item.comment = row[5].as<std::string>();
            item.created_at = row[6].as<int64_t>();
            result.push_back(item);
        }
        return result;
    });
}

std::pair<int32_t, double> EnhancedRepository::GetPostRatingStats(int64_t post_id) {
    return ExecuteQuery<std::pair<int32_t, double>>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::POST_APPRECIATION_STATS, post_id);
        int32_t count = result[0][0].as<int32_t>();
        double avg = result[0][1].is_null() ? 0 : result[0][1].as<double>();
        return std::make_pair(count, avg);
    });
}

bool EnhancedRepository::RecordShare(int64_t post_id, const std::string& user_id,
                                     const std::string& platform) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SHARE_RECORD_ADD, post_id, user_id, platform);
        return true;
    });
}

int32_t EnhancedRepository::GetShareCount(int64_t post_id) {
    return ExecuteQuery<int32_t>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::SHARE_RECORD_COUNT, post_id);
        return result[0][0].as<int32_t>();
    });
}

std::vector<std::pair<std::string, int32_t>> EnhancedRepository::GetSharePlatformStats(
    int64_t post_id) {
    return ExecuteQuery<std::vector<std::pair<std::string, int32_t>>>([&](pqxx::work& tx) {
        std::vector<std::pair<std::string, int32_t>> result;
        auto res = tx.exec_params(sql::SHARE_PLATFORM_STATS, post_id);
        for (auto& row : res) {
            result.emplace_back(
                row[0].as<std::string>(),
                row[1].as<int32_t>()
            );
        }
        return result;
    });
}

bool EnhancedRepository::RecordVisit(const std::string& visitor_id,
                                     const std::string& target_user_id, int64_t post_id,
                                     const std::string& ip, const std::string& ua, int32_t duration) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::VISIT_RECORD_ADD,
            visitor_id.empty() ? nullptr : visitor_id.c_str(),
            target_user_id.empty() ? nullptr : target_user_id.c_str(),
            post_id > 0 ? post_id : nullptr,
            ip, ua, duration);
        return true;
    });
}

std::vector<VisitRecordEntity> EnhancedRepository::GetProfileVisitors(
    const std::string& user_id, int page, int page_size) {
    return ExecuteQuery<std::vector<VisitRecordEntity>>([&](pqxx::work& tx) {
        std::vector<VisitRecordEntity> result;
        auto res = tx.exec_params(sql::VISIT_GET_PROFILE_VISITORS,
            user_id, page_size, page * page_size);
        for (auto& row : res) {
            VisitRecordEntity item;
            item.visitor_id = row[0].as<std::string>();
            item.visited_at = row[3].as<int64_t>();
            result.push_back(item);
        }
        return result;
    });
}

bool EnhancedRepository::AddUserTag(const std::string& user_id, const std::string& tagged_user_id,
                                    const std::string& tag_name, const std::string& tag_color) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::USER_TAG_ADD, user_id, tagged_user_id, tag_name, tag_color);
        return true;
    });
}

bool EnhancedRepository::RemoveUserTag(const std::string& user_id, const std::string& tagged_user_id,
                                       const std::string& tag_name) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::USER_TAG_REMOVE, user_id, tagged_user_id, tag_name);
        return true;
    });
}

std::vector<UserTagEntity> EnhancedRepository::GetUserTags(
    const std::string& user_id, const std::string& tagged_user_id) {
    return ExecuteQuery<std::vector<UserTagEntity>>([&](pqxx::work& tx) {
        std::vector<UserTagEntity> result;
        auto res = tx.exec_params(sql::USER_TAG_GET, user_id, tagged_user_id);
        for (auto& row : res) {
            UserTagEntity item;
            item.tag_name = row[0].as<std::string>();
            item.tag_color = row[1].as<std::string>();
            result.push_back(item);
        }
        return result;
    });
}

bool EnhancedRepository::SetReadingProgress(const ReadingProgressEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::READING_PROGRESS_SET,
            data.user_id, data.post_id, data.progress_percent,
            data.last_read_position, data.total_words,
            data.last_read_at, data.is_completed);
        return true;
    });
}

std::optional<ReadingProgressEntity> EnhancedRepository::GetReadingProgress(
    const std::string& user_id, int64_t post_id) {
    return ExecuteQuery<std::optional<ReadingProgressEntity>>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::READING_PROGRESS_GET, user_id, post_id);
        if (result.empty()) {
            return std::nullopt;
        }
        ReadingProgressEntity data;
        data.progress_percent = result[0][0].as<int32_t>();
        data.last_read_position = result[0][1].as<int32_t>();
        data.total_words = result[0][2].as<int32_t>();
        data.is_completed = result[0][3].as<bool>();
        return data;
    });
}

std::vector<ReadingProgressEntity> EnhancedRepository::GetReadingHistory(
    const std::string& user_id, int page, int page_size) {
    return ExecuteQuery<std::vector<ReadingProgressEntity>>([&](pqxx::work& tx) {
        std::vector<ReadingProgressEntity> result;
        auto res = tx.exec_params(sql::READING_HISTORY,
            user_id, page_size, page * page_size);
        for (auto& row : res) {
            ReadingProgressEntity item;
            item.post_id = row[0].as<int64_t>();
            item.last_read_at = row[2].as<int64_t>();
            item.progress_percent = row[3].as<int32_t>();
            result.push_back(item);
        }
        return result;
    });
}

int64_t EnhancedRepository::AddNote(const UserNoteEntity& data) {
    return ExecuteQuery<int64_t>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::USER_NOTE_ADD,
            data.user_id, data.target_type, data.target_id,
            data.note_content, data.color, data.is_pinned, data.updated_at);
        return result[0][0].as<int64_t>();
    });
}

bool EnhancedRepository::UpdateNote(int64_t note_id, const UserNoteEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::USER_NOTE_UPDATE,
            data.note_content, data.color, data.is_pinned,
            data.updated_at, note_id, data.user_id);
        return true;
    });
}

bool EnhancedRepository::DeleteNote(int64_t note_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::USER_NOTE_DELETE, note_id, user_id);
        return true;
    });
}

std::vector<UserNoteEntity> EnhancedRepository::GetNotes(
    const std::string& user_id, const std::string& target_type, int64_t target_id) {
    return ExecuteQuery<std::vector<UserNoteEntity>>([&](pqxx::work& tx) {
        std::vector<UserNoteEntity> result;
        auto res = tx.exec_params(sql::USER_NOTE_GET, user_id, target_type, target_id);
        for (auto& row : res) {
            UserNoteEntity item;
            item.id = row[0].as<int64_t>();
            item.note_content = row[3].as<std::string>();
            item.color = row[4].as<std::string>();
            item.is_pinned = row[5].as<bool>();
            item.created_at = row[6].as<int64_t>();
            item.updated_at = row[7].as<int64_t>();
            result.push_back(item);
        }
        return result;
    });
}

bool EnhancedRepository::RecordContentEdit(const ContentHistoryEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::CONTENT_HISTORY_ADD,
            data.post_id > 0 ? data.post_id : nullptr,
            data.comment_id > 0 ? data.comment_id : nullptr,
            data.edited_by, data.old_title, data.old_content,
            data.new_title, data.new_content, data.edit_reason);
        return true;
    });
}

std::vector<ContentHistoryEntity> EnhancedRepository::GetContentHistory(
    int64_t post_id, int64_t comment_id, int page, int page_size) {
    return ExecuteQuery<std::vector<ContentHistoryEntity>>([&](pqxx::work& tx) {
        std::vector<ContentHistoryEntity> result;
        auto res = tx.exec_params(sql::CONTENT_HISTORY_GET,
            post_id > 0 ? post_id : nullptr,
            comment_id > 0 ? comment_id : nullptr,
            page_size, page * page_size);
        for (auto& row : res) {
            ContentHistoryEntity item;
            item.id = row[0].as<int64_t>();
            item.edited_by = row[1].as<std::string>();
            item.old_content = row[3].as<std::string>();
            item.new_content = row[5].as<std::string>();
            item.edit_reason = row[6].as<std::string>();
            item.edited_at = row[7].as<int64_t>();
            result.push_back(item);
        }
        return result;
    });
}

bool EnhancedRepository::AddQuickAccess(const QuickAccessEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::QUICKACCESS_ADD,
            data.user_id, data.item_type, data.item_id,
            data.item_name, data.item_icon, data.sort_order);
        return true;
    });
}

bool EnhancedRepository::RemoveQuickAccess(const std::string& user_id,
                                           const std::string& item_type, int64_t item_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::QUICKACCESS_REMOVE, user_id, item_type, item_id);
        return true;
    });
}

std::vector<QuickAccessEntity> EnhancedRepository::GetQuickAccess(const std::string& user_id) {
    return ExecuteQuery<std::vector<QuickAccessEntity>>([&](pqxx::work& tx) {
        std::vector<QuickAccessEntity> result;
        auto res = tx.exec_params(sql::QUICKACCESS_GET, user_id);
        for (auto& row : res) {
            QuickAccessEntity item;
            item.item_type = row[0].as<std::string>();
            item.item_id = row[1].as<int64_t>();
            item.item_name = row[2].as<std::string>();
            item.item_icon = row[3].as<std::string>();
            item.sort_order = row[4].as<int32_t>();
            result.push_back(item);
        }
        return result;
    });
}

std::vector<FilterWordEntity> EnhancedRepository::GetAllFilterWords() {
    return ExecuteQuery<std::vector<FilterWordEntity>>([&](pqxx::work& tx) {
        std::vector<FilterWordEntity> result;
        auto res = tx.exec(sql::WORDFILTER_GET_ALL);
        for (auto& row : res) {
            FilterWordEntity item;
            item.word = row[0].as<std::string>();
            item.replacement = row[1].as<std::string>();
            item.filter_level = row[2].as<int32_t>();
            item.is_regex = row[3].as<bool>();
            result.push_back(item);
        }
        return result;
    });
}

bool EnhancedRepository::AddFilterWord(const FilterWordEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::WORDFILTER_ADD,
            data.word, data.replacement, data.filter_level,
            data.is_regex, data.created_by);
        return true;
    });
}

bool EnhancedRepository::RemoveFilterWord(int64_t id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::WORDFILTER_REMOVE, id);
        return true;
    });
}

std::string EnhancedRepository::FilterContent(const std::string& content) {
    std::string result = content;
    auto filters = GetAllFilterWords();
    for (auto& filter : filters) {
        if (filter.is_regex) {
            try {
                std::regex pattern(filter.word, std::regex::icase);
                result = std::regex_replace(result, pattern, filter.replacement);
            } catch (...) {}
        } else {
            size_t pos = 0;
            while ((pos = result.find(filter.word, pos)) != std::string::npos) {
                result.replace(pos, filter.word.length(), filter.replacement);
                pos += filter.replacement.length();
            }
        }
    }
    return result;
}

int64_t EnhancedRepository::CreateSeries(const SeriesEntity& data) {
    return ExecuteQuery<int64_t>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::SERIES_CREATE,
            data.user_id, data.title, data.description,
            data.cover_image, data.is_public);
        return result[0][0].as<int64_t>();
    });
}

bool EnhancedRepository::UpdateSeries(int64_t series_id, const SeriesEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SERIES_UPDATE,
            data.title, data.description, data.cover_image,
            data.is_public, data.updated_at, series_id, data.user_id);
        return true;
    });
}

bool EnhancedRepository::DeleteSeries(int64_t series_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SERIES_DELETE, series_id, user_id);
        return true;
    });
}

std::vector<SeriesEntity> EnhancedRepository::GetUserSeries(
    const std::string& user_id, int page, int page_size) {
    return ExecuteQuery<std::vector<SeriesEntity>>([&](pqxx::work& tx) {
        std::vector<SeriesEntity> result;
        auto res = tx.exec_params(sql::SERIES_GET_BY_USER,
            user_id, page_size, page * page_size);
        for (auto& row : res) {
            SeriesEntity item;
            item.id = row[0].as<int64_t>();
            item.title = row[1].as<std::string>();
            item.description = row[2].as<std::string>();
            item.cover_image = row[3].as<std::string>();
            item.is_public = row[4].as<bool>();
            item.post_count = row[5].as<int32_t>();
            item.view_count = row[6].as<int32_t>();
            item.created_at = row[7].as<int64_t>();
            result.push_back(item);
        }
        return result;
    });
}

bool EnhancedRepository::AddPostToSeries(int64_t series_id, int64_t post_id, int32_t sort_order) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SERIES_POST_ADD, series_id, post_id, sort_order);
        return true;
    });
}

bool EnhancedRepository::RemovePostFromSeries(int64_t series_id, int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SERIES_POST_REMOVE, series_id, post_id);
        return true;
    });
}

std::vector<SeriesPostEntity> EnhancedRepository::GetSeriesPosts(int64_t series_id) {
    return ExecuteQuery<std::vector<SeriesPostEntity>>([&](pqxx::work& tx) {
        std::vector<SeriesPostEntity> result;
        auto res = tx.exec_params(sql::SERIES_POSTS_GET, series_id);
        for (auto& row : res) {
            SeriesPostEntity item;
            item.post_id = row[0].as<int64_t>();
            item.post_title = row[1].as<std::string>();
            item.sort_order = row[2].as<int32_t>();
            item.added_at = row[3].as<int64_t>();
            result.push_back(item);
        }
        return result;
    });
}

std::optional<UserPreferencesEntity> EnhancedRepository::GetUserPreferences(
    const std::string& user_id) {
    return ExecuteQuery<std::optional<UserPreferencesEntity>>([&](pqxx::work& tx) {
        auto result = tx.exec_params(sql::PREFERENCES_GET, user_id);
        if (result.empty()) {
            return std::nullopt;
        }
        UserPreferencesEntity prefs;
        prefs.user_id = user_id;
        prefs.content_language = result[0][0].as<std::string>();
        prefs.default_sort = result[0][1].as<std::string>();
        prefs.show_nsfw = result[0][2].as<bool>();
        prefs.blur_nsfw = result[0][3].as<bool>();
        prefs.auto_play_video = result[0][4].as<bool>();
        prefs.infinite_scroll = result[0][5].as<bool>();
        prefs.compact_mode = result[0][6].as<bool>();
        prefs.night_mode = result[0][7].as<std::string>();
        prefs.font_size = result[0][8].as<std::string>();
        prefs.notify_on_like = result[0][9].as<bool>();
        prefs.notify_on_comment = result[0][10].as<bool>();
        prefs.notify_on_follow = result[0][11].as<bool>();
        prefs.notify_on_mention = result[0][12].as<bool>();
        prefs.notify_on_message = result[0][13].as<bool>();
        prefs.email_digest = result[0][14].as<bool>();
        prefs.show_online_status = result[0][15].as<bool>();
        prefs.show_read_status = result[0][16].as<bool>();
        return prefs;
    });
}

bool EnhancedRepository::SetUserPreferences(const UserPreferencesEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::PREFERENCES_SET,
            data.user_id, data.content_language, data.default_sort,
            data.show_nsfw, data.blur_nsfw, data.auto_play_video,
            data.infinite_scroll, data.compact_mode, data.night_mode,
            data.font_size, data.notify_on_like, data.notify_on_comment,
            data.notify_on_follow, data.notify_on_mention,
            data.notify_on_message, data.email_digest,
            data.show_online_status, data.show_read_status,
            data.updated_at);
        return true;
    });
}

bool EnhancedRepository::UpdateContributorRanking(const ContributorRankingEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::RANKING_UPDATE_CONTRIBUTOR,
            data.user_id, data.contribution_score,
            data.posts_weight, data.comments_weight,
            data.likes_weight, data.uploads_weight,
            data.help_weight, data.last_updated);
        return true;
    });
}

std::vector<ContributorRankingEntity> EnhancedRepository::GetTopContributors(
    int limit, int offset) {
    return ExecuteQuery<std::vector<ContributorRankingEntity>>([&](pqxx::work& tx) {
        std::vector<ContributorRankingEntity> result;
        auto res = tx.exec_params(sql::RANKING_GET_TOP, limit, offset);
        for (auto& row : res) {
            ContributorRankingEntity item;
            item.user_id = row[0].as<std::string>();
            item.username = row[1].as<std::string>();
            item.avatar = row[2].as<std::string>();
            item.contribution_score = row[3].as<double>();
            item.posts_weight = row[4].as<int32_t>();
            item.comments_weight = row[5].as<int32_t>();
            item.likes_weight = row[6].as<int32_t>();
            result.push_back(item);
        }
        return result;
    });
}

} // namespace furbbs::repository
