#include "advanced_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

bool AdvancedRepository::AddModerator(const ModeratorEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::MOD_ADD,
            data.section_id, data.user_id, data.assigned_by,
            data.permission_level, data.can_manage_posts,
            data.can_manage_comments, data.can_manage_users,
            data.can_manage_reports);
        return true;
    });
}

bool AdvancedRepository::RemoveModerator(int64_t section_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MOD_REMOVE, section_id, user_id);
        return r.affected_rows() > 0;
    });
}

std::vector<ModeratorEntity> AdvancedRepository::GetUserModeratorRoles(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MOD_GET_BY_USER, user_id);
        return MapResults<ModeratorEntity>(r, [](const pqxx::row& row, ModeratorEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.section_id = row["section_id"].as<int64_t>();
            e.permission_level = row["permission_level"].as<std::string>();
            e.can_manage_posts = row["can_manage_posts"].as<bool>();
            e.can_manage_comments = row["can_manage_comments"].as<bool>();
            e.can_manage_users = row["can_manage_users"].as<bool>();
            e.can_manage_reports = row["can_manage_reports"].as<bool>();
            e.assigned_at = row["assigned_at"].as<int64_t>();
        });
    });
}

bool AdvancedRepository::CheckModeratorPermission(const std::string& user_id,
        int64_t section_id, const std::string& permission) {
    return ExecuteQueryOne<bool>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MOD_CHECK_PERM, user_id, section_id);
        return !r.empty();
    }).value_or(false);
}

bool AdvancedRepository::CreatePunishment(const PunishmentEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::PUNISH_CREATE,
            data.user_id, data.punishment_type, data.reason,
            data.duration, data.points_deducted,
            data.executed_by, data.expires_at);
        tx.exec_params(sql::PUNISH_HISTORY,
            data.user_id, data.punishment_type, data.reason,
            data.points_deducted, data.executed_by);
        return true;
    });
}

std::vector<PunishmentEntity> AdvancedRepository::GetActivePunishments(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        tx.exec_params(sql::PUNISH_EXPIRE);
        pqxx::result r = tx.exec_params(sql::PUNISH_GET_ACTIVE, user_id);
        return MapResults<PunishmentEntity>(r, [](const pqxx::row& row, PunishmentEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.punishment_type = row["punishment_type"].as<std::string>();
            e.reason = row["reason"].as<std::string>();
            e.duration = row["duration"].as<int64_t>();
            e.points_deducted = row["points_deducted"].as<int32_t>();
            e.executed_by = row["executed_by"].as<std::string>();
            e.executed_at = row["executed_at"].as<int64_t>();
            e.expires_at = row["expires_at"].as<int64_t>();
        });
    });
}

bool AdvancedRepository::ExpireOldPunishments() {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::PUNISH_EXPIRE);
        return true;
    });
}

void AdvancedRepository::RecordPunishmentHistory(const PunishmentEntity& data) {
    ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::PUNISH_HISTORY,
            data.user_id, data.punishment_type, data.reason,
            data.points_deducted, data.executed_by);
        return true;
    });
}

bool AdvancedRepository::CreatePoll(const PollEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::POLL_CREATE,
            data.post_id, data.question, data.options,
            data.is_multiple, data.end_at);
        return true;
    });
}

std::optional<PollEntity> AdvancedRepository::GetPoll(int64_t post_id, const std::string& voter_id) {
    return ExecuteQueryOne<PollEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::POLL_GET, post_id);
        if (r.empty()) return PollEntity{};
        PollEntity e;
        e.post_id = post_id;
        e.question = r[0]["question"].as<std::string>();
        e.options = r[0]["options"].as<std::vector<std::string>>();
        e.vote_counts = r[0]["vote_counts"].as<std::vector<int32_t>>();
        e.is_multiple = r[0]["is_multiple"].as<bool>();
        e.end_at = r[0]["end_at"].as<int64_t>();
        e.created_at = r[0]["created_at"].as<int64_t>();
        if (!voter_id.empty()) {
            pqxx::result rv = tx.exec_params(sql::POLL_CHECK_VOTE, post_id, voter_id);
            e.has_voted = !rv.empty();
        }
        return e;
    });
}

bool AdvancedRepository::VotePoll(int64_t poll_id, const std::string& user_id, int option_index) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::POLL_VOTE,
            poll_id, user_id, option_index, option_index + 1);
        return true;
    });
}

bool AdvancedRepository::HasUserVoted(int64_t poll_id, const std::string& user_id) {
    return ExecuteQueryOne<bool>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::POLL_CHECK_VOTE, poll_id, user_id);
        return !r.empty();
    }).value_or(false);
}

bool AdvancedRepository::UpdateHotScore(const HotScoreEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::HOT_UPDATE_SCORE,
            data.post_id, data.hot_score, data.view_weight,
            data.like_weight, data.comment_weight);
        return true;
    });
}

std::vector<int64_t> AdvancedRepository::GetTopHotPosts(int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::HOT_GET_TOP, limit, offset);
        std::vector<int64_t> result;
        for (auto& row : r) {
            result.push_back(row["post_id"].as<int64_t>());
        }
        return result;
    });
}

std::optional<WatermarkEntity> AdvancedRepository::GetUserWatermark(const std::string& user_id) {
    return ExecuteQueryOne<WatermarkEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WATERMARK_GET, user_id);
        if (r.empty()) return WatermarkEntity{};
        WatermarkEntity e;
        e.id = r[0]["id"].as<int64_t>();
        e.watermark_text = r[0]["watermark_text"].as<std::string>();
        e.watermark_position = r[0]["watermark_position"].as<std::string>();
        e.opacity = r[0]["opacity"].as<double>();
        e.is_enabled = r[0]["is_enabled"].as<bool>();
        return e;
    });
}

bool AdvancedRepository::SetUserWatermark(const WatermarkEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::WATERMARK_SET,
            data.user_id, data.watermark_text,
            data.watermark_position, data.opacity, data.is_enabled);
        return true;
    });
}

std::optional<FeedSettingsEntity> AdvancedRepository::GetFeedSettings(const std::string& user_id) {
    return ExecuteQueryOne<FeedSettingsEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::FEED_SETTINGS_GET, user_id);
        if (r.empty()) return FeedSettingsEntity{};
        FeedSettingsEntity e;
        e.user_id = user_id;
        e.feed_type = r[0]["feed_type"].as<std::string>();
        return e;
    });
}

bool AdvancedRepository::SetFeedSettings(const FeedSettingsEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::FEED_SETTINGS_SET,
            data.user_id, data.feed_type,
            data.include_sections, data.exclude_tags);
        return true;
    });
}

void AdvancedRepository::LogRecommendation(const std::string& user_id, int64_t post_id,
        const std::string& algorithm, const std::string& action, double score) {
    ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::RECOMMEND_LOG,
            user_id, post_id, algorithm, action, score);
        return true;
    });
}

} // namespace furbbs::repository
