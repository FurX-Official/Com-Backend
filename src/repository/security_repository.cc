#include "security_repository.h"
#include "../db/database.h"
#include "../db/sql_queries.h"
#include <pqxx/pqxx>

namespace furbbs::repository {

bool SecurityRepository::AddIpToBlacklist(const IpBlacklistEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::IP_BLACKLIST_ADD,
            data.ip_address, data.reason, data.banned_by, data.expires_at);
        return true;
    });
}

bool SecurityRepository::IsIpBlacklisted(const std::string& ip_address) {
    return ExecuteQueryOne<bool>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::IP_BLACKLIST_CHECK, ip_address);
        return !r.empty();
    }).value_or(false);
}

bool SecurityRepository::RemoveIpFromBlacklist(const std::string& ip_address) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::IP_BLACKLIST_REMOVE, ip_address);
        return r.affected_rows() > 0;
    });
}

int32_t SecurityRepository::IncrementRateLimit(const std::string& identifier,
        const std::string& limit_type, int64_t window_start) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::RATE_LIMIT_INC,
            identifier, limit_type, window_start);
        return r[0][0].as<int32_t>();
    });
}

bool SecurityRepository::ResetRateLimit(const std::string& identifier, const std::string& limit_type) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::RATE_LIMIT_RESET, identifier, limit_type);
        return true;
    });
}

bool SecurityRepository::CreateSession(const UserSessionEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SESSION_CREATE,
            data.session_id, data.user_id, "{}",
            data.ip_address, data.location, data.user_agent);
        return true;
    });
}

std::vector<UserSessionEntity> SecurityRepository::GetUserSessions(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::SESSION_GET_BY_USER, user_id);
        return MapResults<UserSessionEntity>(r, [](const pqxx::row& row, UserSessionEntity& e) {
            e.session_id = row["session_id"].as<std::string>();
            e.ip_address = row["ip_address"].as<std::string>();
            e.location = row["location"].as<std::string>();
            e.user_agent = row["user_agent"].as<std::string>();
            e.last_active = row["last_active"].as<int64_t>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool SecurityRepository::RevokeSession(const std::string& session_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::SESSION_REVOKE, session_id, user_id);
        return r.affected_rows() > 0;
    });
}

bool SecurityRepository::UpdateSessionActivity(const std::string& session_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::SESSION_UPDATE, session_id);
        return true;
    });
}

bool SecurityRepository::AddLoginHistory(const LoginHistoryEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::LOGIN_HISTORY_ADD,
            data.user_id, data.ip_address, data.location,
            "{}", data.user_agent, data.was_successful, data.failure_reason);
        return true;
    });
}

std::vector<LoginHistoryEntity> SecurityRepository::GetLoginHistory(
        const std::string& user_id, int limit, int offset) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::LOGIN_HISTORY_GET, user_id, limit, offset);
        return MapResults<LoginHistoryEntity>(r, [](const pqxx::row& row, LoginHistoryEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.ip_address = row["ip_address"].as<std::string>();
            e.location = row["location"].as<std::string>();
            e.user_agent = row["user_agent"].as<std::string>();
            e.was_successful = row["was_successful"].as<bool>();
            e.failure_reason = row["failure_reason"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool SecurityRepository::BlockUser(const UserBlockEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::USER_BLOCK_ADD,
            data.blocker_id, data.blocked_id, data.block_type, data.reason);
        return true;
    });
}

bool SecurityRepository::UnblockUser(const std::string& blocker_id, const std::string& blocked_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::USER_BLOCK_REMOVE, blocker_id, blocked_id);
        return r.affected_rows() > 0;
    });
}

bool SecurityRepository::IsUserBlocked(const std::string& blocker_id, const std::string& blocked_id) {
    return ExecuteQueryOne<bool>([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::USER_BLOCK_CHECK, blocker_id, blocked_id);
        return !r.empty();
    }).value_or(false);
}

std::vector<UserBlockEntity> SecurityRepository::GetBlockedUsers(const std::string& blocker_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::USER_BLOCK_LIST, blocker_id);
        return MapResults<UserBlockEntity>(r, [](const pqxx::row& row, UserBlockEntity& e) {
            e.blocked_id = row["blocked_id"].as<std::string>();
            e.block_type = row["block_type"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

int64_t SecurityRepository::CreateAnnouncement(const AnnouncementEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::ANNOUNCEMENT_CREATE,
            data.title, data.content, data.announcement_type,
            data.priority, data.show_until, data.created_by);
        return r[0][0].as<int64_t>();
    });
}

std::vector<AnnouncementEntity> SecurityRepository::GetActiveAnnouncements() {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::ANNOUNCEMENT_GET_ACTIVE);
        return MapResults<AnnouncementEntity>(r, [](const pqxx::row& row, AnnouncementEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.title = row["title"].as<std::string>();
            e.content = row["content"].as<std::string>();
            e.announcement_type = row["announcement_type"].as<std::string>();
            e.priority = row["priority"].as<int32_t>();
            e.created_by = row["created_by"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

int64_t SecurityRepository::CreateWebhookEndpoint(const WebhookEndpointEntity& data) {
    return ExecuteInsert([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WEBHOOK_CREATE,
            data.user_id, data.endpoint_url, data.secret, data.events);
        return r[0][0].as<int64_t>();
    });
}

std::vector<WebhookEndpointEntity> SecurityRepository::GetUserWebhooks(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::WEBHOOK_GET_BY_USER, user_id);
        return MapResults<WebhookEndpointEntity>(r, [](const pqxx::row& row, WebhookEndpointEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.endpoint_url = row["endpoint_url"].as<std::string>();
            e.is_active = row["is_active"].as<bool>();
            e.failure_count = row["failure_count"].as<int32_t>();
        });
    });
}

bool SecurityRepository::LogWebhookCall(int64_t endpoint_id, const std::string& event_type,
        const std::string& request_body, int response_status, const std::string& response_body) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::WEBHOOK_LOG,
            endpoint_id, event_type, request_body, response_status, response_body);
        return true;
    });
}

bool SecurityRepository::CreateSecurityAlert(const SecurityAlertEntity& data) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        tx.exec_params(sql::ALERT_CREATE,
            data.user_id, data.alert_type, data.severity,
            "{}", data.ip_address);
        return true;
    });
}

std::vector<SecurityAlertEntity> SecurityRepository::GetUserAlerts(const std::string& user_id) {
    return ExecuteQuery([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::ALERT_GET_USER, user_id);
        return MapResults<SecurityAlertEntity>(r, [](const pqxx::row& row, SecurityAlertEntity& e) {
            e.id = row["id"].as<int64_t>();
            e.alert_type = row["alert_type"].as<std::string>();
            e.severity = row["severity"].as<std::string>();
            e.ip_address = row["ip_address"].as<std::string>();
            e.created_at = row["created_at"].as<int64_t>();
        });
    });
}

bool SecurityRepository::ResolveAlert(int64_t alert_id, const std::string& user_id) {
    return ExecuteUpdate([&](pqxx::work& tx) {
        pqxx::result r = tx.exec_params(sql::ALERT_RESOLVE, alert_id, user_id);
        return r.affected_rows() > 0;
    });
}

} // namespace furbbs::repository
