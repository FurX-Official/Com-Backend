#include "base_controller.h"
#include "../service_impl/furry_core_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status CreateFursonaRelation(::trpc::ServerContextPtr context,
                                      const ::furbbs::CreateFursonaRelationRequest* request,
                                      ::furbbs::CreateFursonaRelationResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::FursonaRelationEntity data;
    data.fursona_a_id = request->fursona_a_id();
    data.fursona_b_id = request->fursona_b_id();
    data.relation_type = request->relation_type();
    data.anniversary = request->anniversary();

    int64_t id = service::FurryCoreService::Instance().CreateRelation(auth.user_id, data);
    response->set_relation_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Relation exists");
    return ::trpc::kSuccStatus;
}

::trpc::Status ConfirmRelation(::trpc::ServerContextPtr context,
                               const ::furbbs::ConfirmRelationRequest* request,
                               ::furbbs::ConfirmRelationResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::FurryCoreService::Instance().ConfirmRelation(
        auth.user_id, request->relation_id());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Confirmed" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetFursonaRelations(::trpc::ServerContextPtr context,
                                    const ::furbbs::GetFursonaRelationsRequest* request,
                                    ::furbbs::GetFursonaRelationsResponse* response) {
    auto list = service::FurryCoreService::Instance().GetFursonaRelations(request->fursona_id());

    for (auto& r : list) {
        auto* rel = response->add_relations();
        rel->set_id(r.id);
        rel->set_fursona_a_id(r.fursona_a_id);
        rel->set_fursona_a_name(r.fursona_a_name);
        rel->set_fursona_b_id(r.fursona_b_id);
        rel->set_fursona_b_name(r.fursona_b_name);
        rel->set_relation_type(r.relation_type);
        rel->set_user_a_confirmed(r.user_a_confirmed);
        rel->set_user_b_confirmed(r.user_b_confirmed);
        rel->set_anniversary(r.anniversary);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateWorldSetting(::trpc::ServerContextPtr context,
                                 const ::furbbs::CreateWorldSettingRequest* request,
                                 ::furbbs::CreateWorldSettingResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::WorldSettingEntity data;
    data.name = request->name();
    data.description = request->description();
    data.cover_image = request->cover_image();
    data.setting_type = request->setting_type();
    for (int i = 0; i < request->tags_size(); i++) {
        data.tags.push_back(request->tags(i));
    }
    data.is_public = request->is_public();

    int64_t id = service::FurryCoreService::Instance().CreateWorld(auth.user_id, data);
    response->set_world_id(id);
    BaseController::SetResponse(response, id > 0 ? 200 : 400,
                               id > 0 ? "Created" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetWorlds(::trpc::ServerContextPtr context,
                        const ::furbbs::GetWorldsRequest* request,
                        ::furbbs::GetWorldsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());

    auto list = service::FurryCoreService::Instance().GetWorlds(
        request->user_id(), auth.user_id, request->only_public(),
        request->page(), request->page_size());

    for (auto& w : list) {
        auto* world = response->add_worlds();
        world->set_id(w.id);
        world->set_name(w.name);
        world->set_description(w.description);
        world->set_cover_image(w.cover_image);
        world->set_is_public(w.is_public);
        world->set_view_count(w.view_count);
        world->set_like_count(w.like_count);
        world->set_created_at(w.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status LikeWorld(::trpc::ServerContextPtr context,
                          const ::furbbs::LikeWorldRequest* request,
                          ::furbbs::LikeWorldResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::FurryCoreService::Instance().LikeWorld(
        auth.user_id, request->world_id(), request->like());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
