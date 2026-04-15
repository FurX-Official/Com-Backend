#include "base_controller.h"
#include "../service_impl/community_enhanced_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status CreateContentReport(::trpc::ServerContextPtr context,
                                    const ::furbbs::CreateContentReportRequest* request,
                                    ::furbbs::CreateContentReportResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::ContentReportEntity e;
    e.reporter_id = auth.user_id;
    e.content_type = request->content_type();
    e.content_id = request->content_id();
    e.report_reason = request->reason();
    e.report_details = request->details();

    int64_t id = service::CommunityEnhancedService::Instance().CreateReport(e);
    if (id > 0) {
        response->set_report_id(id);
        BaseController::SetResponse(response, 200, "Submitted");
    } else {
        BaseController::SetResponse(response, 400, "Already reported");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status GetPendingReports(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetPendingReportsRequest* request,
                                  ::furbbs::GetPendingReportsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    auto reports = service::CommunityEnhancedService::Instance().GetPendingReports(
        request->page(), request->page_size());
    for (auto& r : reports) {
        auto* item = response->add_reports();
        item->set_id(r.id);
        item->set_reporter_id(r.reporter_id);
        item->set_content_type(r.content_type);
        item->set_content_id(r.content_id);
        item->set_reason(r.report_reason);
        item->set_created_at(r.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status HandleReport(::trpc::ServerContextPtr context,
                             const ::furbbs::HandleReportRequest* request,
                             ::furbbs::HandleReportResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().HandleReport(
        request->report_id(), request->status(), auth.user_id, request->handler_notes());
    if (success) {
        BaseController::SetResponse(response, 200, "Handled");
    } else {
        BaseController::SetResponse(response, 404, "Not found");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateCommentReply(::trpc::ServerContextPtr context,
                                   const ::furbbs::CreateCommentReplyRequest* request,
                                   ::furbbs::CreateCommentReplyResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::CommentReplyEntity e;
    e.comment_id = request->comment_id();
    e.parent_reply_id = request->parent_reply_id();
    e.post_id = request->post_id();
    e.user_id = auth.user_id;
    e.reply_to_user_id = request->reply_to_user_id();
    e.content = request->content();

    int64_t id = service::CommunityEnhancedService::Instance().CreateReply(e);
    if (id > 0) {
        response->set_reply_id(id);
        BaseController::SetResponse(response, 200, "Created");
    } else {
        BaseController::SetResponse(response, 403, "Blocked by user");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status GetCommentReplies(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetCommentRepliesRequest* request,
                                  ::furbbs::GetCommentRepliesResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    auto replies = service::CommunityEnhancedService::Instance().GetCommentReplies(
        request->comment_id(), auth.valid ? auth.user_id : "");
    for (auto& r : replies) {
        auto* item = response->add_replies();
        item->set_id(r.id);
        item->set_user_id(r.user_id);
        item->set_reply_to_user_id(r.reply_to_user_id);
        item->set_content(r.content);
        item->set_like_count(r.like_count);
        item->set_created_at(r.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status DeleteCommentReply(::trpc::ServerContextPtr context,
                                    const ::furbbs::DeleteCommentReplyRequest* request,
                                    ::furbbs::DeleteCommentReplyResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().DeleteReply(
        request->reply_id(), auth.user_id);
    if (success) {
        BaseController::SetResponse(response, 200, "Deleted");
    } else {
        BaseController::SetResponse(response, 404, "Not found");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status SetPostSticky(::trpc::ServerContextPtr context,
                              const ::furbbs::SetPostStickyRequest* request,
                              ::furbbs::SetPostStickyResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().SetSticky(
        request->post_id(), request->section_id(), request->priority(), auth.user_id);
    if (success) {
        BaseController::SetResponse(response, 200, "Stickied");
    } else {
        BaseController::SetResponse(response, 400, "Failed");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status RemovePostSticky(::trpc::ServerContextPtr context,
                                 const ::furbbs::RemovePostStickyRequest* request,
                                 ::furbbs::RemovePostStickyResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().RemoveSticky(
        request->post_id(), auth.user_id);
    if (success) {
        BaseController::SetResponse(response, 200, "Unstickied");
    } else {
        BaseController::SetResponse(response, 404, "Not found");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status SetPostDigest(::trpc::ServerContextPtr context,
                              const ::furbbs::SetPostDigestRequest* request,
                              ::furbbs::SetPostDigestResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().SetDigest(
        request->post_id(), request->digest_level(), auth.user_id, request->description());
    if (success) {
        BaseController::SetResponse(response, 200, "Set");
    } else {
        BaseController::SetResponse(response, 400, "Failed");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateCollectionFolder(::trpc::ServerContextPtr context,
                                       const ::furbbs::CreateCollectionFolderRequest* request,
                                       ::furbbs::CreateCollectionFolderResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::CollectionFolderEntity e;
    e.user_id = auth.user_id;
    e.name = request->name();
    e.description = request->description();
    e.is_public = request->is_public();

    int64_t id = service::CommunityEnhancedService::Instance().CreateFolder(e);
    response->set_folder_id(id);
    BaseController::SetResponse(response, 200, "Created");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserFolders(::trpc::ServerContextPtr context,
                               const ::furbbs::GetUserFoldersRequest* request,
                               ::furbbs::GetUserFoldersResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto folders = service::CommunityEnhancedService::Instance().GetUserFolders(auth.user_id);
    for (auto& f : folders) {
        auto* item = response->add_folders();
        item->set_id(f.id);
        item->set_name(f.name);
        item->set_description(f.description);
        item->set_is_public(f.is_public);
        item->set_item_count(f.item_count);
        item->set_created_at(f.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status AddToCollectionFolder(::trpc::ServerContextPtr context,
                                      const ::furbbs::AddToCollectionFolderRequest* request,
                                      ::furbbs::AddToCollectionFolderResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().AddToFolder(
        request->folder_id(), request->post_id(), auth.user_id);
    if (success) {
        BaseController::SetResponse(response, 200, "Added");
    } else {
        BaseController::SetResponse(response, 400, "Already added");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status AddUserTag(::trpc::ServerContextPtr context,
                           const ::furbbs::AddUserTagRequest* request,
                           ::furbbs::AddUserTagResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().AddUserTag(
        auth.user_id, request->target_user_id(), request->tag());
    if (success) {
        BaseController::SetResponse(response, 200, "Added");
    } else {
        BaseController::SetResponse(response, 400, "Failed");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status RemoveUserTag(::trpc::ServerContextPtr context,
                              const ::furbbs::RemoveUserTagRequest* request,
                              ::furbbs::RemoveUserTagResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityEnhancedService::Instance().RemoveUserTag(
        auth.user_id, request->target_user_id(), request->tag());
    if (success) {
        BaseController::SetResponse(response, 200, "Removed");
    } else {
        BaseController::SetResponse(response, 404, "Not found");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status AddKeywordFilter(::trpc::ServerContextPtr context,
                                 const ::furbbs::AddKeywordFilterRequest* request,
                                 ::furbbs::AddKeywordFilterResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::UserKeywordFilterEntity e;
    e.user_id = auth.user_id;
    e.keyword = request->keyword();
    e.filter_type = request->filter_type();

    bool success = service::CommunityEnhancedService::Instance().AddKeywordFilter(e);
    if (success) {
        BaseController::SetResponse(response, 200, "Added");
    } else {
        BaseController::SetResponse(response, 400, "Already exists");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status GetKeywordFilters(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetKeywordFiltersRequest* request,
                                  ::furbbs::GetKeywordFiltersResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto filters = service::CommunityEnhancedService::Instance().GetUserFilters(auth.user_id);
    for (auto& f : filters) {
        auto* item = response->add_filters();
        item->set_keyword(f.keyword);
        item->set_filter_type(f.filter_type);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status SavePostDraft(::trpc::ServerContextPtr context,
                              const ::furbbs::SavePostDraftRequest* request,
                              ::furbbs::SavePostDraftResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::PostDraftEntity e;
    e.user_id = auth.user_id;
    e.title = request->title();
    e.content = request->content();
    e.section_id = request->section_id();
    e.fursona_id = request->fursona_id();
    e.is_auto_save = request->is_auto_save();

    int64_t id = service::CommunityEnhancedService::Instance().SaveDraft(e);
    response->set_draft_id(id);
    BaseController::SetResponse(response, 200, "Saved");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserDrafts(::trpc::ServerContextPtr context,
                              const ::furbbs::GetUserDraftsRequest* request,
                              ::furbbs::GetUserDraftsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto drafts = service::CommunityEnhancedService::Instance().GetUserDrafts(auth.user_id);
    for (auto& d : drafts) {
        auto* item = response->add_drafts();
        item->set_id(d.id);
        item->set_title(d.title);
        item->set_updated_at(d.updated_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status RecordPostView(::trpc::ServerContextPtr context,
                               const ::furbbs::RecordPostViewRequest* request,
                               ::furbbs::RecordPostViewResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    service::CommunityEnhancedService::Instance().RecordPostView(
        request->post_id(), auth.valid ? auth.user_id : "");
    BaseController::SetResponse(response, 200, "Recorded");
    return ::trpc::kSuccStatus;
}

::trpc::Status RecordPostShare(::trpc::ServerContextPtr context,
                                const ::furbbs::RecordPostShareRequest* request,
                                ::furbbs::RecordPostShareResponse* response) {
    service::CommunityEnhancedService::Instance().RecordPostShare(request->post_id());
    BaseController::SetResponse(response, 200, "Recorded");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
