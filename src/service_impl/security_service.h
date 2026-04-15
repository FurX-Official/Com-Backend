#ifndef FURBBS_SERVICE_IMPL_SECURITY_SERVICE_H
#define FURBBS_SERVICE_IMPL_SECURITY_SERVICE_H

#include "../repository/security_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <mutex>

namespace furbbs::service {

class SecurityService {
public:
    static SecurityService& Instance() {
        static SecurityService instance;
        return instance;
    }

    bool IsIpBlacklisted(const std::string& ip_address) {
        return repository::SecurityRepository::Instance().IsIpBlacklisted(ip_address);
    }

    bool AddIpToBlacklist(const std::string& ip, const std::string& reason,
                          const std::string& banned_by, int64_t duration_seconds = 0) {
        repository::IpBlacklistEntity e;
        e.ip_address = ip;
        e.reason = reason;
        e.banned_by = banned_by;
        if (duration_seconds > 0) {
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                (now + std::chrono::seconds(duration_seconds)).time_since_epoch());
            e.expires_at = ms.count();
        }
        return repository::SecurityRepository::Instance().AddIpToBlacklist(e);
    }

    bool CheckRateLimit(const std::string& identifier, const std::string& limit_type,
                        int max_requests, int window_seconds) {
        std::lock_guard<std::mutex> lock(rate_mutex_);
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        int64_t window_start = ms - (window_seconds * 1000LL);

        int count = repository::SecurityRepository::Instance().IncrementRateLimit(
            identifier, limit_type, window_start);

        return count <= max_requests;
    }

    bool BlockUser(const std::string& blocker_id, const std::string& blocked_id,
                   const std::string& reason = "") {
        if (blocker_id == blocked_id) return false;
        repository::UserBlockEntity e;
        e.blocker_id = blocker_id;
        e.blocked_id = blocked_id;
        e.block_type = "all";
        e.reason = reason;
        return repository::SecurityRepository::Instance().BlockUser(e);
    }

    bool UnblockUser(const std::string& blocker_id, const std::string& blocked_id) {
        return repository::SecurityRepository::Instance().UnblockUser(blocker_id, blocked_id);
    }

    bool IsUserBlocked(const std::string& checker_id, const std::string& target_id) {
        return repository::SecurityRepository::Instance().IsUserBlocked(checker_id, target_id) ||
               repository::SecurityRepository::Instance().IsUserBlocked(target_id, checker_id);
    }

    std::vector<repository::UserBlockEntity> GetBlockedUsers(const std::string& blocker_id) {
        return repository::SecurityRepository::Instance().GetBlockedUsers(blocker_id);
    }

    bool CreateSession(const std::string& session_id, const std::string& user_id,
                       const std::string& ip_address = "", const std::string& user_agent = "") {
        repository::UserSessionEntity e;
        e.session_id = session_id;
        e.user_id = user_id;
        e.ip_address = ip_address;
        e.user_agent = user_agent;
        return repository::SecurityRepository::Instance().CreateSession(e);
    }

    std::vector<repository::UserSessionEntity> GetUserSessions(const std::string& user_id) {
        return repository::SecurityRepository::Instance().GetUserSessions(user_id);
    }

    bool RevokeSession(const std::string& session_id, const std::string& user_id) {
        return repository::SecurityRepository::Instance().RevokeSession(session_id, user_id);
    }

    void RecordLogin(const std::string& user_id, bool success,
                     const std::string& ip_address = "", const std::string& failure_reason = "") {
        repository::LoginHistoryEntity e;
        e.user_id = user_id;
        e.ip_address = ip_address;
        e.was_successful = success;
        e.failure_reason = failure_reason;
        repository::SecurityRepository::Instance().AddLoginHistory(e);

        if (!success) {
            CheckBruteForce(user_id, ip_address);
        }
    }

    std::vector<repository::LoginHistoryEntity> GetLoginHistory(
            const std::string& user_id, int page, int page_size) {
        return repository::SecurityRepository::Instance().GetLoginHistory(
            user_id, page_size, (page - 1) * page_size);
    }

    std::vector<repository::AnnouncementEntity> GetActiveAnnouncements() {
        return repository::SecurityRepository::Instance().GetActiveAnnouncements();
    }

    int64_t CreateAnnouncement(const repository::AnnouncementEntity& data) {
        return repository::SecurityRepository::Instance().CreateAnnouncement(data);
    }

    int64_t CreateWebhook(const repository::WebhookEndpointEntity& data) {
        return repository::SecurityRepository::Instance().CreateWebhookEndpoint(data);
    }

    std::vector<repository::WebhookEndpointEntity> GetUserWebhooks(const std::string& user_id) {
        return repository::SecurityRepository::Instance().GetUserWebhooks(user_id);
    }

    std::vector<repository::SecurityAlertEntity> GetUserAlerts(const std::string& user_id) {
        return repository::SecurityRepository::Instance().GetUserAlerts(user_id);
    }

    bool ResolveAlert(int64_t alert_id, const std::string& user_id) {
        return repository::SecurityRepository::Instance().ResolveAlert(alert_id, user_id);
    }

    void TriggerSecurityAlert(const std::string& user_id, const std::string& alert_type,
                               const std::string& severity = "medium",
                               const std::string& ip_address = "") {
        repository::SecurityAlertEntity e;
        e.user_id = user_id;
        e.alert_type = alert_type;
        e.severity = severity;
        e.ip_address = ip_address;
        repository::SecurityRepository::Instance().CreateSecurityAlert(e);
    }

private:
    SecurityService() = default;

    void CheckBruteForce(const std::string& user_id, const std::string& ip) {
        if (!CheckRateLimit(ip, "brute_force", 5, 300)) {
            AddIpToBlacklist(ip, "暴力破解登录", "system", 3600);
            TriggerSecurityAlert(user_id, "brute_force_detected", "high", ip);
        }
    }

    std::mutex rate_mutex_;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_SECURITY_SERVICE_H
