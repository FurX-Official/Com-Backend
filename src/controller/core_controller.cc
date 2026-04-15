#include "base_controller.h"
#include "../service_impl/core_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status CreateGalleryItem(::trpc::ServerContextPtr context,
                                  const ::furbbs::CreateGalleryItemRequest* request,
                                  ::furbbs::CreateGalleryItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::GalleryItemEntity data;
    data.user_id = auth.user_id;
    data.fursona_id = request->fursona_id();
    data.title = request->title();
    data.description = request->description();
    data.file_url = request->file_url();
    data.thumbnail_url = request->thumbnail_url();
    data.file_type = request->file_type();
    data.file_size = request->file_size();
    data.artist_name = request->artist_name();
    data.artist_url = request->artist_url();
    for (int i = 0; i < request->tags_size(); i++) {
        data.tags.push_back(request->tags(i));
    }
    data.is_public = request->is_public();
    data.is_nsfw = request->is_nsfw();

    int64_t id = service::CoreService::Instance().CreateGalleryItem(data);
    response->set_item_id(id);
    service::CoreService::Instance().LogAction(auth.user_id, "gallery_create", "item", id);
    BaseController::SetResponse(response, 200, "Created");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserGallery(::trpc::ServerContextPtr context,
                               const ::furbbs::GetUserGalleryRequest* request,
                               ::furbbs::GetUserGalleryResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());

    auto list = service::CoreService::Instance().GetUserGallery(
        request->user_id(), auth.user_id, request->page(), request->page_size());

    for (auto& item : list) {
        auto* p = response->add_items();
        p->set_id(item.id);
        p->set_title(item.title);
        p->set_thumbnail_url(item.thumbnail_url);
        p->set_file_type(item.file_type);
        p->set_is_nsfw(item.is_nsfw);
        p->set_view_count(item.view_count);
        p->set_like_count(item.like_count);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status LikeGalleryItem(::trpc::ServerContextPtr context,
                                const ::furbbs::LikeGalleryItemRequest* request,
                                ::furbbs::LikeGalleryItemResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CoreService::Instance().LikeGalleryItem(
        request->item_id(), auth.user_id, request->like());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateAlbum(::trpc::ServerContextPtr context,
                            const ::furbbs::CreateAlbumRequest* request,
                            ::furbbs::CreateAlbumResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::GalleryAlbumEntity data;
    data.user_id = auth.user_id;
    data.title = request->title();
    data.description = request->description();
    data.cover_image = request->cover_image();
    data.is_public = request->is_public();

    int64_t id = service::CoreService::Instance().CreateAlbum(data);
    response->set_album_id(id);
    BaseController::SetResponse(response, 200, "Created");
    return ::trpc::kSuccStatus;
}

::trpc::Status SearchContent(::trpc::ServerContextPtr context,
                              const ::furbbs::SearchContentRequest* request,
                              ::furbbs::SearchContentResponse* response) {
    auto results = service::CoreService::Instance().Search(
        request->query(), request->page(), request->page_size());

    for (auto& r : results) {
        auto* item = response->add_results();
        item->set_content_type(r.content_type);
        item->set_content_id(r.content_id);
        item->set_title(r.title);
        item->set_relevance(r.rank);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status UpdatePresence(::trpc::ServerContextPtr context,
                               const ::furbbs::UpdatePresenceRequest* request,
                               ::furbbs::UpdatePresenceResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::CoreService::Instance().UpdatePresence(
        auth.user_id, request->status());

    BaseController::SetResponse(response, success ? 200 : 400,
                               success ? "Updated" : "Failed");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetOnlineUsers(::trpc::ServerContextPtr context,
                               const ::furbbs::GetOnlineUsersRequest* request,
                               ::furbbs::GetOnlineUsersResponse* response) {
    auto list = service::CoreService::Instance().GetOnlineUsers(request->limit());

    for (auto& u : list) {
        auto* p = response->add_users();
        p->set_user_id(u.user_id);
        p->set_status(u.status);
        p->set_last_active(u.last_active);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status RequestDataExport(::trpc::ServerContextPtr context,
                                  const ::furbbs::RequestDataExportRequest* request,
                                  ::furbbs::RequestDataExportResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    int64_t id = service::CoreService::Instance().CreateExportTask(
        auth.user_id, request->export_type());

    response->set_task_id(id);
    service::CoreService::Instance().LogAction(auth.user_id, "export_request", "task", id);
    BaseController::SetResponse(response, 200, "Task created");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetPublicConfigs(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetPublicConfigsRequest* request,
                                 ::furbbs::GetPublicConfigsResponse* response) {
    auto configs = service::CoreService::Instance().GetPublicConfigs();

    for (auto& c : configs) {
        auto* p = response->add_configs();
        p->set_config_key(c.config_key);
        p->set_config_value(c.config_value);
        p->set_config_type(c.config_type);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetAuditLogs(::trpc::ServerContextPtr context,
                             const ::furbbs::GetAuditLogsRequest* request,
                             ::furbbs::GetAuditLogsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto logs = service::CoreService::Instance().GetUserAuditLogs(
        auth.user_id, request->page(), request->page_size());

    for (auto& l : logs) {
        auto* p = response->add_logs();
        p->set_action(l.action);
        p->set_resource_type(l.resource_type);
        p->set_resource_id(l.resource_id);
        p->set_created_at(l.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
