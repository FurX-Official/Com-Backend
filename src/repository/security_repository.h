#ifndef FURBBS_REPOSITORY_SECURITY_REPOSITORY_H
#define FURBBS_REPOSITORY_SECURITY_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <map>
#include <cstdint>

namespace furbbs::repository {

struct IpBlacklistEntity {
    int64_t id = 0;
    std::string ip_address;
    std::string reason;
    std::string banned_by;
    int64_t expires_at = 0;
    int64_t created_at = 0;
};

struct RateLimitEntity {
    std::string identifier;
    std::string limit_type;
    int32_t request_count = 0;
    int64_t window_start = 0;
};

struct UserSessionEntity {
    std::string session_id;
    std::string user_id;
    std::map<std::string, std::string> device_info;
    std::string ip_address;
    std::string location;
    std::string user_agent;
    int64_t last_active = 0;
    int64_t created_at = 0;
};

struct LoginHistoryEntity {
    int64_t id = 0;
    std::string user_id;
    std::string ip_address;
    std::string location;
    std::map<std::string, std::string> device_info;
    std::string user_agent;
    bool was_successful = true;
    std::string failure_reason;
    int64_t created_at = 0;
};

struct UserBlockEntity {
    std::string blocker_id;
    std::string blocked_id;
    std::string block_type;
    std::string reason;
    int64_t created_at = 0;
};

struct AnnouncementEntity {
    int64_t id = 0;
    std::string title;
    std::string content;
    std::string announcement_type;
    int32_t priority = 0;
    bool is_active = true;
    int64_t show_until = 0;
    std::string created_by;
    int64_t created_at = 0;
};

struct WebhookEndpointEntity {
    int64_t id = 0;
    std::string user_id;
    std::string endpoint_url;
    std::string secret;
    std::vector<std::string> events;
    bool is_active = true;
    int64_t last_called_at = 0;
    int32_t failure_count = 0;
    int64_t created_at = 0;
};

struct SecurityAlertEntity {
    int64_t id = 0;
    std::string user_id;
    std::string alert_type;
    std::string severity;
    std::map<std::string, std::string> details;
    std::string ip_address;
    bool is_resolved = false;
    int64_t created_at = 0;
};

class SecurityRepository : protected BaseRepository {
public:
    static SecurityRepository& Instance() {
        static SecurityRepository instance;
        return instance;
    }

    bool AddIpToBlacklist(const IpBlacklistEntity& data);
    bool IsIpBlacklisted(const std::string& ip_address);
    bool RemoveIpFromBlacklist(const std::string& ip_address);

    int32_t IncrementRateLimit(const std::string& identifier,
                                const std::string& limit_type, int64_t window_start);
    bool ResetRateLimit(const std::string& identifier, const std::string& limit_type);

    bool CreateSession(const UserSessionEntity& data);
    std::vector<UserSessionEntity> GetUserSessions(const std::string& user_id);
    bool RevokeSession(const std::string& session_id, const std::string& user_id);
    bool UpdateSessionActivity(const std::string& session_id);

    bool AddLoginHistory(const LoginHistoryEntity& data);
    std::vector<LoginHistoryEntity> GetLoginHistory(
        const std::string& user_id, int limit, int offset);

    bool BlockUser(const UserBlockEntity& data);
    bool UnblockUser(const std::string& blocker_id, const std::string& blocked_id);
    bool IsUserBlocked(const std::string& blocker_id, const std::string& blocked_id);
    std::vector<UserBlockEntity> GetBlockedUsers(const std::string& blocker_id);

    int64_t CreateAnnouncement(const AnnouncementEntity& data);
    std::vector<AnnouncementEntity> GetActiveAnnouncements();

    int64_t CreateWebhookEndpoint(const WebhookEndpointEntity& data);
    std::vector<WebhookEndpointEntity> GetUserWebhooks(const std::string& user_id);
    bool LogWebhookCall(int64_t endpoint_id, const std::string& event_type,
                        const std::string& request_body, int response_status,
                        const std::string& response_body);

    bool CreateSecurityAlert(const SecurityAlertEntity& data);
    std::vector<SecurityAlertEntity> GetUserAlerts(const std::string& user_id);
    bool ResolveAlert(int64_t alert_id, const std::string& user_id);

private:
    SecurityRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_SECURITY_REPOSITORY_H
