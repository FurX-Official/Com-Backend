#include "controller/common.h"
#include "service_impl/admin_service.h"

namespace furbbs::controller {

trpc::Status AdminController::CheckPermission(::trpc::ServerContextPtr,
                                               const ::furbbs::CheckPermissionRequest* request,
                                               ::furbbs::CheckPermissionResponse* response) {
    auto perm = static_cast<repository::Permission>(request->permission_code());
    bool has_perm = service::AdminService::Instance().CheckPermission(request->token(), perm);
    response->set_has_permission(has_perm);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::GetAllRoles(::trpc::ServerContextPtr,
                                           const ::furbbs::GetAllRolesRequest* request,
                                           ::furbbs::GetAllRolesResponse* response) {
    auto list = service::AdminService::Instance().GetAllRoles(request->token());
    for (const auto& item : list) {
        auto* role = response->add_roles();
        role->set_id(item.id);
        role->set_name(item.name);
        role->set_display_name(item.display_name);
        role->set_description(item.description);
        role->set_color(item.color);
        for (auto p : item.permissions) {
            role->add_permissions(p);
        }
        role->set_level(item.level);
        role->set_is_default(item.is_default);
        role->set_created_at(item.created_at);
    }
    return trpc::kSuccStatus;
}

trpc::Status AdminController::CreateRole(::trpc::ServerContextPtr,
                                          const ::furbbs::CreateRoleRequest* request,
                                          ::furbbs::CreateRoleResponse* response) {
    repository::RoleEntity role;
    role.name = request->role().name();
    role.display_name = request->role().display_name();
    role.description = request->role().description();
    role.color = request->role().color();
    for (auto p : request->role().permissions()) {
        role.permissions.push_back(p);
    }
    role.level = request->role().level();
    role.is_default = request->role().is_default();

    bool success = service::AdminService::Instance().CreateRole(request->token(), role);
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::UpdateRole(::trpc::ServerContextPtr,
                                          const ::furbbs::UpdateRoleRequest* request,
                                          ::furbbs::UpdateRoleResponse* response) {
    repository::RoleEntity role;
    role.id = request->role().id();
    role.name = request->role().name();
    role.display_name = request->role().display_name();
    role.description = request->role().description();
    role.color = request->role().color();
    for (auto p : request->role().permissions()) {
        role.permissions.push_back(p);
    }
    role.level = request->role().level();
    role.is_default = request->role().is_default();

    bool success = service::AdminService::Instance().UpdateRole(request->token(), role);
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::DeleteRole(::trpc::ServerContextPtr,
                                          const ::furbbs::DeleteRoleRequest* request,
                                          ::furbbs::DeleteRoleResponse* response) {
    bool success = service::AdminService::Instance().DeleteRole(request->token(), request->role_id());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::AssignRole(::trpc::ServerContextPtr,
                                          const ::furbbs::AssignRoleRequest* request,
                                          ::furbbs::AssignRoleResponse* response) {
    bool success = service::AdminService::Instance().AssignRole(
        request->token(), request->target_user_id(), request->role_id());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::GetAuditLogs(::trpc::ServerContextPtr,
                                            const ::furbbs::GetAuditLogsRequest* request,
                                            ::furbbs::GetAuditLogsResponse* response) {
    int total = 0;
    auto list = service::AdminService::Instance().GetAuditLogs(
        request->token(), request->operator_id(), request->start_time(),
        request->end_time(), request->action(), request->page(),
        request->page_size(), total);

    for (const auto& item : list) {
        auto* log = response->add_logs();
        log->set_id(item.id);
        log->set_operator_id(item.operator_id);
        log->set_operator_name(item.operator_name);
        log->set_action(item.action);
        log->set_resource_type(item.resource_type);
        log->set_resource_id(item.resource_id);
        log->set_ip_address(item.ip_address);
        log->set_details(item.details);
        log->set_success(item.success);
        log->set_created_at(item.created_at);
    }
    response->set_total(total);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::CreateReport(::trpc::ServerContextPtr,
                                            const ::furbbs::CreateReportRequest* request,
                                            ::furbbs::CreateReportResponse* response) {
    std::vector<std::string> evidence;
    for (const auto& e : request->evidence()) {
        evidence.push_back(e);
    }

    auto report_id = service::AdminService::Instance().CreateReport(
        request->token(), request->type(), request->target_type(),
        request->target_id(), request->reason(), request->description(), evidence);

    response->set_report_id(report_id);
    response->set_success(report_id > 0);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::GetReports(::trpc::ServerContextPtr,
                                          const ::furbbs::GetReportsRequest* request,
                                          ::furbbs::GetReportsResponse* response) {
    int total = 0;
    auto list = service::AdminService::Instance().GetReports(
        request->token(), request->status(), request->target_type(),
        request->page(), request->page_size(), total);

    for (const auto& item : list) {
        auto* report = response->add_reports();
        report->set_id(item.id);
        report->set_reporter_id(item.reporter_id);
        report->set_reporter_name(item.reporter_name);
        report->set_type(item.type);
        report->set_target_type(item.target_type);
        report->set_target_id(item.target_id);
        report->set_target_title(item.target_title);
        report->set_target_author_id(item.target_author_id);
        report->set_reason(item.reason);
        report->set_description(item.description);
        for (const auto& e : item.evidence) {
            report->add_evidence(e);
        }
        report->set_status(item.status);
        report->set_processor_id(item.processor_id);
        report->set_processor_name(item.processor_name);
        report->set_result(item.result);
        report->set_created_at(item.created_at);
        report->set_processed_at(item.processed_at);
    }
    response->set_total(total);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::ProcessReport(::trpc::ServerContextPtr,
                                             const ::furbbs::ProcessReportRequest* request,
                                             ::furbbs::ProcessReportResponse* response) {
    bool success = service::AdminService::Instance().ProcessReport(
        request->token(), request->report_id(), request->new_status(), request->result());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::ManageUserBan(::trpc::ServerContextPtr,
                                              const ::furbbs::ManageUserBanRequest* request,
                                              ::furbbs::ManageUserBanResponse* response) {
    bool success = service::AdminService::Instance().ManageUserBan(
        request->token(), request->target_user_id(), request->duration(),
        request->reason(), request->is_ban());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::RemoveContent(::trpc::ServerContextPtr,
                                             const ::furbbs::RemoveContentRequest* request,
                                             ::furbbs::RemoveContentResponse* response) {
    bool success = service::AdminService::Instance().RemoveContent(
        request->token(), request->target_type(), request->target_id(), request->reason());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::PinContent(::trpc::ServerContextPtr,
                                          const ::furbbs::PinContentRequest* request,
                                          ::furbbs::PinContentResponse* response) {
    bool success = service::AdminService::Instance().PinContent(
        request->token(), request->target_type(), request->target_id(), request->is_pin());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::LockContent(::trpc::ServerContextPtr,
                                           const ::furbbs::LockContentRequest* request,
                                           ::furbbs::LockContentResponse* response) {
    bool success = service::AdminService::Instance().LockContent(
        request->token(), request->post_id(), request->is_lock());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status AdminController::GetAdminUsers(::trpc::ServerContextPtr,
                                              const ::furbbs::GetAdminUsersRequest* request,
                                              ::furbbs::GetAdminUsersResponse* response) {
    int total = 0;
    auto list = service::AdminService::Instance().GetAdminUsers(
        request->token(), request->page(), request->page_size(), total);

    for (const auto& item : list) {
        auto* admin = response->add_admins();
        admin->set_user_id(item.user_id);
        admin->set_username(item.username);
        admin->set_avatar(item.avatar);
        admin->set_role_id(item.role_id);
        admin->set_role_name(item.role_name);
        admin->set_role_level(item.role_level);
        admin->set_is_active(item.is_active);
        admin->set_last_login_ip(item.last_login_ip);
        admin->set_last_login_at(item.last_login_at);
        admin->set_created_at(item.created_at);
    }
    response->set_total(total);
    return trpc::kSuccStatus;
}

} // namespace furbbs::controller
