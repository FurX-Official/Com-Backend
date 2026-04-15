#include "advanced_service.h"
#include "repository/post_repository.h"

namespace furbbs::service {

int64_t AdvancedService::CreateGroup(const std::string& token, const std::string& name,
                                      const std::string& description, bool is_public,
                                      bool allow_join_request, 
                                      const std::vector<std::string>& tags) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || name.empty()) {
        return 0;
    }

    return repository::AdvancedRepository::Instance().CreateGroup(
        user_opt->id, name, description, is_public, allow_join_request, tags);
}

std::vector<repository::GroupEntity> AdvancedService::GetGroups(
    const std::string& token, const std::string& user_id,
    const std::string& tag, const std::string& keyword,
    int page, int page_size, int& out_total) {

    std::string viewer_id;
    if (!token.empty()) {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user_opt) viewer_id = user_opt->id;
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::AdvancedRepository::Instance().GetGroupCount(
        user_id, tag, keyword);

    return repository::AdvancedRepository::Instance().GetGroups(
        user_id, viewer_id, tag, keyword, page_size, offset);
}

std::optional<repository::GroupEntity> AdvancedService::GetGroupDetail(
    const std::string& token, int64_t group_id) {

    std::string viewer_id;
    if (!token.empty()) {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user_opt) viewer_id = user_opt->id;
    }

    return repository::AdvancedRepository::Instance().GetGroup(group_id, viewer_id);
}

bool AdvancedService::ManageGroupMember(const std::string& token, int64_t group_id,
                                       const std::string& target_user_id, int action, 
                                       int new_role) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    return repository::AdvancedRepository::Instance().ManageGroupMember(
        group_id, user_opt->id, target_user_id, action, new_role);
}

int64_t AdvancedService::CreateGroupPost(const std::string& token, int64_t group_id,
                                        const std::string& title, 
                                        const std::string& content,
                                        const std::vector<int64_t>& tag_ids) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || title.empty()) {
        return 0;
    }

    int64_t post_id = repository::AdvancedRepository::Instance().CreateGroupPost(
        group_id, user_opt->id, title, content, tag_ids);

    if (post_id > 0) {
        ProcessMentions(user_opt->id, content, post_id, 0);
    }

    return post_id;
}

int64_t AdvancedService::RegisterEvent(const std::string& token, int64_t event_id,
                                      int32_t ticket_type, int32_t guest_count,
                                      const std::string& contact) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return 0;
    }

    return repository::AdvancedRepository::Instance().RegisterEvent(
        event_id, user_opt->id, ticket_type, guest_count, contact);
}

std::vector<repository::EventRegEntity> AdvancedService::GetEventRegistrations(
    const std::string& token, int64_t event_id,
    int page, int page_size, int& out_total, int& out_confirmed) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        out_total = 0;
        out_confirmed = 0;
        return {};
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::AdvancedRepository::Instance().GetRegistrationCount(event_id);
    out_confirmed = repository::AdvancedRepository::Instance().GetConfirmedCount(event_id);

    return repository::AdvancedRepository::Instance().GetEventRegistrations(
        event_id, user_opt->id, page_size, offset);
}

bool AdvancedService::UpdateRegistrationStatus(const std::string& token, int64_t reg_id, 
                                          int32_t new_status) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    return repository::AdvancedRepository::Instance().UpdateRegistrationStatus(
        reg_id, user_opt->id, new_status);
}

std::vector<repository::MentionEntity> AdvancedService::GetMentions(
    const std::string& token, bool only_unread,
    int page, int page_size, int& out_total, int& out_unread) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        out_total = 0;
        out_unread = 0;
        return {};
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::AdvancedRepository::Instance().GetMentionCount(
        user_opt->id, only_unread);
    out_unread = repository::AdvancedRepository::Instance().GetUnreadMentionCount(
        user_opt->id);

    return repository::AdvancedRepository::Instance().GetUserMentions(
        user_opt->id, only_unread, page_size, offset);
}

void AdvancedService::MarkMentionRead(const std::string& token, int64_t mention_id, 
                                   bool mark_all) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return;
    }

    repository::AdvancedRepository::Instance().MarkMentionRead(
        user_opt->id, mention_id, mark_all);
}

bool AdvancedService::FavoritePost(const std::string& token, int64_t post_id, 
                                bool favorite) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    return repository::AdvancedRepository::Instance().SetPostFavorite(
        user_opt->id, post_id, favorite);
}

std::vector<PostEntity> AdvancedService::GetFavoritePosts(
    const std::string& token, int page, int page_size, int& out_total) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        out_total = 0;
        return {};
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    auto post_ids = repository::AdvancedRepository::Instance().GetFavoritePostIds(
        user_opt->id, page_size, offset);
    out_total = repository::AdvancedRepository::Instance().GetFavoritePostCount(
        user_opt->id);

    std::vector<PostEntity> posts;
    for (int64_t id : post_ids) {
        auto post = repository::PostRepository::Instance().GetPost(id);
        if (post) posts.push_back(*post);
    }
    return posts;
}

int64_t AdvancedService::SaveDraft(const std::string& token, int64_t draft_id,
                                  const std::string& title, const std::string& content,
                                  int32_t section_id, const std::vector<int64_t>& tag_ids,
                                  int64_t group_id) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return 0;
    }

    return repository::AdvancedRepository::Instance().SaveDraft(
        user_opt->id, draft_id, title, content, section_id, tag_ids, group_id);
}

std::vector<repository::DraftEntity> AdvancedService::GetDrafts(
    const std::string& token, int page, int page_size, int& out_total) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        out_total = 0;
        return {};
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::AdvancedRepository::Instance().GetDraftCount(user_opt->id);
    return repository::AdvancedRepository::Instance().GetUserDrafts(
        user_opt->id, page_size, offset);
}

bool AdvancedService::DeleteDraft(const std::string& token, int64_t draft_id) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }

    return repository::AdvancedRepository::Instance().DeleteDraft(user_opt->id, draft_id);
}

void AdvancedService::ProcessMentions(const std::string& user_id, 
                                    const std::string& content,
                                    int64_t post_id, int64_t comment_id) {

    auto mentioned = repository::AdvancedRepository::Instance().ExtractMentions(content);
    if (mentioned.empty()) return;

    std::string preview = content.substr(0, std::min((size_t)200, content.size()));
    repository::AdvancedRepository::Instance().CreateMention(
        user_id, mentioned, post_id, comment_id, preview);
}

} // namespace furbbs::service
