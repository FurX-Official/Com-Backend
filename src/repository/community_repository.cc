#include "community_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

int64_t CommunityRepository::SaveCard(const std::string& user_id, const FursonaCardEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CARD_CREATE,
            data.fursona_id, user_id, data.template_id, data.theme_color,
            data.background_image, data.show_stats, data.show_artist, data.card_layout);
        return r[0][0].as<int64_t>();
    });
}

std::optional<FursonaCardEntity> CommunityRepository::GetCard(int64_t fursona_id) {
    return ExecuteQueryOne<FursonaCardEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::CARD_GET_BY_FURSONA, fursona_id);
        if (r.empty()) return std::optional<FursonaCardEntity>();
        FursonaCardEntity e;
        MapRow(r[0], e, [](const pqxx::row& row, FursonaCardEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.fursona_id = row["fursona_id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.fursona_name = row["fursona_name"].as<std::string>();
            e.species = row["species"].as<std::string>();
            e.gender = row["gender"].as<std::string>();
            e.template_id = row["template_id"].as<std::string>();
            e.theme_color = row["theme_color"].as<std::string>();
            e.background_image = row["background_image"].as<std::string>();
            e.show_stats = row["show_stats"].as<bool>();
            e.show_artist = row["show_artist"].as<bool>();
            e.card_layout = row["card_layout"].as<std::string>();
            e.view_count = row["view_count"].as<int32_t>();
        });
        return e;
    });
}

bool CommunityRepository::IncrementCardView(int64_t fursona_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::CARD_INC_VIEW, fursona_id);
        return true;
    });
}

bool CommunityRepository::SetContentRating(const ContentRatingEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::RATING_SET,
            data.content_type, data.content_id, data.user_id,
            data.rating_level, data.content_warnings, data.rated_by);
        return true;
    });
}

std::optional<ContentRatingEntity> CommunityRepository::GetContentRating(
        const std::string& content_type, int64_t content_id) {
    return ExecuteQueryOne<ContentRatingEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::RATING_GET, content_type, content_id);
        if (r.empty()) return std::optional<ContentRatingEntity>();
        ContentRatingEntity e;
        MapRow(r[0], e, [](const pqxx::row& row, ContentRatingEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.content_type = row["content_type"].as<std::string>();
            e.content_id = row["content_id"].as<int64_t>();
            e.rating_level = row["rating_level"].as<std::string>();
            e.is_age_verified = row["is_age_verified"].as<bool>();
            e.rated_by = row["rated_by"].as<std::string>();
            e.rated_at = row["rated_at"].as<int64_t>();
        });
        return e;
    });
}

bool CommunityRepository::UpdateContentPrefs(const ContentPrefsEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::PREFS_UPDATE,
            data.user_id, data.show_safe, data.show_questionable,
            data.show_explicit, data.enabled_warnings,
            data.blur_sensitive, data.age_verified);
        return true;
    });
}

std::optional<ContentPrefsEntity> CommunityRepository::GetContentPrefs(const std::string& user_id) {
    return ExecuteQueryOne<ContentPrefsEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::PREFS_GET, user_id);
        if (r.empty()) return std::optional<ContentPrefsEntity>();
        ContentPrefsEntity e;
        MapRow(r[0], e, [](const pqxx::row& row, ContentPrefsEntity& e) {
            e.user_id = row["user_id"].as<std::string>();
            e.show_safe = row["show_safe"].as<bool>();
            e.show_questionable = row["show_questionable"].as<bool>();
            e.show_explicit = row["show_explicit"].as<bool>();
            e.blur_sensitive = row["blur_sensitive"].as<bool>();
            e.age_verified = row["age_verified"].as<bool>();
        });
        return e;
    });
}

int64_t CommunityRepository::RequestPermission(const CreationPermissionEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::PERMISSION_REQUEST,
            data.author_user_id, data.authorized_user_id,
            data.fursona_id, data.permission_type, data.terms);
        return r[0][0].as<int64_t>();
    });
}

bool CommunityRepository::ApprovePermission(const std::string& author_id, int64_t permission_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::PERMISSION_APPROVE, permission_id, author_id);
        return r.affected_rows() > 0;
    });
}

std::vector<CreationPermissionEntity> CommunityRepository::GetUserPermissions(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::PERMISSION_GET_BY_USER, user_id);
        return MapResults<CreationPermissionEntity>(r, [](const pqxx::row& row, CreationPermissionEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.author_user_id = row["author_user_id"].as<std::string>();
            e.authorized_user_id = row["authorized_user_id"].as<std::string>();
            e.fursona_id = row["fursona_id"].as<int64_t>();
            e.fursona_name = row["fursona_name"].as<std::string>();
            e.permission_type = row["permission_type"].as<std::string>();
            e.terms = row["terms"].as<std::string>();
            e.is_approved = row["is_approved"].as<bool>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool CommunityRepository::AddInteraction(const FursonaInteractionEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::INTERACTION_ADD,
            data.from_fursona_id, data.to_fursona_id,
            data.interaction_type, data.user_note, data.intimacy_score);
        return true;
    });
}

std::vector<FursonaInteractionEntity> CommunityRepository::GetFursonaInteractions(int64_t fursona_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::INTERACTION_GET, fursona_id);
        return MapResults<FursonaInteractionEntity>(r, [](const pqxx::row& row, FursonaInteractionEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.from_fursona_id = row["from_fursona_id"].as<int64_t>();
            e.to_fursona_id = row["to_fursona_id"].as<int64_t>();
            e.fursona_name = row["fursona_name"].as<std::string>();
            e.interaction_type = row["interaction_type"].as<std::string>();
            e.user_note = row["user_note"].as<std::string>();
            e.intimacy_score = row["intimacy_score"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool CommunityRepository::SubmitToModeration(const ModerationItemEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::MOD_SUBMIT,
            data.content_type, data.content_id, data.submitter_id);
        return true;
    });
}

bool CommunityRepository::ReviewModeration(const ModerationItemEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MOD_REVIEW,
            data.status, data.moderator_id, data.moderator_note,
            data.violation_type, data.id);
        return r.affected_rows() > 0;
    });
}

std::vector<ModerationItemEntity> CommunityRepository::GetModerationQueue(
        const std::string& status, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::MOD_GET_QUEUE, status, limit, offset);
        return MapResults<ModerationItemEntity>(r, [](const pqxx::row& row, ModerationItemEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.content_type = row["content_type"].as<std::string>();
            e.content_id = row["content_id"].as<int64_t>();
            e.submitter_id = row["submitter_id"].as<std::string>();
            e.status = row["status"].as<std::string>();
            e.moderator_id = row["moderator_id"].as<std::string>();
            e.moderator_note = row["moderator_note"].as<std::string>();
            e.violation_type = row["violation_type"].as<std::string>();
            e.submitted_at = row["submitted_at"].as<int64_t>();
            e.reviewed_at = row["reviewed_at"].as<int64_t>();
        });
    });
}

} // namespace furbbs::repository
