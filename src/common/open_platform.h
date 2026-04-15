#ifndef FURBBS_COMMON_OPEN_PLATFORM_H
#define FURBBS_COMMON_OPEN_PLATFORM_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace furbbs::common {

class ApiKeyGenerator {
public:
    static std::string GenerateClientId();
    static std::string GenerateClientSecret();
    static std::string GenerateWebhookSecret();
    static std::string GenerateNonce();
};

class WebhookSigner {
public:
    static std::string Sign(const std::string& payload, const std::string& secret);
    static bool Verify(const std::string& payload, const std::string& signature, const std::string& secret);
};

class WebhookDispatcher {
public:
    static WebhookDispatcher& Instance();

    void DispatchEvent(const std::string& event, const std::map<std::string, std::string>& data);
    void AddWebhook(int64_t app_id, const std::string& endpoint, 
                    const std::vector<std::string>& events, const std::string& secret);

private:
    WebhookDispatcher() = default;
    
    struct WebhookConfig {
        int64_t app_id;
        std::string endpoint;
        std::vector<std::string> events;
        std::string secret;
    };

    std::vector<WebhookConfig> webhooks_;
    std::mutex mutex_;

    void SendWebhook(const WebhookConfig& config, const std::string& event, 
                     const std::map<std::string, std::string>& data);
};

class RateLimiter {
public:
    static RateLimiter& Instance();

    bool Allow(const std::string& client_id, int64_t limit);
    void RecordCall(const std::string& client_id, const std::string& endpoint);
    int64_t GetRemaining(const std::string& client_id, int64_t limit);
    int64_t GetTodayCalls(const std::string& client_id);

private:
    RateLimiter() = default;
    
    std::map<std::string, std::pair<int64_t, int64_t>> counters_;
    std::mutex mutex_;
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_OPEN_PLATFORM_H
