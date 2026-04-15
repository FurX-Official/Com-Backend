#include "admin_repository.h"
#include <nlohmann/json.hpp>

namespace furbbs::repository {

bool AdminRepository::HasPermission(const std::string& user_id, Permission permission) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::ADMIN_GET_USER_PERMISSIONS, user_id);
        if (result.empty()) return false;

        auto perm_array = result[0][0].as<std::string>();
        auto perms = nlohmann::json::parse(perm_array);
        int32_t perm_code = static_cast<int32_t>(permission);

        for (const auto& p : perms) {
            if (p == perm_code || p == 999) {
                return true;
            }
        }
        return false;
    });
}

std::optional<RoleEntity> AdminRepository::GetUserRole(const std::string& user_id) {
    return Execute<std::optional<RoleEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::ADMIN_GET_USER_ROLE, user_id);
        if (result.empty()) return std::nullopt;

        RoleEntity role;
        role.id = result[0][0].as<int32_t>();
        role.name = result[0][1].as<std::string>();
        role.display_name = result[0][2].as<std::string>();
        role.description = result[0][3].as<std::string>();
        role.color = result[0][4].as<std::string>();
        
        auto perm_array = result[0][5].as<std::string>();
        auto perms = nlohmann::json::parse(perm_array);
        for (const auto& p : perms) {
            role.permissions.push_back(p);
        }
        
        role.level = result[0][6].as<int32_t>();
        role.is_default = result[0][7].as<bool>();
        role.created_at = result[0][8].as<int64_t>();
        return role;
    });
}

std::vector<RoleEntity> AdminRepository::GetAllRoles() {
    return Execute<std::vector<RoleEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::ADMIN_GET_ALL_ROLES);
        std::vector<RoleEntity> list;
        for (const auto& row : result) {
            RoleEntity role;
            role.id = row[0].as<int32_t>();
            role.name = row[1].as<std::string>();
            role.display_name = row[2].as<std::string>();
            role.description = row[3].as<std::string>();
            role.color = row[4].as<std::string>();
            
            auto perm_array = row[5].as<std::string>();
            auto perms = nlohmann::json::parse(perm_array);
            for (const auto& p : perms) {
                role.permissions.push_back(p);
            }
            
            role.level = row[6].as<int32_t>();
            role.is_default = row[7].as<bool>();
            role.created_at = row[8].as<int64_t>();
            list.push_back(role);
        }
        return list;
    });
}

bool AdminRepository::CreateRole(const RoleEntity& role) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        nlohmann::json perm_json = role.permissions;
        auto result = txn.exec_params(sql::ADMIN_CREATE_ROLE, role.name, role.display_name,
                                      role.description, role.color, perm_json.dump(),
                                      role.level, role.is_default, timestamp);
        return !result.empty();
    });
}

bool AdminRepository::UpdateRole(const RoleEntity& role) {
    return Execute<bool>([&](pqxx::work& txn) {
        nlohmann::json perm_json = role.permissions;
        auto result = txn.exec_params(sql::ADMIN_UPDATE_ROLE, role.name, role.display_name,
                                      role.description, role.color, perm_json.dump(),
                                      role.level, role.is_default, role.id);
        return result.affected_rows() > 0;
    });
}

bool AdminRepository::DeleteRole(int32_t role_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::ADMIN_DELETE_ROLE, role_id);
        return result.affected_rows() > 0;
    });
}

bool AdminRepository::AssignRole(const std::string& user_id, int32_t role_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        auto result = txn.exec_params(sql::ADMIN_ASSIGN_ROLE, user_id, role_id, timestamp);
        return result.affected_rows() > 0;
    });
}

void AdminRepository::LogAudit(const std::string& operator_id, const std::string& action,
                                const std::string& resource_type, const std::string& resource_id,
                                const std::string& ip, const std::string& details, bool success) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        std::string operator_name = "";
        auto user_result = txn.exec_params(R"(SELECT username FROM users WHERE id = $1)", operator_id);
        if (!user_result.empty()) {
            operator_name = user_result[0][0].as<std::string>();
        }
        
        txn.exec_params(sql::ADMIN_INSERT_AUDIT_LOG, operator_id, operator_name, action,
                        resource_type, resource_id, ip, details, success, timestamp);
    });
}

std::vector<AuditLogEntity> AdminRepository::GetAuditLogs(const std::string& operator_id,
                                                           int64_t start_time, int64_t end_time,
                                                           const std::string& action, int limit, int offset,
                                                           int& out_total) {
    return Execute<std::vector<AuditLogEntity>>([&](pqxx::work& txn) {
        auto count_result = txn.exec_params(sql::ADMIN_COUNT_AUDIT_LOGS,
                                            operator_id, start_time, end_time, action);
        out_total = count_result[0][0].as<int>();

        auto result = txn.exec_params(sql::ADMIN_GET_AUDIT_LOGS,
                                      operator_id, start_time, end_time, action, limit, offset);
        std::vector<AuditLogEntity> list;
        for (const auto& row : result) {
            AuditLogEntity e;
            e.id = row[0].as<int64_t>();
            e.operator_id = row[1].as<std::string>();
            e.operator_name = row[2].as<std::string>();
            e.action = row[3].as<std::string>();
            e.resource_type = row[4].as<std::string>();
            e.resource_id = row[5].as<std::string>();
            e.ip_address = row[6].as<std::string>();
            e.details = row[7].as<std::string>();
            e.success = row[8].as<bool>();
            e.created_at = row[9].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

int64_t AdminRepository::CreateReport(const std::string& reporter_id, int32_t type,
                                       const std::string& target_type, const std::string& target_id,
                                       const std::string& reason, const std::string& description,
                                       const std::vector<std::string>& evidence) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        nlohmann::json evidence_json = evidence;
        auto result = txn.exec_params(sql::ADMIN_CREATE_REPORT, reporter_id, type,
                                      target_type, target_id, reason, description,
                                      evidence_json.dump(), timestamp);
        return result[0][0].as<int64_t>();
    });
}

std::vector<ReportEntity> AdminRepository::GetReports(int32_t status, const std::string& target_type,
                                                       int limit, int offset, int& out_total) {
    return Execute<std::vector<ReportEntity>>([&](pqxx::work& txn) {
        auto count_result = txn.exec_params(sql::ADMIN_COUNT_REPORTS, status, target_type);
        out_total = count_result[0][0].as<int>();

        auto result = txn.exec_params(sql::ADMIN_GET_REPORTS, status, target_type, limit, offset);
        std::vector<ReportEntity> list;
        for (const auto& row : result) {
            ReportEntity e;
            e.id = row[0].as<int64_t>();
            e.reporter_id = row[1].as<std::string>();
            e.reporter_name = row[2].as<std::string>();
            e.type = row[3].as<int32_t>();
            e.target_type = row[4].as<std::string>();
            e.target_id = row[5].as<std::string>();
            e.target_title = row[6].as<std::string>();
            e.target_author_id = row[7].as<std::string>();
            e.reason = row[8].as<std::string>();
            e.description = row[9].as<std::string>();
            
            if (!row[10].is_null()) {
                auto evidence_json = nlohmann::json::parse(row[10].as<std::string>());
                for (const auto& ev : evidence_json) {
                    e.evidence.push_back(ev);
                }
            }
            
            e.status = row[11].as<int32_t>();
            if (!row[12].is_null()) e.processor_id = row[12].as<std::string>();
            if (!row[13].is_null()) e.processor_name = row[13].as<std::string>();
            if (!row[14].is_null()) e.result = row[14].as<std::string>();
            e.created_at = row[15].as<int64_t>();
            if (!row[16].is_null()) e.processed_at = row[16].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

bool AdminRepository::ProcessReport(int64_t report_id, const std::string& processor_id,
                                     int32_t new_status, const std::string& result) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        auto exec_result = txn.exec_params(sql::ADMIN_PROCESS_REPORT,
                                            new_status, processor_id, result, timestamp, report_id);
        return exec_result.affected_rows() > 0;
    });
}

std::vector<AdminUserEntity> AdminRepository::GetAdminUsers(int limit, int offset, int& out_total) {
    return Execute<std::vector<AdminUserEntity>>([&](pqxx::work& txn) {
        auto count_result = txn.exec(sql::ADMIN_COUNT_ADMIN_USERS);
        out_total = count_result[0][0].as<int>();

        auto result = txn.exec_params(sql::ADMIN_GET_ADMIN_USERS, limit, offset);
        std::vector<AdminUserEntity> list;
        for (const auto& row : result) {
            AdminUserEntity e;
            e.user_id = row[0].as<std::string>();
            e.username = row[1].as<std::string>();
            e.avatar = row[2].as<std::string>();
            e.role_id = row[3].as<int32_t>();
            e.role_name = row[4].as<std::string>();
            e.role_level = row[5].as<int32_t>();
            e.is_active = row[6].as<bool>();
            if (!row[7].is_null()) e.last_login_ip = row[7].as<std::string>();
            if (!row[8].is_null()) e.last_login_at = row[8].as<int64_t>();
            e.created_at = row[9].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

} // namespace furbbs::repository
