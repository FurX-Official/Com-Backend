#include "advanced_service.h"
#include "repository/user_repository.h"
#include "repository/post_repository.h"
#include <chrono>
#include <cmath>

namespace furbbs::service {

void AdvancedService::InitializePunishmentRules() {
    punishment_rules_["spam"] = {
        {"spam", 1, "mute", 24 * 60 * 60 * 1000LL, 20},
        {"spam", 3, "ban", 7 * 24 * 60 * 60 * 1000LL, 50},
        {"spam", 5, "ban", 30 * 24 * 60 * 60 * 1000LL, 100}
    };
    punishment_rules_["harassment"] = {
        {"harassment", 1, "mute", 3 * 24 * 60 * 60 * 1000LL, 30},
        {"harassment", 2, "ban", 14 * 24 * 60 * 60 * 1000LL, 80}
    };
    punishment_rules_["nsfw_violation"] = {
        {"nsfw_violation", 1, "mute", 7 * 24 * 60 * 60 * 1000LL, 40},
        {"nsfw_violation", 2, "ban", 30 * 24 * 60 * 60 * 1000LL, 100}
    };
    punishment_rules_["troll"] = {
        {"troll", 1, "warn", 0, 10},
        {"troll", 2, "mute", 24 * 60 * 60 * 1000LL, 25},
        {"troll", 4, "ban", 7 * 24 * 60 * 60 * 1000LL, 60}
    };
    punishment_rules_["copyright"] = {
        {"copyright", 1, "warn", 0, 15},
        {"copyright", 2, "mute", 48 * 60 * 60 * 1000LL, 35}
    };
}

std::string AdvancedService::ValidateAndGetUserId(const std::string& token) {
    auto auth_result = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!auth_result) {
        return "";
    }
    return auth_result->user_id;
}

bool AdvancedService::IsAdmin(const std::string& user_id) {
    auto user = repository::UserRepository::Instance().GetUserInfo(user_id);
    if (!user) {
        return false;
    }
    return user->role == "admin" || user->role == "super_admin";
}

bool AdvancedService::AddModerator(const std::string& token, int64_t section_id,
                                   const std::string& target_user_id,
                                   const std::string& permission_level,
                                   bool can_manage_posts, bool can_manage_comments,
                                   bool can_manage_users, bool can_manage_reports) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    if (!IsAdmin(user_id)) {
        return false;
    }
    repository::ModeratorEntity mod;
    mod.section_id = section_id;
    mod.user_id = target_user_id;
    mod.assigned_by = user_id;
    mod.permission_level = permission_level;
    mod.can_manage_posts = can_manage_posts;
    mod.can_manage_comments = can_manage_comments;
    mod.can_manage_users = can_manage_users;
    mod.can_manage_reports = can_manage_reports;
    return repository::AdvancedRepository::Instance().AddModerator(mod);
}

bool AdvancedService::RemoveModerator(const std::string& token, int64_t section_id,
                                      const std::string& target_user_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    if (!IsAdmin(user_id)) {
        return false;
    }
    return repository::AdvancedRepository::Instance().RemoveModerator(section_id, target_user_id);
}

std::vector<repository::ModeratorEntity> AdvancedService::GetUserModeratorRoles(
    const std::string& user_id) {
    return repository::AdvancedRepository::Instance().GetUserModeratorRoles(user_id);
}

bool AdvancedService::CheckModeratorPermission(const std::string& user_id,
                                               int64_t section_id,
                                               const std::string& permission) {
    if (IsAdmin(user_id)) {
        return true;
    }
    return repository::AdvancedRepository::Instance().CheckModeratorPermission(
        user_id, section_id, permission);
}

bool AdvancedService::PunishUser(const std::string& token, const std::string& target_user_id,
                                 const std::string& punishment_type, const std::string& reason,
                                 int64_t duration_ms, int points_deducted) {
    std::string operator_id = ValidateAndGetUserId(token);
    if (operator_id.empty()) {
        return false;
    }
    if (!IsAdmin(operator_id) && !CheckModeratorPermission(operator_id, 0, "users")) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::PunishmentEntity punishment;
    punishment.user_id = target_user_id;
    punishment.punishment_type = punishment_type;
    punishment.reason = reason;
    punishment.duration = duration_ms;
    punishment.points_deducted = points_deducted;
    punishment.executed_by = operator_id;
    punishment.executed_at = now_ms;
    punishment.expires_at = duration_ms > 0 ? now_ms + duration_ms : 0;
    bool success = repository::AdvancedRepository::Instance().CreatePunishment(punishment);
    if (success && points_deducted > 0) {
        repository::UserRepository::Instance().DeductPoints(target_user_id, points_deducted);
    }
    return success;
}

bool AdvancedService::AutoPunishUser(const std::string& user_id, const std::string& offense_type,
                                     const std::string& reason) {
    int& count = user_offense_counts_[user_id + "_" + offense_type];
    count++;
    auto it = punishment_rules_.find(offense_type);
    if (it == punishment_rules_.end()) {
        return false;
    }
    PunishmentRule* matched_rule = nullptr;
    for (auto& rule : it->second) {
        if (count >= rule.points_threshold) {
            matched_rule = &rule;
        }
    }
    if (!matched_rule) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    repository::PunishmentEntity punishment;
    punishment.user_id = user_id;
    punishment.punishment_type = matched_rule->punishment_type;
    punishment.reason = reason + " (自动处罚 - 第" + std::to_string(count) + "次违规)";
    punishment.duration = matched_rule->duration_ms;
    punishment.points_deducted = matched_rule->points_deduction;
    punishment.executed_by = "system";
    punishment.executed_at = now_ms;
    punishment.expires_at = matched_rule->duration_ms > 0 ?
        now_ms + matched_rule->duration_ms : 0;
    bool success = repository::AdvancedRepository::Instance().CreatePunishment(punishment);
    if (success && matched_rule->points_deduction > 0) {
        repository::UserRepository::Instance().DeductPoints(
            user_id, matched_rule->points_deduction);
    }
    return success;
}

std::vector<repository::PunishmentEntity> AdvancedService::GetUserPunishments(
    const std::string& token, const std::string& target_user_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    if (user_id != target_user_id && !IsAdmin(user_id)) {
        return {};
    }
    return repository::AdvancedRepository::Instance().GetActivePunishments(target_user_id);
}

bool AdvancedService::IsUserPunished(const std::string& user_id,
                                     const std::string& punishment_type) {
    auto punishments = repository::AdvancedRepository::Instance().GetActivePunishments(user_id);
    for (auto& p : punishments) {
        if (p.punishment_type == punishment_type && p.is_active) {
            return true;
        }
    }
    return false;
}

void AdvancedService::ExpireOldPunishments() {
    repository::AdvancedRepository::Instance().ExpireOldPunishments();
}

bool AdvancedService::CreatePoll(const std::string& token, int64_t post_id,
                                 const std::string& question,
                                 const std::vector<std::string>& options,
                                 bool is_multiple, int64_t end_at) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto post = repository::PostRepository::Instance().GetPost(post_id);
    if (!post || post->author_id != user_id) {
        return false;
    }
    repository::PollEntity poll;
    poll.post_id = post_id;
    poll.question = question;
    poll.options = options;
    poll.vote_counts.resize(options.size(), 0);
    poll.is_multiple = is_multiple;
    poll.end_at = end_at;
    return repository::AdvancedRepository::Instance().CreatePoll(poll);
}

std::optional<repository::PollEntity> AdvancedService::GetPoll(
    const std::string& token, int64_t post_id) {
    std::string user_id = ValidateAndGetUserId(token);
    return repository::AdvancedRepository::Instance().GetPoll(post_id, user_id);
}

bool AdvancedService::VotePoll(const std::string& token, int64_t post_id,
                               const std::vector<int32_t>& option_indices) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    if (option_indices.empty()) {
        return false;
    }
    auto poll = repository::AdvancedRepository::Instance().GetPoll(post_id);
    if (!poll) {
        return false;
    }
    if (!poll->is_multiple && option_indices.size() > 1) {
        return false;
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    if (poll->end_at > 0 && now_ms > poll->end_at) {
        return false;
    }
    bool all_success = true;
    for (int idx : option_indices) {
        if (!repository::AdvancedRepository::Instance().VotePoll(
            post_id, user_id, idx)) {
            all_success = false;
        }
    }
    return all_success;
}

void AdvancedService::CalculateHotScores() {
    auto posts = repository::PostRepository::Instance().GetRecentPosts(1000);
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    for (auto& post : posts) {
        double age_hours = (now_ms - post.created_at) / (1000.0 * 3600.0);
        double view_weight = post.view_count * 0.1;
        double like_weight = post.like_count * 1.0;
        double comment_weight = post.comment_count * 2.0;
        double gravity = 1.8;
        double hot_score = (view_weight + like_weight + comment_weight) /
            std::pow((age_hours + 2), gravity);
        repository::HotScoreEntity score;
        score.post_id = post.id;
        score.hot_score = hot_score;
        score.view_weight = view_weight;
        score.like_weight = like_weight;
        score.comment_weight = comment_weight;
        repository::AdvancedRepository::Instance().UpdateHotScore(score);
    }
}

std::vector<int64_t> AdvancedService::GetHotPosts(int limit, int offset) {
    return repository::AdvancedRepository::Instance().GetTopHotPosts(limit, offset);
}

std::vector<int64_t> AdvancedService::GetPersonalizedFeed(const std::string& user_id,
                                                          int limit, int offset) {
    auto settings = repository::AdvancedRepository::Instance().GetFeedSettings(user_id);
    if (!settings) {
        return GetHotPosts(limit, offset);
    }
    if (settings->feed_type == "latest") {
        auto posts = repository::PostRepository::Instance().GetPosts(
            0, "", "", offset, limit);
        std::vector<int64_t> ids;
        for (auto& p : posts) {
            ids.push_back(p.id);
        }
        return ids;
    }
    if (settings->feed_type == "following") {
        auto posts = repository::SocialRepository::Instance().GetFollowingPosts(
            user_id, offset, limit);
        std::vector<int64_t> ids;
        for (auto& p : posts) {
            ids.push_back(p.id);
        }
        return ids;
    }
    return GetHotPosts(limit, offset);
}

bool AdvancedService::SetUserWatermark(const std::string& token, const std::string& text,
                                       const std::string& position, double opacity,
                                       bool is_enabled) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    repository::WatermarkEntity wm;
    wm.user_id = user_id;
    wm.watermark_text = text;
    wm.watermark_position = position;
    wm.opacity = opacity;
    wm.is_enabled = is_enabled;
    return repository::AdvancedRepository::Instance().SetUserWatermark(wm);
}

std::optional<repository::WatermarkEntity> AdvancedService::GetUserWatermark(
    const std::string& token) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return std::nullopt;
    }
    return repository::AdvancedRepository::Instance().GetUserWatermark(user_id);
}

bool AdvancedService::SetFeedSettings(const std::string& token, const std::string& feed_type,
                                      const std::vector<int64_t>& include_sections,
                                      const std::vector<std::string>& exclude_tags) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    repository::FeedSettingsEntity settings;
    settings.user_id = user_id;
    settings.feed_type = feed_type;
    settings.include_sections = include_sections;
    settings.exclude_tags = exclude_tags;
    return repository::AdvancedRepository::Instance().SetFeedSettings(settings);
}

std::optional<repository::FeedSettingsEntity> AdvancedService::GetFeedSettings(
    const std::string& token) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return std::nullopt;
    }
    return repository::AdvancedRepository::Instance().GetFeedSettings(user_id);
}

void AdvancedService::LogRecommendation(const std::string& user_id, int64_t post_id,
                                        const std::string& algorithm, const std::string& action,
                                        double score) {
    repository::AdvancedRepository::Instance().LogRecommendation(
        user_id, post_id, algorithm, action, score);
}

int64_t AdvancedService::CreateGroup(const std::string& token, const std::string& name,
                                     const std::string& description, bool is_public,
                                     bool allow_join_request,
                                     const std::vector<std::string>& tags) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return 0;
    }
    repository::GroupEntity group;
    group.name = name;
    group.description = description;
    group.creator_id = user_id;
    group.is_public = is_public;
    group.allow_join_request = allow_join_request;
    group.tags = tags;
    return repository::AdvancedRepository::Instance().CreateGroup(group);
}

std::vector<repository::GroupEntity> AdvancedService::GetGroups(
    const std::string& token, const std::string& user_id,
    const std::string& tag, const std::string& keyword,
    int page, int page_size, int& out_total) {
    std::string requester_id = ValidateAndGetUserId(token);
    return repository::AdvancedRepository::Instance().SearchGroups(
        user_id, tag, keyword, page, page_size, out_total);
}

std::optional<repository::GroupEntity> AdvancedService::GetGroupDetail(
    const std::string& token, int64_t group_id) {
    std::string user_id = ValidateAndGetUserId(token);
    return repository::AdvancedRepository::Instance().GetGroupById(group_id);
}

bool AdvancedService::ManageGroupMember(const std::string& token, int64_t group_id,
                                        const std::string& target_user_id, int action,
                                        int new_role) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    auto group = repository::AdvancedRepository::Instance().GetGroupById(group_id);
    if (!group) {
        return false;
    }
    if (group->creator_id != user_id) {
        auto members = repository::AdvancedRepository::Instance().GetGroupMembers(group_id);
        bool is_admin = false;
        for (auto& m : members) {
            if (m.user_id == user_id && m.role == "admin") {
                is_admin = true;
                break;
            }
        }
        if (!is_admin) {
            return false;
        }
    }
    switch (action) {
        case 0:
            return repository::AdvancedRepository::Instance().AddGroupMember(
                group_id, target_user_id, "member");
        case 1:
            return repository::AdvancedRepository::Instance().RemoveGroupMember(
                group_id, target_user_id);
        case 2:
            return repository::AdvancedRepository::Instance().UpdateGroupMemberRole(
                group_id, target_user_id, new_role == 1 ? "admin" : "member");
        case 3:
            return repository::AdvancedRepository::Instance().HandleJoinRequest(
                group_id, target_user_id, true);
        case 4:
            return repository::AdvancedRepository::Instance().HandleJoinRequest(
                group_id, target_user_id, false);
        default:
            return false;
    }
}

int64_t AdvancedService::CreateGroupPost(const std::string& token, int64_t group_id,
                                         const std::string& title, const std::string& content,
                                         const std::vector<int64_t>& tag_ids) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return 0;
    }
    PostEntity post;
    post.title = title;
    post.content = content;
    post.author_id = user_id;
    post.section_id = 0;
    post.group_id = group_id;
    post.tag_ids = tag_ids;
    return repository::PostRepository::Instance().CreatePost(post);
}

int64_t AdvancedService::RegisterEvent(const std::string& token, int64_t event_id,
                                       int32_t ticket_type, int32_t guest_count,
                                       const std::string& contact) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return 0;
    }
    repository::EventRegEntity reg;
    reg.event_id = event_id;
    reg.user_id = user_id;
    reg.ticket_type = ticket_type;
    reg.guest_count = guest_count;
    reg.contact = contact;
    return repository::AdvancedRepository::Instance().CreateEventRegistration(reg);
}

std::vector<repository::EventRegEntity> AdvancedService::GetEventRegistrations(
    const std::string& token, int64_t event_id,
    int page, int page_size, int& out_total, int& out_confirmed) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    auto event = repository::CommunityRepository::Instance().GetEvent(event_id);
    if (!event || event->creator_id != user_id) {
        return {};
    }
    return repository::AdvancedRepository::Instance().GetEventRegistrations(
        event_id, page, page_size, out_total, out_confirmed);
}

bool AdvancedService::UpdateRegistrationStatus(const std::string& token, int64_t reg_id,
                                               int32_t new_status) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::AdvancedRepository::Instance().UpdateRegistrationStatus(
        reg_id, new_status);
}

std::vector<repository::MentionEntity> AdvancedService::GetMentions(
    const std::string& token, bool only_unread,
    int page, int page_size, int& out_total, int& out_unread) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::AdvancedRepository::Instance().GetUserMentions(
        user_id, only_unread, page, page_size, out_total, out_unread);
}

void AdvancedService::MarkMentionRead(const std::string& token, int64_t mention_id,
                                      bool mark_all) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return;
    }
    if (mark_all) {
        repository::AdvancedRepository::Instance().MarkAllMentionsRead(user_id);
    } else {
        repository::AdvancedRepository::Instance().MarkMentionRead(mention_id);
    }
}

bool AdvancedService::FavoritePost(const std::string& token, int64_t post_id,
                                   bool favorite) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    if (favorite) {
        return repository::AdvancedRepository::Instance().FavoritePost(user_id, post_id);
    } else {
        return repository::AdvancedRepository::Instance().UnfavoritePost(user_id, post_id);
    }
}

std::vector<PostEntity> AdvancedService::GetFavoritePosts(
    const std::string& token, int page, int page_size, int& out_total) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::AdvancedRepository::Instance().GetUserFavorites(
        user_id, page, page_size, out_total);
}

int64_t AdvancedService::SaveDraft(const std::string& token, int64_t draft_id,
                                   const std::string& title, const std::string& content,
                                   int32_t section_id, const std::vector<int64_t>& tag_ids,
                                   int64_t group_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return 0;
    }
    repository::DraftEntity draft;
    draft.id = draft_id;
    draft.user_id = user_id;
    draft.title = title;
    draft.content = content;
    draft.section_id = section_id;
    draft.tag_ids = tag_ids;
    draft.group_id = group_id;
    return repository::AdvancedRepository::Instance().SaveDraft(draft);
}

std::vector<repository::DraftEntity> AdvancedService::GetDrafts(
    const std::string& token, int page, int page_size, int& out_total) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return {};
    }
    return repository::AdvancedRepository::Instance().GetUserDrafts(
        user_id, page, page_size, out_total);
}

bool AdvancedService::DeleteDraft(const std::string& token, int64_t draft_id) {
    std::string user_id = ValidateAndGetUserId(token);
    if (user_id.empty()) {
        return false;
    }
    return repository::AdvancedRepository::Instance().DeleteDraft(draft_id, user_id);
}

void AdvancedService::ProcessMentions(const std::string& user_id,
                                      const std::string& content,
                                      int64_t post_id, int64_t comment_id) {
    repository::AdvancedRepository::Instance().ProcessMentions(
        user_id, content, post_id, comment_id);
}

} // namespace furbbs::service
