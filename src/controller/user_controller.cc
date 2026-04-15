#include "base_controller.h"
#include "../service_impl/user_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status GetUserInfo(::trpc::ServerContextPtr context,
                           const ::furbbs::GetUserInfoRequest* request,
                           ::furbbs::GetUserInfoResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    auto info = service::UserService::Instance().GetUserInfo(
        auth.user_id, request->user_id());
    
    auto* out = response->mutable_user();
    out->set_id(info.id);
    out->set_username(info.username);
    out->set_avatar(info.avatar);
    out->set_signature(info.signature);
    out->set_points(info.points);
    out->set_level(info.level);
    out->set_gender(info.gender);
    out->set_created_at(info.created_at);
    
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status UpdateUserProfile(::trpc::ServerContextPtr context,
                                 const ::furbbs::UpdateProfileRequest* request,
                                 ::furbbs::UpdateProfileResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    service::UserProfileEntity profile;
    profile.signature = request->signature();
    profile.gender = request->gender();
    profile.location = request->location();
    profile.birthday = request->birthday();
    profile.website = request->website();

    bool success = service::UserService::Instance().UpdateProfile(
        auth.user_id, profile);
    
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserStats(::trpc::ServerContextPtr context,
                            const ::furbbs::GetUserStatsRequest* request,
                            ::furbbs::GetUserStatsResponse* response) {
    auto stats = service::UserService::Instance().GetUserStats(request->user_id());
    
    response->set_post_count(stats.post_count);
    response->set_comment_count(stats.comment_count);
    response->set_follower_count(stats.follower_count);
    response->set_following_count(stats.following_count);
    response->set_like_received(stats.like_received);
    
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status SearchUsers(::trpc::ServerContextPtr context,
                           const ::furbbs::SearchUsersRequest* request,
                           ::furbbs::SearchUsersResponse* response) {
    int total = 0;
    auto users = service::UserService::Instance().SearchUsers(
        request->keyword(), request->page(), request->page_size(), total);
    
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

::trpc::Status GetFursonaList(::trpc::ServerContextPtr context,
                              const ::furbbs::GetFursonaListRequest* request,
                              ::furbbs::GetFursonaListResponse* response) {
    int total = 0;
    auto fursonas = service::UserService::Instance().GetUserFursonas(
        request->user_id(), request->page(), request->page_size(), total);
    
    for (const auto& f : fursonas) {
        auto* out = response->add_fursonas();
        out->set_id(f.id);
        out->set_name(f.name);
        out->set_species(f.species);
        out->set_avatar(f.avatar);
    }
    response->set_total(total);
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateFursona(::trpc::ServerContextPtr context,
                             const ::furbbs::CreateFursonaRequest* request,
                             ::furbbs::CreateFursonaResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    service::FursonaEntity fursona;
    fursona.name = request->name();
    fursona.species = request->species();
    fursona.gender = request->gender();
    fursona.description = request->description();
    fursona.avatar = request->avatar();
    fursona.ref_sheet = request->ref_sheet();

    int64_t id = service::UserService::Instance().CreateFursona(auth.user_id, fursona);
    
    response->set_fursona_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status UpdateFursona(::trpc::ServerContextPtr context,
                             const ::furbbs::UpdateFursonaRequest* request,
                             ::furbbs::UpdateFursonaResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    service::FursonaEntity fursona;
    fursona.name = request->name();
    fursona.species = request->species();
    fursona.description = request->description();
    fursona.avatar = request->avatar();

    bool success = service::UserService::Instance().UpdateFursona(
        auth.user_id, request->fursona_id(), fursona);
    
    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Updated" : "Permission denied");
    return ::trpc::kSuccStatus;
}

::trpc::Status DeleteFursona(::trpc::ServerContextPtr context,
                             const ::furbbs::DeleteFursonaRequest* request,
                             ::furbbs::DeleteFursonaResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::UserService::Instance().DeleteFursona(
        auth.user_id, request->fursona_id());
    
    BaseController::SetResponse(response, success ? 200 : 403,
                               success ? "Deleted" : "Permission denied");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
