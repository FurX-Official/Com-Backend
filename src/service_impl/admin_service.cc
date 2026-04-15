#include "admin_service.h"
#include "service_impl/punishment_service.h"

namespace furbbs::service {

std::string AdminService::permission_to_string(repository::Permission p) {
    switch (p) {
        case repository::Permission::USER_VIEW: return "USER_VIEW";
        case repository::Permission::USER_EDIT: return "USER_EDIT";
        case repository::Permission::USER_DELETE: return "USER_DELETE";
        case repository::Permission::USER_BAN: return "USER_BAN";
        case repository::Permission::POST_VIEW: return "POST_VIEW";
        case repository::Permission::POST_EDIT: return "POST_EDIT";
        case repository::Permission::POST_DELETE: return "POST_DELETE";
        case repository::Permission::POST_PIN: return "POST_PIN";
        case repository::Permission::POST_LOCK: return "POST_LOCK";
        case repository::Permission::COMMENT_MODERATE: return "COMMENT_MODERATE";
        case repository::Permission::GALLERY_MODERATE: return "GALLERY_MODERATE";
        case repository::Permission::REPORT_VIEW: return "REPORT_VIEW";
        case repository::Permission::REPORT_PROCESS: return "REPORT_PROCESS";
        case repository::Permission::PERMISSION_MANAGE: return "PERMISSION_MANAGE";
        case repository::Permission::ROLE_MANAGE: return "ROLE_MANAGE";
        case repository::Permission::AUDIT_LOG_VIEW: return "AUDIT_LOG_VIEW";
        case repository::Permission::SYSTEM_CONFIG: return "SYSTEM_CONFIG";
        case repository::Permission::ANNOUNCEMENT_MANAGE: return "ANNOUNCEMENT_MANAGE";
        case repository::Permission::ECONOMY_MANAGE: return "ECONOMY_MANAGE";
        case repository::Permission::SUPER_ADMIN: return "SUPER_ADMIN";
        default: return "UNKNOWN";
    }
}

bool AdminService::CheckPermission(const std::string& token, repository::Permission permission) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return false;
    return repository::AdminRepository::Instance().HasPermission(*user_id, permission);
}

std::vector<repository::RoleEntity> AdminService::GetAllRoles(const std::string& token) {
    if (!CheckPermission(token, repository::Permission::ROLE_MANAGE)) return {};
    return repository::AdminRepository::Instance().GetAllRoles();
}

bool AdminService::CreateRole(const std::string& token, const repository::RoleEntity& role) {
    if (!CheckPermission(token, repository::Permission::ROLE_MANAGE)) return false;
    return repository::AdminRepository::Instance().CreateRole(role);
}

bool AdminService::UpdateRole(const std::string& token, const repository::RoleEntity& role) {
    if (!CheckPermission(token, repository::Permission::ROLE_MANAGE)) return false;
    return repository::AdminRepository::Instance().UpdateRole(role);
}

bool AdminService::DeleteRole(const std::string& token, int32_t role_id) {
    if (!CheckPermission(token, repository::Permission::ROLE_MANAGE)) return false;
    return repository::AdminRepository::Instance().DeleteRole(role_id);
}

bool AdminService::AssignRole(const std::string& token, const std::string& target_user_id, int32_t role_id) {
    if (!CheckPermission(token, repository::Permission::PERMISSION_MANAGE)) return false;
    return repository::AdminRepository::Instance().AssignRole(target_user_id, role_id);
}

void AdminService::LogAction(const std::string& token, const std::string& action,
                              const std::string& resource_type, const std::string& resource_id,
                              const std::string& ip, const std::string& details, bool success) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return;
    repository::AdminRepository::Instance().LogAudit(*user_id, action, resource_type,
                                                      resource_id, ip, details, success);
}

std::vector<repository::AuditLogEntity> AdminService::GetAuditLogs(const std::string& token,
                                                                     const std::string& operator_id,
                                                                     int64_t start_time, int64_t end_time,
                                                                     const std::string& action,
                                                                     int page, int page_size, int& out_total) {
    if (!CheckPermission(token, repository::Permission::AUDIT_LOG_VIEW)) {
        out_total = 0;
        return {};
    }
    return repository::AdminRepository::Instance().GetAuditLogs(operator_id, start_time, end_time,
                                                                 action, page_size, (page - 1) * page_size, out_total);
}

int64_t AdminService::CreateReport(const std::string& token, int32_t type,
                                    const std::string& target_type, const std::string& target_id,
                                    const std::string& reason, const std::string& description,
                                    const std::vector<std::string>& evidence) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return 0;
    return repository::AdminRepository::Instance().CreateReport(*user_id, type, target_type,
                                                                 target_id, reason, description, evidence);
}

std::vector<repository::ReportEntity> AdminService::GetReports(const std::string& token, int32_t status,
                                                                const std::string& target_type,
                                                                int page, int page_size, int& out_total) {
    if (!CheckPermission(token, repository::Permission::REPORT_VIEW)) {
        out_total = 0;
        return {};
    }
    return repository::AdminRepository::Instance().GetReports(status, target_type, page_size,
                                                              (page - 1) * page_size, out_total);
}

bool AdminService::ProcessReport(const std::string& token, int64_t report_id,
                                  int32_t new_status, const std::string& result) {
    if (!CheckPermission(token, repository::Permission::REPORT_PROCESS)) return false;
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return false;
    return repository::AdminRepository::Instance().ProcessReport(report_id, *user_id, new_status, result);
}

bool AdminService::ManageUserBan(const std::string& token, const std::string& target_user_id,
                                   int64_t duration, const std::string& reason, bool ban) {
    if (!CheckPermission(token, repository::Permission::USER_BAN)) return false;
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return false;

    if (ban) {
        return PunishmentService::Instance().BanUser(target_user_id, duration, reason, *user_id);
    } else {
        return PunishmentService::Instance().UnbanUser(target_user_id, *user_id);
    }
}

bool AdminService::RemoveContent(const std::string& token, const std::string& target_type,
                                  int64_t target_id, const std::string& reason) {
    if (target_type == "post") {
        if (!CheckPermission(token, repository::Permission::POST_DELETE)) return false;
    } else if (target_type == "comment") {
        if (!CheckPermission(token, repository::Permission::COMMENT_MODERATE)) return false;
    } else if (target_type == "gallery") {
        if (!CheckPermission(token, repository::Permission::GALLERY_MODERATE)) return false;
    } else {
        return false;
    }
    LogAction(token, "REMOVE_CONTENT", target_type, std::to_string(target_id), "", reason, true);
    return true;
}

bool AdminService::PinContent(const std::string& token, const std::string& target_type,
                               int64_t target_id, bool pin) {
    if (target_type != "post") return false;
    if (!CheckPermission(token, repository::Permission::POST_PIN)) return false;
    LogAction(token, pin ? "PIN_CONTENT" : "UNPIN_CONTENT", target_type, std::to_string(target_id), "", "", true);
    return true;
}

bool AdminService::LockContent(const std::string& token, int64_t post_id, bool lock) {
    if (!CheckPermission(token, repository::Permission::POST_LOCK)) return false;
    LogAction(token, lock ? "LOCK_CONTENT" : "UNLOCK_CONTENT", "post", std::to_string(post_id), "", "", true);
    return true;
}

std::vector<repository::AdminUserEntity> AdminService::GetAdminUsers(const std::string& token,
                                                                      int page, int page_size, int& out_total) {
    if (!CheckPermission(token, repository::Permission::SUPER_ADMIN)) {
        out_total = 0;
        return {};
    }
    return repository::AdminRepository::Instance().GetAdminUsers(page_size, (page - 1) * page_size, out_total);
}

} // namespace furbbs::service
