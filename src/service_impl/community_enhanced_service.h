#ifndef FURBBS_SERVICE_IMPL_COMMUNITY_ENHANCED_SERVICE_H
#define FURBBS_SERVICE_IMPL_COMMUNITY_ENHANCED_SERVICE_H

#include "../repository/community_enhanced_repository.h"
#include "../repository/security_repository.h"
#include "core_service.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::service {

class CommunityEnhancedService {
public:
    static CommunityEnhancedService& Instance() {
        static CommunityEnhancedService instance;
        return instance;
    }

    int64_t CreateReport(const repository::ContentReportEntity& data) {
        auto id = repository::CommunityEnhancedRepository::Instance().CreateReport(data);
        if (id > 0) {
            CoreService::Instance().LogAction(
                data.reporter_id, "report_create", data.content_type, data.content_id);
        }
        return id;
    }

    std::vector<repository::ContentReportEntity> GetPendingReports(
            int page, int page_size) {
        return repository::CommunityEnhancedRepository::Instance().GetPendingReports(
            page_size, (page - 1) * page_size);
    }

    bool HandleReport(int64_t report_id, const std::string& status,
                      const std::string& handled_by, const std::string& notes) {
        bool success = repository::CommunityEnhancedRepository::Instance().HandleReport(
            report_id, status, handled_by, notes);
        if (success) {
            CoreService::Instance().LogAction(
                handled_by, "report_handle", "report", report_id);
        }
        return success;
    }

    int64_t CreateReply(const repository::CommentReplyEntity& data) {
        if (service::SecurityService::Instance().IsUserBlocked(
                data.user_id, data.reply_to_user_id)) {
            return 0;
        }
        return repository::CommunityEnhancedRepository::Instance().CreateReply(data);
    }

    std::vector<repository::CommentReplyEntity> GetCommentReplies(
            int64_t comment_id, const std::string& viewer_id = "") {
        auto replies = repository::CommunityEnhancedRepository::Instance().GetCommentReplies(comment_id);
        std::vector<repository::CommentReplyEntity> filtered;
        for (auto& r : replies) {
            if (!viewer_id.empty() && service::SecurityService::Instance().IsUserBlocked(
                    viewer_id, r.user_id)) {
                continue;
            }
            filtered.push_back(r);
        }
        return filtered;
    }

    bool DeleteReply(int64_t reply_id, const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().DeleteReply(reply_id, user_id);
    }

    bool LikeReply(int64_t reply_id) {
        return repository::CommunityEnhancedRepository::Instance().LikeReply(reply_id);
    }

    bool SetSticky(int64_t post_id, int64_t section_id, int priority,
                   const std::string& sticky_by) {
        bool success = repository::CommunityEnhancedRepository::Instance().SetSticky(
            post_id, section_id, priority, sticky_by);
        if (success) {
            CoreService::Instance().LogAction(sticky_by, "post_sticky", "post", post_id);
        }
        return success;
    }

    bool RemoveSticky(int64_t post_id, const std::string& operator_id) {
        bool success = repository::CommunityEnhancedRepository::Instance().RemoveSticky(post_id);
        if (success) {
            CoreService::Instance().LogAction(operator_id, "post_unsticky", "post", post_id);
        }
        return success;
    }

    bool SetDigest(int64_t post_id, const std::string& level,
                   const std::string& recommended_by, const std::string& description) {
        bool success = repository::CommunityEnhancedRepository::Instance().SetDigest(
            post_id, level, recommended_by, description);
        if (success) {
            CoreService::Instance().LogAction(
                recommended_by, "post_digest", "post", post_id);
        }
        return success;
    }

    int64_t CreateFolder(const repository::CollectionFolderEntity& data) {
        return repository::CommunityEnhancedRepository::Instance().CreateFolder(data);
    }

    std::vector<repository::CollectionFolderEntity> GetUserFolders(
            const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().GetUserFolders(user_id);
    }

    bool AddToFolder(int64_t folder_id, int64_t post_id, const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().AddToFolder(folder_id, post_id);
    }

    bool RemoveFromFolder(int64_t folder_id, int64_t post_id, const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().RemoveFromFolder(folder_id, post_id);
    }

    std::vector<int64_t> GetFolderItems(int64_t folder_id, int page, int page_size) {
        return repository::CommunityEnhancedRepository::Instance().GetFolderItems(
            folder_id, page_size, (page - 1) * page_size);
    }

    bool AddUserTag(const std::string& tagger_id, const std::string& tagged_id,
                    const std::string& tag) {
        return repository::CommunityEnhancedRepository::Instance().AddUserTag(
            tagger_id, tagged_id, tag);
    }

    bool RemoveUserTag(const std::string& tagger_id, const std::string& tagged_id,
                       const std::string& tag) {
        return repository::CommunityEnhancedRepository::Instance().RemoveUserTag(
            tagger_id, tagged_id, tag);
    }

    std::vector<std::string> GetUserTags(const std::string& tagger_id,
                                         const std::string& tagged_id) {
        return repository::CommunityEnhancedRepository::Instance().GetUserTags(tagger_id, tagged_id);
    }

    bool AddKeywordFilter(const repository::UserKeywordFilterEntity& data) {
        return repository::CommunityEnhancedRepository::Instance().AddKeywordFilter(data);
    }

    bool RemoveKeywordFilter(int64_t filter_id, const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().RemoveKeywordFilter(filter_id, user_id);
    }

    std::vector<repository::UserKeywordFilterEntity> GetUserFilters(
            const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().GetUserFilters(user_id);
    }

    int64_t SaveDraft(const repository::PostDraftEntity& data) {
        return repository::CommunityEnhancedRepository::Instance().SaveDraft(data);
    }

    std::vector<repository::PostDraftEntity> GetUserDrafts(const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().GetUserDrafts(user_id);
    }

    bool DeleteDraft(int64_t draft_id, const std::string& user_id) {
        return repository::CommunityEnhancedRepository::Instance().DeleteDraft(draft_id, user_id);
    }

    void RecordPostView(int64_t post_id, const std::string& viewer_id = "") {
        repository::CommunityEnhancedRepository::Instance().IncrementViewCount(post_id);
    }

    void RecordPostShare(int64_t post_id) {
        repository::CommunityEnhancedRepository::Instance().IncrementShareCount(post_id);
    }

    std::optional<repository::PostShareStatsEntity> GetPostStats(int64_t post_id) {
        return repository::CommunityEnhancedRepository::Instance().GetPostStats(post_id);
    }

    template<typename T>
    void ApplyKeywordFilters(const std::string& user_id, std::vector<T>& items,
                             std::function<std::string(const T&)> get_text) {
        auto filters = GetUserFilters(user_id);
        std::vector<T> result;
        for (auto& item : items) {
            bool filtered = false;
            std::string text = get_text(item);
            for (auto& f : filters) {
                if (text.find(f.keyword) != std::string::npos) {
                    filtered = true;
                    break;
                }
            }
            if (!filtered) {
                result.push_back(item);
            }
        }
        items.swap(result);
    }

private:
    CommunityEnhancedService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_COMMUNITY_ENHANCED_SERVICE_H
