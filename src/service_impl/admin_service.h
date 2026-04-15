#ifndef FURBBS_SERVICE_IMPL_ADMIN_SERVICE_H
#define FURBBS_SERVICE_IMPL_ADMIN_SERVICE_H

#include "repository/admin_repository.h"
#include "repository/user_repository.h"
#include "repository/post_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>
#include <functional>

namespace furbbs::service {

class AdminService {
public:
    static AdminService& Instance() {
        static AdminService instance;
        return instance;
    }

    bool CheckPermission(const std::string& token, repository::Permission permission);

    template<typename T>
    T WithPermission(const std::string& token, repository::Permission permission,
                     std::function<T()> action, const std::string& resource_type,
                     const std::string& resource_id, const std::string& ip = "") {
        if (!CheckPermission(token, permission)) {
            LogAction(token, "PERMISSION_DENIED", resource_type, resource_id, ip, "", false);
            return T{};
        }
        T result = action();
        LogAction(token, permission_to_string(permission), resource_type, resource_id, ip, "", true);
        return result;
    }

    std::vector<repository::RoleEntity> GetAllRoles(const std::string& token);

    bool CreateRole(const std::string& token, const repository::RoleEntity& role);

    bool UpdateRole(const std::string& token, const repository::RoleEntity& role);

    bool DeleteRole(const std::string& token, int32_t role_id);

    bool AssignRole(const std::string& token, const std::string& target_user_id, int32_t role_id);

    void LogAction(const std::string& token, const std::string& action,
                   const std::string& resource_type, const std::string& resource_id,
                   const std::string& ip, const std::string& details, bool success);

    std::vector<repository::AuditLogEntity> GetAuditLogs(const std::string& token,
                                                         const std::string& operator_id,
                                                         int64_t start_time, int64_t end_time,
                                                         const std::string& action,
                                                         int page, int page_size, int& out_total);

    int64_t CreateReport(const std::string& token, int32_t type,
                         const std::string& target_type, const std::string& target_id,
                         const std::string& reason, const std::string& description,
                         const std::vector<std::string>& evidence);

    std::vector<repository::ReportEntity> GetReports(const std::string& token, int32_t status,
                                                      const std::string& target_type,
                                                      int page, int page_size, int& out_total);

    bool ProcessReport(const std::string& token, int64_t report_id,
                       int32_t new_status, const std::string& result);

    bool ManageUserBan(const std::string& token, const std::string& target_user_id,
                        int64_t duration, const std::string& reason, bool ban);

    bool RemoveContent(const std::string& token, const std::string& target_type,
                        int64_t target_id, const std::string& reason);

    bool PinContent(const std::string& token, const std::string& target_type,
                     int64_t target_id, bool pin);

    bool LockContent(const std::string& token, int64_t post_id, bool lock);

    std::vector<repository::AdminUserEntity> GetAdminUsers(const std::string& token,
                                                            int page, int page_size, int& out_total);

private:
    AdminService() = default;
    static std::string permission_to_string(repository::Permission p);
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_ADMIN_SERVICE_H
