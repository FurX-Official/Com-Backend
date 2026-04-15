#ifndef FURBBS_REPOSITORY_ADMIN_REPOSITORY_H
#define FURBBS_REPOSITORY_ADMIN_REPOSITORY_H

#include "base_repository.h"
#include "db/sql_queries.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::repository {

enum class Permission {
    USER_VIEW = 1,
    USER_EDIT = 2,
    USER_DELETE = 3,
    USER_BAN = 4,
    
    POST_VIEW = 10,
    POST_EDIT = 11,
    POST_DELETE = 12,
    POST_PIN = 13,
    POST_LOCK = 14,
    
    COMMENT_MODERATE = 20,
    
    GALLERY_MODERATE = 30,
    
    REPORT_VIEW = 40,
    REPORT_PROCESS = 41,
    
    PERMISSION_MANAGE = 50,
    ROLE_MANAGE = 51,
    
    AUDIT_LOG_VIEW = 60,
    
    SYSTEM_CONFIG = 70,
    
    ANNOUNCEMENT_MANAGE = 80,
    
    ECONOMY_MANAGE = 90,
    
    SUPER_ADMIN = 999
};

struct RoleEntity {
    int32_t id = 0;
    std::string name;
    std::string display_name;
    std::string description;
    std::string color;
    std::vector<int32_t> permissions;
    int32_t level = 0;
    bool is_default = false;
    int64_t created_at = 0;
};

struct AdminUserEntity {
    std::string user_id;
    std::string username;
    std::string avatar;
    int32_t role_id = 0;
    std::string role_name;
    int32_t role_level = 0;
    bool is_active = true;
    std::string last_login_ip;
    int64_t last_login_at = 0;
    int64_t created_at = 0;
};

struct AuditLogEntity {
    int64_t id = 0;
    std::string operator_id;
    std::string operator_name;
    std::string action;
    std::string resource_type;
    std::string resource_id;
    std::string ip_address;
    std::string user_agent;
    std::string details;
    bool success = true;
    int64_t created_at = 0;
};

struct ReportEntity {
    int64_t id = 0;
    std::string reporter_id;
    std::string reporter_name;
    int32_t type = 0;
    std::string target_type;
    std::string target_id;
    std::string target_title;
    std::string target_author_id;
    std::string reason;
    std::string description;
    std::vector<std::string> evidence;
    int32_t status = 0;
    std::string processor_id;
    std::string processor_name;
    std::string result;
    int64_t created_at = 0;
    int64_t processed_at = 0;
};

class AdminRepository : protected BaseRepository {
public:
    static AdminRepository& Instance() {
        static AdminRepository instance;
        return instance;
    }

    bool HasPermission(const std::string& user_id, Permission permission);

    std::optional<RoleEntity> GetUserRole(const std::string& user_id);

    std::vector<RoleEntity> GetAllRoles();

    bool CreateRole(const RoleEntity& role);

    bool UpdateRole(const RoleEntity& role);

    bool DeleteRole(int32_t role_id);

    bool AssignRole(const std::string& user_id, int32_t role_id);

    void LogAudit(const std::string& operator_id, const std::string& action,
                  const std::string& resource_type, const std::string& resource_id,
                  const std::string& ip, const std::string& details, bool success);

    std::vector<AuditLogEntity> GetAuditLogs(const std::string& operator_id,
                                              int64_t start_time, int64_t end_time,
                                              const std::string& action, int limit, int offset,
                                              int& out_total);

    int64_t CreateReport(const std::string& reporter_id, int32_t type,
                         const std::string& target_type, const std::string& target_id,
                         const std::string& reason, const std::string& description,
                         const std::vector<std::string>& evidence);

    std::vector<ReportEntity> GetReports(int32_t status, const std::string& target_type,
                                         int limit, int offset, int& out_total);

    bool ProcessReport(int64_t report_id, const std::string& processor_id,
                       int32_t new_status, const std::string& result);

    std::vector<AdminUserEntity> GetAdminUsers(int limit, int offset, int& out_total);

private:
    AdminRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_ADMIN_REPOSITORY_H
