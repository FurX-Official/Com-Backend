#include "base_controller.h"
#include "../service_impl/social_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status FollowUser(::trpc::ServerContextPtr context,
                           const ::furbbs::FollowUserRequest* request,
                           ::furbbs::FollowUserResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SocialService::Instance().FollowUser(
        auth.user_id, request->target_user_id(), request->follow());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetFollowers(::trpc::ServerContextPtr context,
                              const ::furbbs::GetFollowListRequest* request,
                              ::furbbs::GetFollowListResponse* response) {
    int total = 0;
    auto users = service::SocialService::Instance().GetFollowers(
        request->user_id(), request->page(), request->page_size(), total);

    for (const auto& u : users) {
        auto* out = response->add_users();
        out->set_id(u.id);
        out->set_username(u.username);
        out->set_avatar(u.avatar);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetFollowing(::trpc::ServerContextPtr context,
                              const ::furbbs::GetFollowListRequest* request,
                              ::furbbs::GetFollowListResponse* response) {
    int total = 0;
    auto users = service::SocialService::Instance().GetFollowing(
        request->user_id(), request->page(), request->page_size(), total);

    for (const auto& u : users) {
        auto* out = response->add_users();
        out->set_id(u.id);
        out->set_username(u.username);
        out->set_avatar(u.avatar);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CheckFriendship(::trpc::ServerContextPtr context,
                                 const ::furbbs::CheckFriendshipRequest* request,
                                 ::furbbs::CheckFriendshipResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    
    auto rel = service::SocialService::Instance().CheckFriendship(
        auth.user_id, request->target_user_id());

    response->set_is_following(rel.is_following);
    response->set_is_followed(rel.is_followed);
    response->set_friends_count(rel.mutual_friends_count);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status FavoriteFursona(::trpc::ServerContextPtr context,
                                 const ::furbbs::FavoriteFursonaRequest* request,
                                 ::furbbs::FavoriteFursonaResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SocialService::Instance().FavoriteFursona(
        auth.user_id, request->fursona_id(), request->favorite());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status SendGift(::trpc::ServerContextPtr context,
                          const ::furbbs::SendGiftRequest* request,
                          ::furbbs::SendGiftResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SocialService::Instance().SendGift(
        auth.user_id, request->target_user_id(), request->gift_id(),
        request->quantity(), request->message(), request->anonymous());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Sent" : "Insufficient points or failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserGifts(::trpc::ServerContextPtr context,
                              const ::furbbs::GetUserGiftsRequest* request,
                              ::furbbs::GetUserGiftsResponse* response) {
    int total = 0;
    auto gifts = service::SocialService::Instance().GetUserGifts(
        request->user_id(), request->page(), request->page_size(), total);

    for (const auto& g : gifts) {
        auto* out = response->add_gifts();
        out->set_gift_id(g.gift_id);
        out->set_sender_name(g.sender_name);
        out->set_quantity(g.quantity);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status RateCommission(::trpc::ServerContextPtr context,
                                const ::furbbs::RateCommissionRequest* request,
                                ::furbbs::RateCommissionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SocialService::Instance().RateCommission(
        auth.user_id, request->artist_id(), request->rating(),
        request->comment(), request->anonymous());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Rated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status SubmitQuestion(::trpc::ServerContextPtr context,
                                const ::furbbs::SubmitQuestionRequest* request,
                                ::furbbs::SubmitQuestionResponse* response) {
    bool success = service::SocialService::Instance().SubmitQuestion(
        request->target_user_id(), request->content(),
        request->anonymous(), request->sender_id());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Submitted" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status AnswerQuestion(::trpc::ServerContextPtr context,
                                const ::furbbs::AnswerQuestionRequest* request,
                                ::furbbs::AnswerQuestionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SocialService::Instance().AnswerQuestion(
        auth.user_id, request->question_id(), request->answer(),
        request->is_public());

    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Answered" : "Permission denied");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
