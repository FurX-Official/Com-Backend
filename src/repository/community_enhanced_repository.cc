#include "community_enhanced_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

int64_t CommunityEnhancedRepository::CreateReport(const ContentReportEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::REPORT_CREATE,
            data.reporter_id, data.content_type, data.content_id,
            data.report_reason, data.report_details);
        return r.empty() ? 0 : r[0][0].as<int64_t>();
    });
}

std::vector<ContentReportEntity> CommunityEnhancedRepository::GetPendingReports(
        int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::REPORT_GET_PENDING, limit, offset);
        return MapResults<ContentReportEntity>(r, [](const pqxx::row& row, ContentReportEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.reporter_id = row["reporter_id"].as<std::string>();
            e.content_type = row["content_type"].as<std::string>();
            e.content_id = row["content_id"].as<int64_t>();
            e.report_reason = row["report_reason"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool CommunityEnhancedRepository::HandleReport(int64_t report_id, const std::string& status,
        const std::string& handled_by, const std::string& notes) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::REPORT_HANDLE,
            status, handled_by, notes, report_id);
        return r.affected_rows() > 0;
    });
}

int64_t CommunityEnhancedRepository::CreateReply(const CommentReplyEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::REPLY_CREATE,
            data.comment_id, data.parent_reply_id, data.post_id,
            data.user_id, data.reply_to_user_id, data.content);
        return r[0][0].as<int64_t>();
    });
}

std::vector<CommentReplyEntity> CommunityEnhancedRepository::GetCommentReplies(int64_t comment_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::REPLY_GET_BY_COMMENT, comment_id);
        return MapResults<CommentReplyEntity>(r, [](const pqxx::row& row, CommentReplyEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.user_id = row["user_id"].as<std::string>();
            e.reply_to_user_id = row["reply_to_user_id"].as<std::string>();
            e.content = row["content"].as<std::string>();
            e.like_count = row["like_count"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool CommunityEnhancedRepository::DeleteReply(int64_t reply_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::REPLY_DELETE, reply_id, user_id);
        return r.affected_rows() > 0;
    });
}

bool CommunityEnhancedRepository::LikeReply(int64_t reply_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::REPLY_LIKE, reply_id);
        return true;
    });
}

bool CommunityEnhancedRepository::SetSticky(int64_t post_id, int64_t section_id,
        int priority, const std::string& sticky_by) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::STICKY_SET, post_id, section_id, priority, sticky_by);
        return true;
    });
}

bool CommunityEnhancedRepository::RemoveSticky(int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::STICKY_REMOVE, post_id);
        return r.affected_rows() > 0;
    });
}

std::vector<int64_t> CommunityEnhancedRepository::GetSectionStickyPosts(int64_t section_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::STICKY_GET_BY_SECTION, section_id);
        std::vector<int64_t> result;
        for (auto& row : r) {
            result.push_back(row["id"].as<int64_t>());
        }
        return result;
    });
}

bool CommunityEnhancedRepository::SetDigest(int64_t post_id, const std::string& level,
        const std::string& recommended_by, const std::string& description) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::DIGEST_SET, post_id, level, recommended_by, description);
        return true;
    });
}

bool CommunityEnhancedRepository::RemoveDigest(int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::DIGEST_REMOVE, post_id);
        return r.affected_rows() > 0;
    });
}

std::vector<int64_t> CommunityEnhancedRepository::GetDigestPosts(
        const std::string& level, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::DIGEST_GET_BY_LEVEL, level, limit, offset);
        std::vector<int64_t> result;
        for (auto& row : r) {
            result.push_back(row["id"].as<int64_t>());
        }
        return result;
    });
}

int64_t CommunityEnhancedRepository::CreateFolder(const CollectionFolderEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::FOLDER_CREATE,
            data.user_id, data.name, data.description, data.is_public);
        return r[0][0].as<int64_t>();
    });
}

std::vector<CollectionFolderEntity> CommunityEnhancedRepository::GetUserFolders(
        const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::FOLDER_GET_BY_USER, user_id);
        return MapResults<CollectionFolderEntity>(r, [](const pqxx::row& row, CollectionFolderEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.name = row["name"].as<std::string>();
            e.description = row["description"].as<std::string>();
            e.is_public = row["is_public"].as<bool>();
            e.item_count = row["item_count"].as<int32_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool CommunityEnhancedRepository::AddToFolder(int64_t folder_id, int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::FOLDER_ADD_ITEM, folder_id, post_id);
        return true;
    });
}

bool CommunityEnhancedRepository::RemoveFromFolder(int64_t folder_id, int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::FOLDER_REMOVE_ITEM, folder_id, post_id);
        return true;
    });
}

std::vector<int64_t> CommunityEnhancedRepository::GetFolderItems(
        int64_t folder_id, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::FOLDER_GET_ITEMS, folder_id, limit, offset);
        std::vector<int64_t> result;
        for (auto& row : r) {
            result.push_back(row["id"].as<int64_t>());
        }
        return result;
    });
}

bool CommunityEnhancedRepository::AddUserTag(const std::string& tagger_id,
        const std::string& tagged_id, const std::string& tag) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::USER_TAG_ADD, tagger_id, tagged_id, tag);
        return true;
    });
}

bool CommunityEnhancedRepository::RemoveUserTag(const std::string& tagger_id,
        const std::string& tagged_id, const std::string& tag) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::USER_TAG_REMOVE, tagger_id, tagged_id, tag);
        return r.affected_rows() > 0;
    });
}

std::vector<std::string> CommunityEnhancedRepository::GetUserTags(
        const std::string& tagger_id, const std::string& tagged_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::USER_TAG_GET, tagger_id, tagged_id);
        std::vector<std::string> result;
        for (auto& row : r) {
            result.push_back(row["tag"].as<std::string>());
        }
        return result;
    });
}

bool CommunityEnhancedRepository::AddKeywordFilter(const UserKeywordFilterEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::KEYWORD_FILTER_ADD, data.user_id, data.keyword, data.filter_type);
        return true;
    });
}

bool CommunityEnhancedRepository::RemoveKeywordFilter(int64_t filter_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::KEYWORD_FILTER_REMOVE, user_id, filter_id);
        return r.affected_rows() > 0;
    });
}

std::vector<UserKeywordFilterEntity> CommunityEnhancedRepository::GetUserFilters(
        const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::KEYWORD_FILTER_GET, user_id);
        return MapResults<UserKeywordFilterEntity>(r, [](const pqxx::row& row, UserKeywordFilterEntity& e) {
            e.keyword = row["keyword"].as<std::string>();
            e.filter_type = row["filter_type"].as<std::string>();
        });
    });
}

int64_t CommunityEnhancedRepository::SaveDraft(const PostDraftEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::DRAFT_SAVE,
            data.user_id, data.title, data.content, data.section_id,
            data.tags, data.fursona_id, data.is_auto_save);
        return r[0][0].as<int64_t>();
    });
}

std::vector<PostDraftEntity> CommunityEnhancedRepository::GetUserDrafts(
        const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::DRAFT_GET_BY_USER, user_id);
        return MapResults<PostDraftEntity>(r, [](const pqxx::row& row, PostDraftEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.title = row["title"].as<std::string>();
            e.content = row["content"].as<std::string>();
            e.section_id = row["section_id"].as<int64_t>();
            e.updated_at = row["updated_at"].as<int64_t>();
        });
    });
}

bool CommunityEnhancedRepository::DeleteDraft(int64_t draft_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::DRAFT_DELETE, draft_id, user_id);
        return r.affected_rows() > 0;
    });
}

bool CommunityEnhancedRepository::IncrementViewCount(int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::STATS_INC_VIEW, post_id);
        return true;
    });
}

bool CommunityEnhancedRepository::IncrementShareCount(int64_t post_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::STATS_INC_SHARE, post_id);
        return true;
    });
}

std::optional<PostShareStatsEntity> CommunityEnhancedRepository::GetPostStats(int64_t post_id) {
    return ExecuteQueryOne<PostShareStatsEntity>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::STATS_GET, post_id);
        if (r.empty()) return PostShareStatsEntity{};
        PostShareStatsEntity e;
        e.post_id = post_id;
        e.view_count = r[0]["view_count"].as<int32_t>();
        e.share_count = r[0]["share_count"].as<int32_t>();
        e.download_count = r[0]["download_count"].as<int32_t>();
        return e;
    });
}

} // namespace furbbs::repository
