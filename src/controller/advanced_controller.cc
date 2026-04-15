#include "controller/base_controller.h"
#include "service_impl/advanced_service.h"
#include "furbbs.trpc.ffianimal.pb.h"

namespace furbbs::controller {

using namespace furbbs::trpc::ffianimal;
using namespace furbbs::service;

trpc::Status FurBbsControllerImpl::AddModerator(
    trpc::ServerContextPtr context,
    const AddModeratorRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().AddModerator(
            context->GetMetaData("authorization"),
            request->section_id(),
            request->target_user_id(),
            request->permission_level(),
            request->can_manage_posts(),
            request->can_manage_comments(),
            request->can_manage_users(),
            request->can_manage_reports());
        response->set_success(success);
        response->set_message(success ? "版主添加成功" : "版主添加失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RemoveModerator(
    trpc::ServerContextPtr context,
    const RemoveModeratorRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().RemoveModerator(
            context->GetMetaData("authorization"),
            request->section_id(),
            request->target_user_id());
        response->set_success(success);
        response->set_message(success ? "版主移除成功" : "版主移除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetUserModeratorRoles(
    trpc::ServerContextPtr context,
    const GetUserModeratorRolesRequest* request,
    GetUserModeratorRolesResponse* response) {
    try {
        auto roles = AdvancedService::Instance().GetUserModeratorRoles(
            request->user_id());
        for (auto& role : roles) {
            auto* r = response->add_roles();
            r->set_section_id(role.section_id);
            r->set_permission_level(role.permission_level);
            r->set_can_manage_posts(role.can_manage_posts);
            r->set_can_manage_comments(role.can_manage_comments);
            r->set_can_manage_users(role.can_manage_users);
            r->set_can_manage_reports(role.can_manage_reports);
            r->set_assigned_at(role.assigned_at);
        }
        response->set_success(true);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::PunishUser(
    trpc::ServerContextPtr context,
    const PunishUserRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().PunishUser(
            context->GetMetaData("authorization"),
            request->target_user_id(),
            request->punishment_type(),
            request->reason(),
            request->duration_ms(),
            request->points_deducted());
        response->set_success(success);
        response->set_message(success ? "处罚执行成功" : "处罚执行失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetUserPunishments(
    trpc::ServerContextPtr context,
    const GetUserPunishmentsRequest* request,
    GetUserPunishmentsResponse* response) {
    try {
        auto punishments = AdvancedService::Instance().GetUserPunishments(
            context->GetMetaData("authorization"),
            request->target_user_id());
        for (auto& p : punishments) {
            auto* item = response->add_punishments();
            item->set_punishment_type(p.punishment_type);
            item->set_reason(p.reason);
            item->set_executed_by(p.executed_by);
            item->set_executed_at(p.executed_at);
            item->set_expires_at(p.expires_at);
            item->set_points_deducted(p.points_deducted);
            item->set_is_active(p.is_active);
        }
        response->set_success(true);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::CreatePoll(
    trpc::ServerContextPtr context,
    const CreatePollRequest* request,
    CommonResponse* response) {
    try {
        std::vector<std::string> options;
        for (auto& opt : request->options()) {
            options.push_back(opt);
        }
        bool success = AdvancedService::Instance().CreatePoll(
            context->GetMetaData("authorization"),
            request->post_id(),
            request->question(),
            options,
            request->is_multiple(),
            request->end_at());
        response->set_success(success);
        response->set_message(success ? "投票创建成功" : "投票创建失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetPoll(
    trpc::ServerContextPtr context,
    const GetPollRequest* request,
    GetPollResponse* response) {
    try {
        auto poll = AdvancedService::Instance().GetPoll(
            context->GetMetaData("authorization"),
            request->post_id());
        if (poll) {
            response->set_question(poll->question);
            for (auto& opt : poll->options) {
                response->add_options(opt);
            }
            for (auto cnt : poll->vote_counts) {
                response->add_vote_counts(cnt);
            }
            response->set_is_multiple(poll->is_multiple);
            response->set_end_at(poll->end_at);
            response->set_has_voted(poll->has_voted);
            response->set_success(true);
        } else {
            response->set_success(false);
            response->set_message("投票不存在");
        }
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::VotePoll(
    trpc::ServerContextPtr context,
    const VotePollRequest* request,
    CommonResponse* response) {
    try {
        std::vector<int32_t> indices;
        for (auto idx : request->option_indices()) {
            indices.push_back(idx);
        }
        bool success = AdvancedService::Instance().VotePoll(
            context->GetMetaData("authorization"),
            request->post_id(),
            indices);
        response->set_success(success);
        response->set_message(success ? "投票成功" : "投票失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetHotPosts(
    trpc::ServerContextPtr context,
    const GetHotPostsRequest* request,
    GetPostListResponse* response) {
    try {
        auto post_ids = AdvancedService::Instance().GetHotPosts(
            request->limit(),
            request->offset());
        response->set_success(true);
        for (auto id : post_ids) {
            response->add_post_ids(id);
        }
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetPersonalizedFeed(
    trpc::ServerContextPtr context,
    const GetPersonalizedFeedRequest* request,
    GetPostListResponse* response) {
    try {
        auto auth_result = auth::CasdoorAuth::Instance().VerifyToken(
            context->GetMetaData("authorization"));
        std::string user_id = auth_result ? auth_result->user_id : "";
        auto post_ids = AdvancedService::Instance().GetPersonalizedFeed(
            user_id, request->limit(), request->offset());
        response->set_success(true);
        for (auto id : post_ids) {
            response->add_post_ids(id);
        }
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::SetUserWatermark(
    trpc::ServerContextPtr context,
    const SetUserWatermarkRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().SetUserWatermark(
            context->GetMetaData("authorization"),
            request->watermark_text(),
            request->position(),
            request->opacity(),
            request->is_enabled());
        response->set_success(success);
        response->set_message(success ? "水印设置成功" : "水印设置失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetUserWatermark(
    trpc::ServerContextPtr context,
    const GetUserWatermarkRequest* request,
    GetUserWatermarkResponse* response) {
    try {
        auto wm = AdvancedService::Instance().GetUserWatermark(
            context->GetMetaData("authorization"));
        if (wm) {
            response->set_watermark_text(wm->watermark_text);
            response->set_position(wm->watermark_position);
            response->set_opacity(wm->opacity);
            response->set_is_enabled(wm->is_enabled);
            response->set_success(true);
        } else {
            response->set_success(false);
        }
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::SetFeedSettings(
    trpc::ServerContextPtr context,
    const SetFeedSettingsRequest* request,
    CommonResponse* response) {
    try {
        std::vector<int64_t> sections;
        for (auto s : request->include_sections()) {
            sections.push_back(s);
        }
        std::vector<std::string> tags;
        for (auto& t : request->exclude_tags()) {
            tags.push_back(t);
        }
        bool success = AdvancedService::Instance().SetFeedSettings(
            context->GetMetaData("authorization"),
            request->feed_type(),
            sections,
            tags);
        response->set_success(success);
        response->set_message(success ? "订阅设置已保存" : "设置保存失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetFeedSettings(
    trpc::ServerContextPtr context,
    const GetFeedSettingsRequest* request,
    GetFeedSettingsResponse* response) {
    try {
        auto settings = AdvancedService::Instance().GetFeedSettings(
            context->GetMetaData("authorization"));
        if (settings) {
            response->set_feed_type(settings->feed_type);
            for (auto s : settings->include_sections) {
                response->add_include_sections(s);
            }
            for (auto& t : settings->exclude_tags) {
                response->add_exclude_tags(t);
            }
            response->set_success(true);
        } else {
            response->set_success(false);
        }
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::CreateGroup(
    trpc::ServerContextPtr context,
    const CreateGroupRequest* request,
    CreateEntityResponse* response) {
    try {
        std::vector<std::string> tags;
        for (auto& t : request->tags()) {
            tags.push_back(t);
        }
        int64_t group_id = AdvancedService::Instance().CreateGroup(
            context->GetMetaData("authorization"),
            request->name(),
            request->description(),
            request->is_public(),
            request->allow_join_request(),
            tags);
        response->set_id(group_id);
        response->set_success(group_id > 0);
        response->set_message(group_id > 0 ? "群组创建成功" : "群组创建失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetGroups(
    trpc::ServerContextPtr context,
    const GetGroupsRequest* request,
    GetGroupsResponse* response) {
    try {
        int total = 0;
        auto groups = AdvancedService::Instance().GetGroups(
            context->GetMetaData("authorization"),
            request->user_id(),
            request->tag(),
            request->keyword(),
            request->page(),
            request->page_size(),
            total);
        for (auto& g : groups) {
            auto* item = response->add_groups();
            item->set_id(g.id);
            item->set_name(g.name);
            item->set_description(g.description);
            item->set_creator_id(g.creator_id);
            item->set_is_public(g.is_public);
            item->set_member_count(g.member_count);
            item->set_created_at(g.created_at);
        }
        response->set_total(total);
        response->set_success(true);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::ManageGroupMember(
    trpc::ServerContextPtr context,
    const ManageGroupMemberRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().ManageGroupMember(
            context->GetMetaData("authorization"),
            request->group_id(),
            request->target_user_id(),
            request->action(),
            request->new_role());
        response->set_success(success);
        std::string msg;
        switch (request->action()) {
            case 0: msg = "添加成员"; break;
            case 1: msg = "移除成员"; break;
            case 2: msg = "更新角色"; break;
            case 3: msg = "同意申请"; break;
            case 4: msg = "拒绝申请"; break;
            default: msg = "操作";
        }
        response->set_message(success ? msg + "成功" : msg + "失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetMentions(
    trpc::ServerContextPtr context,
    const GetMentionsRequest* request,
    GetMentionsResponse* response) {
    try {
        int total = 0, unread = 0;
        auto mentions = AdvancedService::Instance().GetMentions(
            context->GetMetaData("authorization"),
            request->only_unread(),
            request->page(),
            request->page_size(),
            total, unread);
        for (auto& m : mentions) {
            auto* item = response->add_mentions();
            item->set_id(m.id);
            item->set_from_user_id(m.from_user_id);
            item->set_post_id(m.post_id);
            item->set_comment_id(m.comment_id);
            item->set_mentioned_at(m.mentioned_at);
            item->set_is_read(m.is_read);
        }
        response->set_total(total);
        response->set_unread_count(unread);
        response->set_success(true);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::MarkMentionRead(
    trpc::ServerContextPtr context,
    const MarkMentionReadRequest* request,
    CommonResponse* response) {
    try {
        AdvancedService::Instance().MarkMentionRead(
            context->GetMetaData("authorization"),
            request->mention_id(),
            request->mark_all());
        response->set_success(true);
        response->set_message("已标记为已读");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::FavoritePost(
    trpc::ServerContextPtr context,
    const FavoritePostRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().FavoritePost(
            context->GetMetaData("authorization"),
            request->post_id(),
            request->favorite());
        response->set_success(success);
        response->set_message(success ?
            (request->favorite() ? "收藏成功" : "取消收藏成功") : "操作失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetFavoritePosts(
    trpc::ServerContextPtr context,
    const GetFavoritePostsRequest* request,
    GetPostListResponse* response) {
    try {
        int total = 0;
        auto posts = AdvancedService::Instance().GetFavoritePosts(
            context->GetMetaData("authorization"),
            request->page(),
            request->page_size(),
            total);
        for (auto& p : posts) {
            response->add_post_ids(p.id);
        }
        response->set_total(total);
        response->set_success(true);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::SaveDraft(
    trpc::ServerContextPtr context,
    const SaveDraftRequest* request,
    CreateEntityResponse* response) {
    try {
        std::vector<int64_t> tag_ids;
        for (auto t : request->tag_ids()) {
            tag_ids.push_back(t);
        }
        int64_t draft_id = AdvancedService::Instance().SaveDraft(
            context->GetMetaData("authorization"),
            request->draft_id(),
            request->title(),
            request->content(),
            request->section_id(),
            tag_ids,
            request->group_id());
        response->set_id(draft_id);
        response->set_success(draft_id > 0);
        response->set_message(draft_id > 0 ? "草稿保存成功" : "草稿保存失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetDrafts(
    trpc::ServerContextPtr context,
    const GetDraftsRequest* request,
    GetDraftsResponse* response) {
    try {
        int total = 0;
        auto drafts = AdvancedService::Instance().GetDrafts(
            context->GetMetaData("authorization"),
            request->page(),
            request->page_size(),
            total);
        for (auto& d : drafts) {
            auto* item = response->add_drafts();
            item->set_id(d.id);
            item->set_title(d.title);
            item->set_section_id(d.section_id);
            item->set_group_id(d.group_id);
            item->set_updated_at(d.updated_at);
        }
        response->set_total(total);
        response->set_success(true);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::DeleteDraft(
    trpc::ServerContextPtr context,
    const DeleteDraftRequest* request,
    CommonResponse* response) {
    try {
        bool success = AdvancedService::Instance().DeleteDraft(
            context->GetMetaData("authorization"),
            request->draft_id());
        response->set_success(success);
        response->set_message(success ? "草稿删除成功" : "草稿删除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RegisterEvent(
    trpc::ServerContextPtr context,
    const RegisterEventRequest* request,
    CreateEntityResponse* response) {
    try {
        int64_t reg_id = AdvancedService::Instance().RegisterEvent(
            context->GetMetaData("authorization"),
            request->event_id(),
            request->ticket_type(),
            request->guest_count(),
            request->contact());
        response->set_id(reg_id);
        response->set_success(reg_id > 0);
        response->set_message(reg_id > 0 ? "报名成功" : "报名失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

} // namespace furbbs::controller
