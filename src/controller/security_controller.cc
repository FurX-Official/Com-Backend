#include "base_controller.h"
#include "../service_impl/security_service.h"
#include "../auth/casdoor_auth.h"
#include <furbbs.pb.h>

namespace furbbs::controller {

::trpc::Status BlockUser(::trpc::ServerContextPtr context,
                          const ::furbbs::BlockUserRequest* request,
                          ::furbbs::BlockUserResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SecurityService::Instance().BlockUser(
        auth.user_id, request->target_user_id(), request->reason());

    if (success) {
        service::CoreService::Instance().LogAction(
            auth.user_id, "user_block", "user", 0);
        BaseController::SetResponse(response, 200, "Blocked");
    } else {
        BaseController::SetResponse(response, 400, "Failed");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status UnblockUser(::trpc::ServerContextPtr context,
                            const ::furbbs::UnblockUserRequest* request,
                            ::furbbs::UnblockUserResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SecurityService::Instance().UnblockUser(
        auth.user_id, request->target_user_id());

    if (success) {
        BaseController::SetResponse(response, 200, "Unblocked");
    } else {
        BaseController::SetResponse(response, 400, "Failed");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status GetBlockedUsers(::trpc::ServerContextPtr context,
                                const ::furbbs::GetBlockedUsersRequest* request,
                                ::furbbs::GetBlockedUsersResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto blocked = service::SecurityService::Instance().GetBlockedUsers(auth.user_id);
    for (auto& b : blocked) {
        response->add_blocked_user_ids(b.blocked_id);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserSessions(::trpc::ServerContextPtr context,
                                const ::furbbs::GetUserSessionsRequest* request,
                                ::furbbs::GetUserSessionsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto sessions = service::SecurityService::Instance().GetUserSessions(auth.user_id);
    for (auto& s : sessions) {
        auto* item = response->add_sessions();
        item->set_session_id(s.session_id);
        item->set_ip_address(s.ip_address);
        item->set_user_agent(s.user_agent);
        item->set_last_active(s.last_active);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status RevokeSession(::trpc::ServerContextPtr context,
                              const ::furbbs::RevokeSessionRequest* request,
                              ::furbbs::RevokeSessionResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SecurityService::Instance().RevokeSession(
        request->session_id(), auth.user_id);

    if (success) {
        BaseController::SetResponse(response, 200, "Session revoked");
    } else {
        BaseController::SetResponse(response, 404, "Session not found");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status GetLoginHistory(::trpc::ServerContextPtr context,
                                const ::furbbs::GetLoginHistoryRequest* request,
                                ::furbbs::GetLoginHistoryResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto history = service::SecurityService::Instance().GetLoginHistory(
        auth.user_id, request->page(), request->page_size());
    for (auto& h : history) {
        auto* item = response->add_history();
        item->set_ip_address(h.ip_address);
        item->set_location(h.location);
        item->set_user_agent(h.user_agent);
        item->set_success(h.was_successful);
        item->set_created_at(h.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetAnnouncements(::trpc::ServerContextPtr context,
                                 const ::furbbs::GetAnnouncementsRequest* request,
                                 ::furbbs::GetAnnouncementsResponse* response) {
    auto announcements = service::SecurityService::Instance().GetActiveAnnouncements();
    for (auto& a : announcements) {
        auto* item = response->add_announcements();
        item->set_id(a.id);
        item->set_title(a.title);
        item->set_content(a.content);
        item->set_type(a.announcement_type);
        item->set_priority(a.priority);
        item->set_created_at(a.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateAnnouncement(::trpc::ServerContextPtr context,
                                   const ::furbbs::CreateAnnouncementRequest* request,
                                   ::furbbs::CreateAnnouncementResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    repository::AnnouncementEntity e;
    e.title = request->title();
    e.content = request->content();
    e.announcement_type = request->type();
    e.priority = request->priority();
    e.created_by = auth.user_id;

    int64_t id = service::SecurityService::Instance().CreateAnnouncement(e);
    response->set_announcement_id(id);
    service::CoreService::Instance().LogAction(
        auth.user_id, "announcement_create", "announcement", id);
    BaseController::SetResponse(response, 200, "Created");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetSecurityAlerts(::trpc::ServerContextPtr context,
                                  const ::furbbs::GetSecurityAlertsRequest* request,
                                  ::furbbs::GetSecurityAlertsResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto alerts = service::SecurityService::Instance().GetUserAlerts(auth.user_id);
    for (auto& a : alerts) {
        auto* item = response->add_alerts();
        item->set_id(a.id);
        item->set_alert_type(a.alert_type);
        item->set_severity(a.severity);
        item->set_ip_address(a.ip_address);
        item->set_created_at(a.created_at);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

::trpc::Status ResolveAlert(::trpc::ServerContextPtr context,
                             const ::furbbs::ResolveAlertRequest* request,
                             ::furbbs::ResolveAlertResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SecurityService::Instance().ResolveAlert(
        request->alert_id(), auth.user_id);

    if (success) {
        BaseController::SetResponse(response, 200, "Resolved");
    } else {
        BaseController::SetResponse(response, 404, "Alert not found");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status AddIpToBlacklist(::trpc::ServerContextPtr context,
                                 const ::furbbs::AddIpToBlacklistRequest* request,
                                 ::furbbs::AddIpToBlacklistResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid || !auth.is_admin) {
        BaseController::SetResponse(response, 403, "Forbidden");
        return ::trpc::kSuccStatus;
    }

    bool success = service::SecurityService::Instance().AddIpToBlacklist(
        request->ip_address(), request->reason(),
        auth.user_id, request->duration_seconds());

    if (success) {
        service::CoreService::Instance().LogAction(
            auth.user_id, "ip_blacklist_add", "security", 0);
        BaseController::SetResponse(response, 200, "Added");
    } else {
        BaseController::SetResponse(response, 400, "Failed");
    }
    return ::trpc::kSuccStatus;
}

::trpc::Status CreateWebhook(::trpc::ServerContextPtr context,
                              const ::furbbs::CreateWebhookRequest* request,
                              ::furbbs::CreateWebhookResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    repository::WebhookEndpointEntity e;
    e.user_id = auth.user_id;
    e.endpoint_url = request->endpoint_url();
    e.secret = request->secret();
    for (int i = 0; i < request->events_size(); i++) {
        e.events.push_back(request->events(i));
    }

    int64_t id = service::SecurityService::Instance().CreateWebhook(e);
    response->set_webhook_id(id);
    BaseController::SetResponse(response, 200, "Created");
    return ::trpc::kSuccStatus;
}

::trpc::Status GetUserWebhooks(::trpc::ServerContextPtr context,
                                const ::furbbs::GetUserWebhooksRequest* request,
                                ::furbbs::GetUserWebhooksResponse* response) {
    auto auth = BaseController::VerifyToken(request->access_token());
    if (!auth.valid) {
        BaseController::SetResponse(response, 401, "Unauthorized");
        return ::trpc::kSuccStatus;
    }

    auto webhooks = service::SecurityService::Instance().GetUserWebhooks(auth.user_id);
    for (auto& w : webhooks) {
        auto* item = response->add_webhooks();
        item->set_id(w.id);
        item->set_endpoint_url(w.endpoint_url);
        item->set_is_active(w.is_active);
    }
    BaseController::SetResponse(response, 200, "Success");
    return ::trpc::kSuccStatus;
}

} // namespace furbbs::controller
