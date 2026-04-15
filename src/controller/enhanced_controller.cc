#include "controller/furbbs_controller.h"
#include "service_impl/enhanced_service.h"

namespace furbbs {

using ::trpc::ServerContextPtr;

trpc::Status FurBbsControllerImpl::LikeComment(
    ServerContextPtr context,
    const CommentLikeRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().LikeComment(
            context->GetMetaData("authorization"),
            request->comment_id());
        response->set_success(success);
        response->set_message(success ? "点赞成功" : "点赞失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::UnlikeComment(
    ServerContextPtr context,
    const CommentUnlikeRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().UnlikeComment(
            context->GetMetaData("authorization"),
            request->comment_id());
        response->set_success(success);
        response->set_message(success ? "取消点赞成功" : "取消点赞失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetCommentLikeStats(
    ServerContextPtr context,
    const GetPostRequest* request,
    CommentLikeCountResponse* response) {
    try {
        int32_t count = service::EnhancedService::Instance().GetCommentLikeCount(
            request->id());
        bool has_liked = service::EnhancedService::Instance().HasLikedComment(
            context->GetMetaData("authorization"),
            request->id());
        response->set_like_count(count);
        response->set_has_liked(has_liked);
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RatePost(
    ServerContextPtr context,
    const RatePostRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RatePost(
            context->GetMetaData("authorization"),
            request->post_id(),
            request->score(),
            request->comment());
        response->set_success(success);
        response->set_message(success ? "评分成功" : "评分失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetPostAppreciations(
    ServerContextPtr context,
    const GetPostAppreciationsRequest* request,
    GetPostAppreciationsResponse* response) {
    try {
        auto appreciations = service::EnhancedService::Instance().GetPostAppreciations(
            request->post_id(),
            request->page(),
            request->page_size());
        auto stats = service::EnhancedService::Instance().GetPostRatingStats(
            request->post_id());
        for (auto& item : appreciations) {
            auto* info = response->add_appreciations();
            info->set_id(item.id);
            info->set_user_id(item.user_id);
            info->set_username(item.username);
            info->set_score(item.score);
            info->set_comment(item.comment);
            info->set_created_at(item.created_at);
        }
        response->set_total(stats.first);
        response->set_average_score(stats.second);
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RecordShare(
    ServerContextPtr context,
    const RecordShareRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RecordShare(
            context->GetMetaData("authorization"),
            request->post_id(),
            request->platform());
        response->set_success(success);
        response->set_message(success ? "分享记录成功" : "分享记录失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetShareStats(
    ServerContextPtr context,
    const GetPostRequest* request,
    ShareStatsResponse* response) {
    try {
        int32_t total = service::EnhancedService::Instance().GetShareCount(
            request->id());
        auto stats = service::EnhancedService::Instance().GetSharePlatformStats(
            request->id());
        response->set_total_shares(total);
        for (auto& item : stats) {
            (*response->mutable_platform_shares())[item.first] = item.second;
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RecordVisit(
    ServerContextPtr context,
    const RecordVisitRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RecordVisit(
            request->visitor_id(),
            request->target_user_id(),
            request->post_id(),
            request->ip_address(),
            request->user_agent(),
            request->duration());
        response->set_success(success);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetProfileVisitors(
    ServerContextPtr context,
    const GetProfileVisitorsRequest* request,
    GetProfileVisitorsResponse* response) {
    try {
        auto visitors = service::EnhancedService::Instance().GetProfileVisitors(
            context->GetMetaData("authorization"),
            request->page(),
            request->page_size());
        for (auto& item : visitors) {
            auto* info = response->add_visitors();
            info->set_user_id(item.visitor_id);
            info->set_last_visit(item.visited_at);
        }
        response->set_total(static_cast<int32_t>(visitors.size()));
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::AddUserTag(
    ServerContextPtr context,
    const AddUserTagRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().AddUserTag(
            context->GetMetaData("authorization"),
            request->tagged_user_id(),
            request->tag_name(),
            request->tag_color());
        response->set_success(success);
        response->set_message(success ? "标签添加成功" : "标签添加失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RemoveUserTag(
    ServerContextPtr context,
    const RemoveUserTagRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RemoveUserTag(
            context->GetMetaData("authorization"),
            request->tagged_user_id(),
            request->tag_name());
        response->set_success(success);
        response->set_message(success ? "标签删除成功" : "标签删除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetUserTags(
    ServerContextPtr context,
    const GetUserRequest* request,
    GetUserTagsResponse* response) {
    try {
        auto tags = service::EnhancedService::Instance().GetUserTags(
            context->GetMetaData("authorization"),
            request->user_id());
        for (auto& item : tags) {
            auto* info = response->add_tags();
            info->set_tag_name(item.tag_name);
            info->set_tag_color(item.tag_color);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::SetReadingProgress(
    ServerContextPtr context,
    const SetReadingProgressRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().SetReadingProgress(
            context->GetMetaData("authorization"),
            request->post_id(),
            request->progress_percent(),
            request->position(),
            request->total_words());
        response->set_success(success);
        response->set_message(success ? "阅读进度已保存" : "进度保存失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetReadingProgress(
    ServerContextPtr context,
    const GetPostRequest* request,
    GetReadingProgressResponse* response) {
    try {
        auto progress = service::EnhancedService::Instance().GetReadingProgress(
            context->GetMetaData("authorization"),
            request->id());
        if (progress) {
            auto* info = response->mutable_progress();
            info->set_post_id(request->id());
            info->set_progress_percent(progress->progress_percent);
            info->set_position(progress->last_read_position);
            info->set_is_completed(progress->is_completed);
            info->set_last_read_at(progress->last_read_at);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetReadingHistoryList(
    ServerContextPtr context,
    const GetReadingHistoryRequest* request,
    GetReadingHistoryResponse* response) {
    try {
        auto history = service::EnhancedService::Instance().GetReadingHistory(
            context->GetMetaData("authorization"),
            request->page(),
            request->page_size());
        for (auto& item : history) {
            auto* info = response->add_history();
            info->set_post_id(item.post_id);
            info->set_progress_percent(item.progress_percent);
            info->set_last_read_at(item.last_read_at);
        }
        response->set_total(static_cast<int32_t>(history.size()));
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::AddNote(
    ServerContextPtr context,
    const AddNoteRequest* request,
    CommonResponse* response) {
    try {
        int64_t id = service::EnhancedService::Instance().AddNote(
            context->GetMetaData("authorization"),
            request->target_type(),
            request->target_id(),
            request->content(),
            request->color(),
            request->is_pinned());
        response->set_success(id > 0);
        response->set_message(id > 0 ? "笔记添加成功" : "笔记添加失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::UpdateNote(
    ServerContextPtr context,
    const UpdateNoteRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().UpdateNote(
            context->GetMetaData("authorization"),
            request->note_id(),
            request->content(),
            request->color(),
            request->is_pinned());
        response->set_success(success);
        response->set_message(success ? "笔记更新成功" : "笔记更新失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::DeleteNote(
    ServerContextPtr context,
    const DeleteNoteRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().DeleteNote(
            context->GetMetaData("authorization"),
            request->note_id());
        response->set_success(success);
        response->set_message(success ? "笔记删除成功" : "笔记删除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetNotes(
    ServerContextPtr context,
    const GetNotesRequest* request,
    GetNotesResponse* response) {
    try {
        auto notes = service::EnhancedService::Instance().GetNotes(
            context->GetMetaData("authorization"),
            request->target_type(),
            request->target_id());
        for (auto& item : notes) {
            auto* info = response->add_notes();
            info->set_id(item.id);
            info->set_content(item.note_content);
            info->set_color(item.color);
            info->set_is_pinned(item.is_pinned);
            info->set_created_at(item.created_at);
            info->set_updated_at(item.updated_at);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetContentHistory(
    ServerContextPtr context,
    const GetContentHistoryRequest* request,
    GetContentHistoryResponse* response) {
    try {
        auto history = service::EnhancedService::Instance().GetContentHistory(
            context->GetMetaData("authorization"),
            request->post_id(),
            request->comment_id(),
            request->page(),
            request->page_size());
        for (auto& item : history) {
            auto* record = response->add_history();
            record->set_id(item.id);
            record->set_edited_by(item.edited_by);
            record->set_old_content(item.old_content);
            record->set_new_content(item.new_content);
            record->set_reason(item.edit_reason);
            record->set_edited_at(item.edited_at);
        }
        response->set_total(static_cast<int32_t>(history.size()));
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::AddQuickAccess(
    ServerContextPtr context,
    const AddQuickAccessRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().AddQuickAccess(
            context->GetMetaData("authorization"),
            request->item_type(),
            request->item_id(),
            request->item_name(),
            request->item_icon(),
            request->sort_order());
        response->set_success(success);
        response->set_message(success ? "快捷访问添加成功" : "快捷访问添加失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RemoveQuickAccess(
    ServerContextPtr context,
    const RemoveQuickAccessRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RemoveQuickAccess(
            context->GetMetaData("authorization"),
            request->item_type(),
            request->item_id());
        response->set_success(success);
        response->set_message(success ? "快捷访问删除成功" : "快捷访问删除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetQuickAccess(
    ServerContextPtr context,
    const GetUserRequest* request,
    GetQuickAccessResponse* response) {
    try {
        auto items = service::EnhancedService::Instance().GetQuickAccess(
            context->GetMetaData("authorization"));
        for (auto& item : items) {
            auto* info = response->add_items();
            info->set_item_type(item.item_type);
            info->set_item_id(item.item_id);
            info->set_item_name(item.item_name);
            info->set_item_icon(item.item_icon);
            info->set_sort_order(item.sort_order);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::AddFilterWord(
    ServerContextPtr context,
    const AddFilterWordRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().AddFilterWord(
            context->GetMetaData("authorization"),
            request->word(),
            request->replacement(),
            request->filter_level(),
            request->is_regex());
        response->set_success(success);
        response->set_message(success ? "敏感词添加成功" : "敏感词添加失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RemoveFilterWord(
    ServerContextPtr context,
    const RemoveFilterWordRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RemoveFilterWord(
            context->GetMetaData("authorization"),
            request->id());
        response->set_success(success);
        response->set_message(success ? "敏感词删除成功" : "敏感词删除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetFilterWords(
    ServerContextPtr context,
    const GetSystemMetricsResponse* request,
    GetFilterWordsResponse* response) {
    try {
        auto words = service::EnhancedService::Instance().GetAllFilterWords();
        for (auto& item : words) {
            auto* info = response->add_words();
            info->set_id(item.id);
            info->set_word(item.word);
            info->set_replacement(item.replacement);
            info->set_filter_level(item.filter_level);
            info->set_is_regex(item.is_regex);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::CreateSeries(
    ServerContextPtr context,
    const CreateSeriesRequest* request,
    CommonResponse* response) {
    try {
        int64_t id = service::EnhancedService::Instance().CreateSeries(
            context->GetMetaData("authorization"),
            request->title(),
            request->description(),
            request->cover_image(),
            request->is_public());
        response->set_success(id > 0);
        response->set_message(id > 0 ? "系列创建成功" : "系列创建失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::UpdateSeries(
    ServerContextPtr context,
    const UpdateSeriesRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().UpdateSeries(
            context->GetMetaData("authorization"),
            request->series_id(),
            request->title(),
            request->description(),
            request->cover_image(),
            request->is_public());
        response->set_success(success);
        response->set_message(success ? "系列更新成功" : "系列更新失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::DeleteSeries(
    ServerContextPtr context,
    const DeleteSeriesRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().DeleteSeries(
            context->GetMetaData("authorization"),
            request->series_id());
        response->set_success(success);
        response->set_message(success ? "系列删除成功" : "系列删除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetUserSeries(
    ServerContextPtr context,
    const GetUserSeriesRequest* request,
    GetUserSeriesResponse* response) {
    try {
        auto series = service::EnhancedService::Instance().GetUserSeries(
            context->GetMetaData("authorization"),
            request->user_id(),
            request->page(),
            request->page_size());
        for (auto& item : series) {
            auto* info = response->add_series();
            info->set_id(item.id);
            info->set_title(item.title);
            info->set_description(item.description);
            info->set_cover_image(item.cover_image);
            info->set_is_public(item.is_public);
            info->set_post_count(item.post_count);
            info->set_view_count(item.view_count);
            info->set_created_at(item.created_at);
        }
        response->set_total(static_cast<int32_t>(series.size()));
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::AddPostToSeries(
    ServerContextPtr context,
    const AddPostToSeriesRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().AddPostToSeries(
            context->GetMetaData("authorization"),
            request->series_id(),
            request->post_id(),
            request->sort_order());
        response->set_success(success);
        response->set_message(success ? "文章添加到系列成功" : "添加失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::RemovePostFromSeries(
    ServerContextPtr context,
    const RemovePostFromSeriesRequest* request,
    CommonResponse* response) {
    try {
        bool success = service::EnhancedService::Instance().RemovePostFromSeries(
            context->GetMetaData("authorization"),
            request->series_id(),
            request->post_id());
        response->set_success(success);
        response->set_message(success ? "文章从系列移除成功" : "移除失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetSeriesPosts(
    ServerContextPtr context,
    const GetPostRequest* request,
    GetSeriesPostsResponse* response) {
    try {
        auto posts = service::EnhancedService::Instance().GetSeriesPosts(
            request->id());
        for (auto& item : posts) {
            auto* info = response->add_posts();
            info->set_post_id(item.post_id);
            info->set_title(item.post_title);
            info->set_sort_order(item.sort_order);
            info->set_added_at(item.added_at);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetUserPreferences(
    ServerContextPtr context,
    const GetUserPreferencesRequest* request,
    GetUserPreferencesResponse* response) {
    try {
        auto prefs = service::EnhancedService::Instance().GetUserPreferences(
            context->GetMetaData("authorization"));
        if (prefs) {
            auto* p = response->mutable_preferences();
            p->set_content_language(prefs->content_language);
            p->set_default_sort(prefs->default_sort);
            p->set_show_nsfw(prefs->show_nsfw);
            p->set_blur_nsfw(prefs->blur_nsfw);
            p->set_auto_play_video(prefs->auto_play_video);
            p->set_infinite_scroll(prefs->infinite_scroll);
            p->set_compact_mode(prefs->compact_mode);
            p->set_night_mode(prefs->night_mode);
            p->set_font_size(prefs->font_size);
            p->set_notify_on_like(prefs->notify_on_like);
            p->set_notify_on_comment(prefs->notify_on_comment);
            p->set_notify_on_follow(prefs->notify_on_follow);
            p->set_notify_on_mention(prefs->notify_on_mention);
            p->set_notify_on_message(prefs->notify_on_message);
            p->set_email_digest(prefs->email_digest);
            p->set_show_online_status(prefs->show_online_status);
            p->set_show_read_status(prefs->show_read_status);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::SetUserPreferences(
    ServerContextPtr context,
    const SetUserPreferencesRequest* request,
    CommonResponse* response) {
    try {
        repository::UserPreferencesEntity prefs;
        prefs.content_language = request->preferences().content_language();
        prefs.default_sort = request->preferences().default_sort();
        prefs.show_nsfw = request->preferences().show_nsfw();
        prefs.blur_nsfw = request->preferences().blur_nsfw();
        prefs.auto_play_video = request->preferences().auto_play_video();
        prefs.infinite_scroll = request->preferences().infinite_scroll();
        prefs.compact_mode = request->preferences().compact_mode();
        prefs.night_mode = request->preferences().night_mode();
        prefs.font_size = request->preferences().font_size();
        prefs.notify_on_like = request->preferences().notify_on_like();
        prefs.notify_on_comment = request->preferences().notify_on_comment();
        prefs.notify_on_follow = request->preferences().notify_on_follow();
        prefs.notify_on_mention = request->preferences().notify_on_mention();
        prefs.notify_on_message = request->preferences().notify_on_message();
        prefs.email_digest = request->preferences().email_digest();
        prefs.show_online_status = request->preferences().show_online_status();
        prefs.show_read_status = request->preferences().show_read_status();
        
        bool success = service::EnhancedService::Instance().SetUserPreferences(
            context->GetMetaData("authorization"), prefs);
        response->set_success(success);
        response->set_message(success ? "偏好设置已保存" : "偏好设置保存失败");
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_message(std::string("操作失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

trpc::Status FurBbsControllerImpl::GetTopContributors(
    ServerContextPtr context,
    const GetTopContributorsRequest* request,
    GetTopContributorsResponse* response) {
    try {
        auto rankings = service::EnhancedService::Instance().GetTopContributors(
            request->limit(),
            request->offset());
        for (auto& item : rankings) {
            auto* ranking = response->add_rankings();
            ranking->set_user_id(item.user_id);
            ranking->set_username(item.username);
            ranking->set_avatar(item.avatar);
            ranking->set_contribution_score(item.contribution_score);
            ranking->set_posts_count(item.posts_weight);
            ranking->set_comments_count(item.comments_weight);
            ranking->set_likes_count(item.likes_weight);
        }
        response->set_code(200);
        return trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(500);
        response->set_message(std::string("获取失败: ") + e.what());
        return trpc::kSuccStatus;
    }
}

} // namespace furbbs
