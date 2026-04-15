#include "open_platform.h"
#include <spdlog/spdlog.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <curl/curl.h>
#include "../db/database.h"

namespace furbbs::common {

std::string ApiKeyGenerator::GenerateClientId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "furbbs_";
    for (int i = 0; i < 16; i++) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string ApiKeyGenerator::GenerateClientSecret() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string ApiKeyGenerator::GenerateWebhookSecret() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "whsec_";
    for (int i = 0; i < 40; i++) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string ApiKeyGenerator::GenerateNonce() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    
    return std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

std::string WebhookSigner::Sign(const std::string& payload, const std::string& secret) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, secret.c_str(), static_cast<int>(secret.length()), EVP_sha256(), nullptr);
    HMAC_Update(ctx, reinterpret_cast<const unsigned char*>(payload.c_str()), payload.length());
    HMAC_Final(ctx, digest, &digest_len);
    HMAC_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < digest_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return "sha256=" + ss.str();
}

bool WebhookSigner::Verify(const std::string& payload, const std::string& signature, const std::string& secret) {
    std::string expected = Sign(payload, secret);
    return expected == signature;
}

WebhookDispatcher& WebhookDispatcher::Instance() {
    static WebhookDispatcher instance;
    return instance;
}

void WebhookDispatcher::AddWebhook(int64_t app_id, const std::string& endpoint,
                                    const std::vector<std::string>& events, const std::string& secret) {
    std::lock_guard<std::mutex> lock(mutex_);
    webhooks_.push_back({app_id, endpoint, events, secret});
}

void WebhookDispatcher::DispatchEvent(const std::string& event, const std::map<std::string, std::string>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& wh : webhooks_) {
        bool event_matched = false;
        for (const auto& e : wh.events) {
            if (e == "*" || e == event) {
                event_matched = true;
                break;
            }
        }
        
        if (event_matched) {
            SendWebhook(wh, event, data);
        }
    }
}

void WebhookDispatcher::SendWebhook(const WebhookConfig& config, const std::string& event,
                                     const std::map<std::string, std::string>& data) {
    try {
        std::stringstream payload_ss;
        payload_ss << "{\"event\":\"" << event << "\",";
        payload_ss << "\"data\":{";
        bool first = true;
        for (const auto& [k, v] : data) {
            if (!first) payload_ss << ",";
            payload_ss << "\"" << k << "\":\"" << v << "\"";
            first = false;
        }
        payload_ss << "}}";
        
        std::string payload = payload_ss.str();
        std::string signature = WebhookSigner::Sign(payload, config.secret);

        CURL* curl = curl_easy_init();
        if (curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, ("X-FurBBS-Signature: " + signature).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, config.endpoint.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                spdlog::warn("Webhook failed: {} -> {}", config.endpoint, curl_easy_strerror(res));
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    } catch (const std::exception& e) {
        spdlog::warn("Webhook exception: {}", e.what());
    }
}

RateLimiter& RateLimiter::Instance() {
    static RateLimiter instance;
    return instance;
}

bool RateLimiter::Allow(const std::string& client_id, int64_t limit) {
    return GetRemaining(client_id, limit) > 0;
}

void RateLimiter::RecordCall(const std::string& client_id, const std::string& endpoint) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO api_stats (client_id, endpoint, call_count, call_date)
                VALUES ($1, $2, 1, CURRENT_DATE)
                ON CONFLICT (client_id, endpoint, call_date) 
                DO UPDATE SET call_count = api_stats.call_count + 1
            )", client_id, endpoint);
        });
    } catch (const std::exception& e) {
        spdlog::warn("Failed to record API call: {}", e.what());
    }
}

int64_t RateLimiter::GetRemaining(const std::string& client_id, int64_t limit) {
    int64_t used = GetTodayCalls(client_id);
    return std::max(0L, limit - used);
}

int64_t RateLimiter::GetTodayCalls(const std::string& client_id) {
    int64_t total = 0;
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT COALESCE(SUM(call_count), 0)
                FROM api_stats
                WHERE client_id = $1 AND call_date = CURRENT_DATE
            )", client_id);
            if (!result.empty()) {
                total = result[0][0].as<int64_t>();
            }
        });
    } catch (const std::exception& e) {
        spdlog::warn("Failed to get API calls: {}", e.what());
    }
    return total;
}

} // namespace furbbs::common
