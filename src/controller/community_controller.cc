#include "base_controller.h"
#include "../service_impl/community_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status SaveFursonaCard(::trpc::ServerContextPtr context,
                                const ::furbbs::SaveFursonaCardRequest* request,
                                ::furbbs::SaveFursonaCardResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::FursonaCardEntity data;
    data.fursona_id = request->fursona_id();
    data.template_id = request->template_id();
    data.theme_color = request->theme_color();
    data.background_image = request->background_image();
    data.show_stats = request->show_stats();
    data.show_artist = request->show_artist();
    data.card_layout = request->card_layout();

    int64_t id = service::CommunityService::Instance().SaveCard(auth.user_id, data);
    response->set_card_id(id);
    BaseController::SetResponse(response, 200, "Saved");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetFursonaCard(::trpc::ServerContextPtr context,
                               const ::furbbs::GetFursonaCardRequest* request,
                               ::furbbs::GetFursonaCardResponse* response) {
    auto card = service::CommunityService::Instance().GetCard(request->fursona_id());
    if (!card) {
        BaseController::SetResponse(response, 404, "Not found");
        return ::trpc::kSuccStatus;
    }

    response->set_card_id(card->id);
    response->set_fursona_id(card->fursona_id);
    response->set_fursona_name(card->fursona_name);
    response->set_species(card->species);
    response->set_gender(card->gender);
    response->set_template_id(card->template_id);
    response->set_theme_color(card->theme_color);
    response->set_background_image(card->background_image);
    response->set_show_stats(card->show_stats);
    response->set_show_artist(card->show_artist);
    response->set_card_layout(card->card_layout);
    response->set_view_count(card->view_count);

    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status SetContentRating(::trpc::ServerContextPtr context,
                                 const ::furbbs::SetContentRatingRequest* request,
                                 ::furbbs::SetContentRatingResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::ContentRatingEntity data;
    data.content_type = request->content_type();
    data.content_id = request->content_id();
    data.user_id = auth.user_id;
    data.rating_level = request->rating_level();
    for (int i = 0; i < request->content_warnings_size(); i++) {
        data.content_warnings.push_back(request->content_warnings(i));
    }
    data.rated_by = auth.user_id;

    bool success = service::CommunityService::Instance().SetContentRating(data);
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Set" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status UpdateContentPrefs(::trpc::ServerContextPtr context,
                                   const ::furbbs::UpdateContentPrefsRequest* request,
                                   ::furbbs::UpdateContentPrefsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::ContentPrefsEntity data;
    data.user_id = auth.user_id;
    data.show_safe = request->show_safe();
    data.show_questionable = request->show_questionable();
    data.show_explicit = request->show_explicit();
    for (int i = 0; i < request->enabled_warnings_size(); i++) {
        data.enabled_warnings.push_back(request->enabled_warnings(i));
    }
    data.blur_sensitive = request->blur_sensitive();

    bool success = service::CommunityService::Instance().UpdateContentPrefs(data);
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status RequestCreationPermission(::trpc::ServerContextPtr context,
                                          const ::furbbs::RequestCreationPermissionRequest* request,
                                          ::furbbs::RequestCreationPermissionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::CreationPermissionEntity data;
    data.author_user_id = request->author_user_id();
    data.authorized_user_id = auth.user_id;
    data.fursona_id = request->fursona_id();
    data.permission_type = request->permission_type();
    data.terms = request->terms();

    int64_t id = service::CommunityService::Instance().RequestPermission(data);
    response->set_permission_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Requested" : "Invalid request");
    return ::trpc::kSuccStatus;
}

::trpc::Status ApprovePermission(::trpc::ServerContextPtr context,
                                  const ::furbbs::ApprovePermissionRequest* request,
                                  ::furbbs::ApprovePermissionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CommunityService::Instance().ApprovePermission(
        auth.user_id, request->permission_id());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Approved" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status AddFursonaInteraction(::trpc::ServerContextPtr context,
                                      const ::furbbs::AddFursonaInteractionRequest* request,
                                      ::furbbs::AddFursonaInteractionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::FursonaInteractionEntity data;
    data.from_fursona_id = request->from_fursona_id();
    data.to_fursona_id = request->to_fursona_id();
    data.interaction_type = request->interaction_type();
    data.user_note = request->user_note();
    data.intimacy_score = request->intimacy_score();

    bool success = service::CommunityService::Instance().AddInteraction(data);
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Added" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status SubmitModeration(::trpc::ServerContextPtr context,
                                 const ::furbbs::SubmitModerationRequest* request,
                                 ::furbbs::SubmitModerationResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::ModerationItemEntity data;
    data.content_type = request->content_type();
    data.content_id = request->content_id();
    data.submitter_id = auth.user_id;

    bool success = service::CommunityService::Instance().SubmitToModeration(data);
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Submitted" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status ReviewModeration(::trpc::ServerContextPtr context,
                                 const ::furbbs::ReviewModerationRequest* request,
                                 ::furbbs::ReviewModerationResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::ModerationItemEntity data;
    data.id = request->queue_id();
    data.status = request->status();
    data.moderator_id = auth.user_id;
    data.moderator_note = request->moderator_note();
    data.violation_type = request->violation_type();

    bool success = service::CommunityService::Instance().ReviewModeration(data);
    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Reviewed" : "Failed");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
