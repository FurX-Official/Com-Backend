#include "base_controller.h"
#include "../service_impl/post_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status CreatePost(::trpc::ServerContextPtr context,
                            const ::furbbs::CreatePostRequest* request,
                            ::furbbs::CreatePostResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    service::PostEntity post;
    post.title = request->title();
    post.content = request->content();
    post.section_id = request->section_id();
    post.content_rating = request->content_rating();
    for (int i = 0; i < request->tags_size(); i++) {
        post.tags.push_back(request->tags(i));
    }
    for (int i = 0; i < request->images_size(); i++) {
        post.images.push_back(request->images(i));
    }

    int64_t id = service::PostService::Instance().CreatePost(auth.user_id, post);
    
    response->set_post_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetPost(::trpc::ServerContextPtr context,
                        const ::furbbs::GetPostRequest* request,
                        ::furbbs::GetPostResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    
    auto post = service::PostService::Instance().GetPost(
        request->post_id(), auth.user_id);
    
    if (!post.has_value()) {
        BaseController::SetResponse(response, 404, "Not found");
        return ::trpc::kSuccStatus;
    }

    auto* out = response->mutable_post();
    out->set_id(post->id);
    out->set_title(post->title);
    out->set_content(post->content);
    out->set_author_id(post->author_id);
    out->set_author_name(post->author_name);
    out->set_view_count(post->view_count);
    out->set_like_count(post->like_count);
    out->set_comment_count(post->comment_count);
    out->set_created_at(post->created_at);
    
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetPostList(::trpc::ServerContextPtr context,
                             const ::furbbs::GetPostListRequest* request,
                             ::furbbs::GetPostListResponse* response) {
    int total = 0;
    auto posts = service::PostService::Instance().GetPostList(
        request->section_id(), request->tag(), request->author_id(),
        request->sort(), request->page(), request->page_size(), total);
    
    for (const auto& p : posts) {
        auto* out = response->add_posts();
        out->set_id(p.id);
        out->set_title(p.title);
        out->set_author_name(p.author_name);
        out->set_like_count(p.like_count);
        out->set_comment_count(p.comment_count);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status UpdatePost(::trpc::ServerContextPtr context,
                            const ::furbbs::UpdatePostRequest* request,
                            ::furbbs::UpdatePostResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    service::PostEntity post;
    post.title = request->title();
    post.content = request->content();

    bool success = service::PostService::Instance().UpdatePost(
        auth.user_id, request->post_id(), post);
    
    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Updated" : "Permission denied");
    return ::trpc::kSuccStatus;
}

::trpc::Status DeletePost(::trpc::ServerContextPtr context,
                            const ::furbbs::DeletePostRequest* request,
                            ::furbbs::DeletePostResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::PostService::Instance().DeletePost(
        auth.user_id, auth.is_admin, request->post_id());
    
    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Deleted" : "Permission denied");
    return ::trpc::kSuccStatus;
}

::trpc::Status LikePost(::trpc::ServerContextPtr context,
                           const ::furbbs::LikePostRequest* request,
                           ::furbbs::LikePostResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::PostService::Instance().LikePost(
        auth.user_id, request->post_id(), request->like());
    
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateComment(::trpc::ServerContextPtr context,
                              const ::furbbs::CreateCommentRequest* request,
                              ::furbbs::CreateCommentResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    service::CommentEntity comment;
    comment.content = request->content();
    comment.parent_id = request->parent_id();

    int64_t id = service::PostService::Instance().CreateComment(
        auth.user_id, request->post_id(), comment);
    
    response->set_comment_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetComments(::trpc::ServerContextPtr context,
                               const ::furbbs::GetCommentsRequest* request,
                               ::furbbs::GetCommentsResponse* response) {
    int total = 0;
    auto comments = service::PostService::Instance().GetComments(
        request->post_id(), request->page(), request->page_size(), total);
    
    for (const auto& c : comments) {
        auto* out = response->add_comments();
        out->set_id(c.id);
        out->set_content(c.content);
        out->set_author_name(c.author_name);
        out->set_like_count(c.like_count);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status DeleteComment(::trpc::ServerContextPtr context,
                                const ::furbbs::DeleteCommentRequest* request,
                                ::furbbs::DeleteCommentResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::PostService::Instance().DeleteComment(
        auth.user_id, auth.is_admin, request->comment_id());
    
    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Deleted" : "Permission denied");
    return ::trpc::kSuccStatus;
}

::trpc::Status SearchPosts(::trpc::ServerContextPtr context,
                             const ::furbbs::SearchPostsRequest* request,
                             ::furbbs::SearchPostsResponse* response) {
    int total = 0;
    auto posts = service::PostService::Instance().SearchPosts(
        request->keyword(), request->page(), request->page_size(), total);
    
    for (const auto& p : posts) {
        auto* out = response->add_posts();
        out->set_id(p.id);
        out->set_title(p.title);
        out->set_author_name(p.author_name);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status SetPostEssence(::trpc::ServerContextPtr context,
                                 const ::furbbs::SetPostEssenceRequest* request,
                                 ::furbbs::SetPostEssenceResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Permission denied");
        return ::trpc::kSuccStatus;
    }

    bool success = service::PostService::Instance().SetEssence(
        request->post_id(), request->essence());
    
    BaseController::SetResponse(response, success ? 200 : 400, "Updated");
    return ::trpc::kSuccStatus;
}

::trpc::Status SetPostSticky(::trpc::ServerContextPtr context,
                               const ::furbbs::SetPostStickyRequest* request,
                               ::furbbs::SetPostStickyResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Permission denied");
        return ::trpc::kSuccStatus;
    }

    bool success = service::PostService::Instance().SetSticky(
        request->post_id(), request->sticky(), request->sticky_weight());
    
    BaseController::SetResponse(response, success ? 200 : 400, "Updated");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
