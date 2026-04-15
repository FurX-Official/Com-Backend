#ifndef FURBBS_COMMON_SECURITY_H
#define FURBBS_COMMON_SECURITY_H

#include <string>
#include <vector>
#include <optional>
#include <regex>
#include <unordered_map>
#include <mutex>
#include <cstdint>

namespace furbbs::common {

class Security {
public:
    static std::string SanitizeHtml(const std::string& input);
    static std::string SanitizeSql(const std::string& input);
    static std::string SanitizeXss(const std::string& input);
    
    static bool ValidateEmail(const std::string& email);
    static bool ValidateUsername(const std::string& username);
    static bool ValidatePassword(const std::string& password);
    static bool ValidateUrl(const std::string& url);
    static bool ValidateContentLength(const std::string& content, size_t max_length);
    
    static std::string HashPassword(const std::string& password, const std::string& salt);
    static std::string GenerateSalt();
    static bool VerifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    
    static std::string EncryptAES(const std::string& plaintext, const std::string& key);
    static std::string DecryptAES(const std::string& ciphertext, const std::string& key);
    
    static std::string GenerateCsrfToken();
    static bool VerifyCsrfToken(const std::string& token);
    
    static std::optional<std::string> DetectSqlInjection(const std::string& input);
    static std::optional<std::string> DetectXss(const std::string& input);

private:
    static const std::vector<std::string> SQL_KEYWORDS;
    static const std::vector<std::regex> XSS_PATTERNS;
    static const std::regex EMAIL_REGEX;
    static const std::regex USERNAME_REGEX;
    static const std::regex URL_REGEX;
};

class RateLimiter {
public:
    static RateLimiter& Instance();
    
    bool Allow(const std::string& key, int max_requests, int window_seconds);
    void Reset(const std::string& key);
    
private:
    RateLimiter() = default;
    ~RateLimiter() = default;
    
    struct RateLimitEntry {
        int count = 0;
        int64_t window_start = 0;
    };
    
    std::unordered_map<std::string, RateLimitEntry> entries_;
    std::mutex mutex_;
};

enum class Permission {
    CREATE_POST = 1 << 0,
    EDIT_ANY_POST = 1 << 1,
    DELETE_ANY_POST = 1 << 2,
    PIN_POST = 1 << 3,
    LOCK_POST = 1 << 4,
    MANAGE_USERS = 1 << 5,
    MODERATE_CONTENT = 1 << 6,
    CREATE_COMMISSION = 1 << 7,
    MANAGE_EVENTS = 1 << 8,
    ADMIN_ACCESS = 1 << 9,
};

class PermissionManager {
public:
    static PermissionManager& Instance();
    
    bool HasPermission(const std::string& user_id, Permission permission);
    void GrantPermission(const std::string& user_id, Permission permission);
    void RevokePermission(const std::string& user_id, Permission permission);
    
    bool IsAdmin(const std::string& user_id);
    bool IsModerator(const std::string& user_id);

private:
    PermissionManager() = default;
    ~PermissionManager() = default;
    
    std::unordered_map<std::string, int> user_permissions_;
    std::mutex mutex_;
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_SECURITY_H
