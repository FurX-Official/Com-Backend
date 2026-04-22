#include "../service_impl/admin_service.h"
#include "../common/result.h"

namespace furbbs::controller {

using ::trpc::ServerContextPtr;
using namespace furbbs::repository;

::trpc::Status GrantPermission(ServerContextPtr context,
                                const ::furbbs::GrantPermissionRequest* request,
                                ::furbbs::GrantPermissionResponse* response) {
    try {
        bool success = service::AdminService::Instance().AssignRole(
            request->access_token(), request->user_id(), request->role_id());
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "权限授予成功" : "权限授予失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status RevokePermission(ServerContextPtr context,
                                 const ::furbbs::RevokePermissionRequest* request,
                                 ::furbbs::RevokePermissionResponse* response) {
    try {
        bool success = service::AdminService::Instance().ManageUserBan(
            request->access_token(), request->user_id(), 0, "", false);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "权限撤销成功" : "权限撤销失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status ListUserPermissions(ServerContextPtr context,
                                    const ::furbbs::ListUserPermissionsRequest* request,
                                    ::furbbs::ListUserPermissionsResponse* response) {
    try {
        auto roles = service::AdminService::Instance().GetUserRoles(
            request->access_token(), request->user_id());
        for (const auto& item : roles) {
            auto* role = response->add_roles();
            role->set_id(item.id);
            role->set_name(item.name);
            role->set_display_name(item.display_name);
            role->set_level(item.level);
        }
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status ModeratePost(ServerContextPtr context,
                             const ::furbbs::ModeratePostRequest* request,
                             ::furbbs::ModeratePostResponse* response) {
    try {
        bool success = false;
        if (request->action() == "remove") {
            success = service::AdminService::Instance().RemoveContent(
                request->access_token(), "post", request->post_id(), request->reason());
        } else if (request->action() == "lock") {
            success = service::AdminService::Instance().LockContent(
                request->access_token(), request->post_id(), true);
        } else if (request->action() == "unlock") {
            success = service::AdminService::Instance().LockContent(
                request->access_token(), request->post_id(), false);
        } else if (request->action() == "pin") {
            success = service::AdminService::Instance().PinContent(
                request->access_token(), "post", request->post_id(), true);
        } else if (request->action() == "unpin") {
            success = service::AdminService::Instance().PinContent(
                request->access_token(), "post", request->post_id(), false);
        }
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "操作成功" : "操作失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status BanUser(ServerContextPtr context,
                        const ::furbbs::BanUserRequest* request,
                        ::furbbs::BanUserResponse* response) {
    try {
        bool success = service::AdminService::Instance().ManageUserBan(
            request->access_token(), request->user_id(),
            request->duration(), request->reason(), true);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "封禁成功" : "封禁失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status UnbanUser(ServerContextPtr context,
                          const ::furbbs::UnbanUserRequest* request,
                          ::furbbs::UnbanUserResponse* response) {
    try {
        bool success = service::AdminService::Instance().ManageUserBan(
            request->access_token(), request->user_id(), 0, "", false);
        response->set_code(success ? RESULT_OK : RESULT_ERROR);
        response->set_message(success ? "解封成功" : "解封失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetUserBan(ServerContextPtr context,
                           const ::furbbs::GetUserBanRequest* request,
                           ::furbbs::GetUserBanResponse* response) {
    try {
        auto ban = service::AdminService::Instance().GetUserBanInfo(
            request->user_id());
        response->set_is_banned(ban.is_banned);
        response->set_reason(ban.reason);
        response->set_banned_by(ban.banned_by);
        response->set_banned_at(ban.banned_at);
        response->set_expires_at(ban.expires_at);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetModeratorLogs(ServerContextPtr context,
                                 const ::furbbs::GetModeratorLogsRequest* request,
                                 ::furbbs::GetModeratorLogsResponse* response) {
    try {
        int total = 0;
        auto logs = service::AdminService::Instance().GetAuditLogs(
            request->access_token(), "", 0, 0, "",
            request->page(), request->page_size(), total);
        for (const auto& item : logs) {
            auto* log = response->add_logs();
            log->set_id(item.id);
            log->set_operator_id(item.operator_id);
            log->set_action(item.action);
            log->set_details(item.details);
            log->set_created_at(item.created_at);
        }
        response->set_total(total);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status CreateReport(ServerContextPtr context,
                             const ::furbbs::CreateReportRequest* request,
                             ::furbbs::CreateReportResponse* response) {
    try {
        std::vector<std::string> evidence;
        for (int i = 0; i < request->evidence_size(); i++) {
            evidence.push_back(request->evidence(i));
        }
        int64_t report_id = service::AdminService::Instance().CreateReport(
            request->access_token(), request->type(),
            request->target_type(), request->target_id(),
            request->reason(), request->description(), evidence);
        response->set_report_id(report_id);
        response->set_code(report_id > 0 ? RESULT_OK : RESULT_ERROR);
        response->set_message(report_id > 0 ? "举报成功" : "举报失败");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetReports(ServerContextPtr context,
                           const ::furbbs::GetReportsRequest* request,
                           ::furbbs::GetReportsResponse* response) {
    try {
        int total = 0;
        auto reports = service::AdminService::Instance().GetReports(
            request->access_token(), request->status(),
            request->target_type(), request->page(), request->page_size(), total);
        for (const auto& item : reports) {
            auto* report = response->add_reports();
            report->set_id(item.id);
            report->set_reporter_id(item.reporter_id);
            report->set_type(item.type);
            report->set_target_type(item.target_type);
            report->set_target_id(item.target_id);
            report->set_status(item.status);
            report->set_created_at(item.created_at);
        }
        response->set_total(total);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

} // namespace furbbs::controller
