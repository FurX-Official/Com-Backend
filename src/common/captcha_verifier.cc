#include "captcha_verifier.h"
#include "../config/config.h"
#include "../db/database.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

namespace furbbs::common {

CaptchaVerifier& CaptchaVerifier::Instance() {
    static CaptchaVerifier instance;
    return instance;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch(std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

std::string CaptchaVerifier::HttpPost(const std::string& url, const std::string& post_data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res == CURLE_OK) {
            return response;
        }
    }
    return "";
}

std::optional<CaptchaResult> CaptchaVerifier::Verify(const std::string& provider,
                                                 const std::string& response_token,
                                                 const std::string& remote_ip) {
    std::string secret;
    std::string verify_url;
    bool found = false;

    db::Database::Instance().Execute([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT secret_key, verify_url FROM captcha_settings 
            WHERE provider = $1 AND is_active = TRUE
        )", provider);
        
        if (!result.empty()) {
            secret = result[0][0].as<std::string>();
            verify_url = result[0][1].as<std::string>();
            found = true;
        }
    });

    if (!found) {
        CaptchaResult res;
        res.success = true;
        return res;
    }

    if (provider == "recaptcha_v2" || provider == "recaptcha_v3") {
        return VerifyReCaptcha(secret, response_token, remote_ip, verify_url);
    } else if (provider == "hcaptcha") {
        return VerifyHCaptcha(secret, response_token, remote_ip, verify_url);
    } else if (provider == "geetest") {
        return VerifyGeeTest(secret, response_token, remote_ip, verify_url);
    }

    return std::nullopt;
}

std::optional<CaptchaResult> CaptchaVerifier::VerifyReCaptcha(const std::string& secret,
                                                              const std::string& response_token,
                                                              const std::string& remote_ip,
                                                              const std::string& verify_url) {
    std::string post_data = "secret=" + secret + "&response=" + response_token;
    if (!remote_ip.empty()) {
        post_data += "&remoteip=" + remote_ip;
    }

    std::string response = HttpPost(verify_url, post_data);
    if (response.empty()) {
        return std::nullopt;
    }

    try {
        auto json = nlohmann::json::parse(response);
        CaptchaResult result;
        result.success = json.value("success", false);
        result.score = json.value("score", 0.0f);
        
        if (json.contains("error-codes") && json["error-codes"].is_array()) {
            for (const auto& err : json["error-codes"]) {
                result.error_codes.push_back(err);
            }
        }
        
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<CaptchaResult> CaptchaVerifier::VerifyHCaptcha(const std::string& secret,
                                                           const std::string& response_token,
                                                           const std::string& remote_ip,
                                                           const std::string& verify_url) {
    std::string post_data = "secret=" + secret + "&response=" + response_token;
    if (!remote_ip.empty()) {
        post_data += "&remoteip=" + remote_ip;
    }

    std::string response = HttpPost(verify_url, post_data);
    if (response.empty()) {
        return std::nullopt;
    }

    try {
        auto json = nlohmann::json::parse(response);
        CaptchaResult result;
        result.success = json.value("success", false);
        
        if (json.contains("error-codes") && json["error-codes"].is_array()) {
            for (const auto& err : json["error-codes"]) {
                result.error_codes.push_back(err);
            }
        }
        
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<CaptchaResult> CaptchaVerifier::VerifyGeeTest(const std::string& secret,
                                                              const std::string& response_token,
                                                              const std::string& remote_ip,
                                                              const std::string& verify_url) {
    try {
        auto json = nlohmann::json::parse(response_token);
        std::string lot_number = json.value("lot_number", "");
        std::string captcha_output = json.value("captcha_output", "");
        std::string pass_token = json.value("pass_token", "");
        std::string gen_time = json.value("gen_time", "");

        std::string post_data = "lot_number=" + lot_number + 
                                "&captcha_output=" + captcha_output +
                                "&pass_token=" + pass_token +
                                "&gen_time=" + gen_time;

        std::string response = HttpPost(verify_url, post_data);
        if (response.empty()) {
            return std::nullopt;
        }

        auto res_json = nlohmann::json::parse(response);
        CaptchaResult result;
        result.success = res_json.value("result", "fail") == "success";
        
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace furbbs::common
