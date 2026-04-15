#include "security.h"
#include <spdlog/spdlog.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <cctype>

namespace furbbs::common {

const std::vector<std::string> Security::SQL_KEYWORDS = {
    "'", "\"", ";", "--", "/*", "*/", "xp_", "sp_", "UNION", "SELECT", "INSERT",
    "UPDATE", "DELETE", "DROP", "ALTER", "CREATE", "TRUNCATE", "EXEC", "UNION ALL"
};

const std::vector<std::regex> Security::XSS_PATTERNS = {
    std::regex("<script[^>]*>.*?</script>", std::regex_constants::icase),
    std::regex("javascript:", std::regex_constants::icase),
    std::regex("vbscript:", std::regex_constants::icase),
    std::regex("on\\w+\\s*=", std::regex_constants::icase),
    std::regex("<iframe[^>]*>", std::regex_constants::icase)
};

const std::regex Security::EMAIL_REGEX(
    R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
);

const std::regex Security::USERNAME_REGEX(
    R"(^[a-zA-Z0-9_]{3,32}$)"
);

const std::regex Security::URL_REGEX(
    R"(^https?://[^\s/$.?#].[^\s]*$)", std::regex_constants::icase
);

std::string Security::SanitizeHtml(const std::string& input) {
    std::string output = input;
    output = std::regex_replace(output, std::regex("<"), "&lt;");
    output = std::regex_replace(output, std::regex(">"), "&gt;");
    output = std::regex_replace(output, std::regex("\""), "&quot;");
    output = std::regex_replace(output, std::regex("'"), "&#39;");
    return output;
}

std::string Security::SanitizeSql(const std::string& input) {
    std::string output = input;
    output = std::regex_replace(output, std::regex("'"), "''");
    output = std::regex_replace(output, std::regex("\\\\"), "\\\\");
    return output;
}

std::string Security::SanitizeXss(const std::string& input) {
    std::string output = input;
    for (const auto& pattern : XSS_PATTERNS) {
        output = std::regex_replace(output, pattern, "[SANITIZED]");
    }
    return output;
}

bool Security::ValidateEmail(const std::string& email) {
    return std::regex_match(email, EMAIL_REGEX);
}

bool Security::ValidateUsername(const std::string& username) {
    return std::regex_match(username, USERNAME_REGEX);
}

bool Security::ValidatePassword(const std::string& password) {
    if (password.length() < 8) return false;
    bool has_upper = std::any_of(password.begin(), password.end(), ::isupper);
    bool has_lower = std::any_of(password.begin(), password.end(), ::islower);
    bool has_digit = std::any_of(password.begin(), password.end(), ::isdigit);
    return has_upper && has_lower && has_digit;
}

bool Security::ValidateUrl(const std::string& url) {
    return url.empty() || std::regex_match(url, URL_REGEX);
}

bool Security::ValidateContentLength(const std::string& content, size_t max_length) {
    return content.length() <= max_length;
}

std::string Security::HashPassword(const std::string& password, const std::string& salt) {
    std::string combined = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::string Security::GenerateSalt() {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return ss.str();
}

bool Security::VerifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    return HashPassword(password, salt) == hash;
}

std::string Security::GenerateCsrfToken() {
    unsigned char token[32];
    RAND_bytes(token, 32);
    
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)token[i];
    }
    return ss.str();
}

bool Security::VerifyCsrfToken(const std::string& token) {
    return token.length() == 64;
}

std::optional<std::string> Security::DetectSqlInjection(const std::string& input) {
    std::string upper_input = input;
    std::transform(upper_input.begin(), upper_input.end(), upper_input.begin(), ::toupper);
    
    for (const auto& keyword : SQL_KEYWORDS) {
        std::string upper_keyword = keyword;
        std::transform(upper_keyword.begin(), upper_keyword.end(), upper_keyword.begin(), ::toupper);
        if (upper_input.find(upper_keyword) != std::string::npos) {
            return keyword;
        }
    }
    return std::nullopt;
}

std::optional<std::string> Security::DetectXss(const std::string& input) {
    for (const auto& pattern : XSS_PATTERNS) {
        if (std::regex_search(input, pattern)) {
            return "Potential XSS detected";
        }
    }
    return std::nullopt;
}

RateLimiter& RateLimiter::Instance() {
    static RateLimiter instance;
    return instance;
}

bool RateLimiter::Allow(const std::string& key, int max_requests, int window_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    auto& entry = entries_[key];
    
    if (now - entry.window_start >= window_seconds) {
        entry.count = 0;
        entry.window_start = now;
    }
    
    if (entry.count >= max_requests) {
        spdlog::warn("Rate limit exceeded for key: {}", key);
        return false;
    }
    
    entry.count++;
    return true;
}

void RateLimiter::Reset(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.erase(key);
}

PermissionManager& PermissionManager::Instance() {
    static PermissionManager instance;
    return instance;
}

bool PermissionManager::HasPermission(const std::string& user_id, Permission permission) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = user_permissions_.find(user_id);
    if (it == user_permissions_.end()) {
        return false;
    }
    return (it->second & static_cast<int>(permission)) != 0;
}

void PermissionManager::GrantPermission(const std::string& user_id, Permission permission) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_permissions_[user_id] |= static_cast<int>(permission);
    spdlog::info("Granted permission {} to user {}", static_cast<int>(permission), user_id);
}

void PermissionManager::RevokePermission(const std::string& user_id, Permission permission) {
    std::lock_guard<std::mutex> lock(mutex_);
    user_permissions_[user_id] &= ~static_cast<int>(permission);
    spdlog::info("Revoked permission {} from user {}", static_cast<int>(permission), user_id);
}

bool PermissionManager::IsAdmin(const std::string& user_id) {
    return HasPermission(user_id, Permission::ADMIN_ACCESS);
}

bool PermissionManager::IsModerator(const std::string& user_id) {
    return HasPermission(user_id, Permission::MODERATE_CONTENT) || IsAdmin(user_id);
}

} // namespace furbbs::common
