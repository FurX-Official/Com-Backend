#include "netease_verify.h"
#include "infrastructure.h"
#include "../config/config.h"
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <random>
#include <curl/curl.h>

namespace furbbs::common {

const std::string NeteaseVerify::API_HOST = "https://verify.dun.163.com";
const std::string NeteaseVerify::REAL_NAME_VERIFY_ENDPOINT = "/v1/realname/authentication";
const std::string NeteaseVerify::FACE_COMPARE_ENDPOINT = "/v1/face/compare";
const std::string NeteaseVerify::IDCARD_OCR_ENDPOINT = "/v1/ocr/idcard";
const std::string NeteaseVerify::GET_RESULT_ENDPOINT = "/v1/realname/result";

NeteaseVerify& NeteaseVerify::Instance() {
    static NeteaseVerify instance;
    return instance;
}

void NeteaseVerify::Initialize(const std::string& secret_id, const std::string& secret_key,
                                const std::string& business_id) {
    secret_id_ = secret_id;
    secret_key_ = secret_key;
    business_id_ = business_id;
    initialized_ = true;
}

std::string NeteaseVerify::GenerateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return std::to_string(millis);
}

std::string NeteaseVerify::GenerateNonce() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99999999);
    return std::to_string(dis(gen));
}

std::string NeteaseVerify::Md5(const std::string& input) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), digest);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::setw(2) << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string NeteaseVerify::Sha1Hex(const std::string& key, const std::string& input) {
    unsigned char digest[SHA_DIGEST_LENGTH];
    unsigned int digest_len;
    HMAC(EVP_sha1(), key.c_str(), static_cast<int>(key.length()),
         reinterpret_cast<const unsigned char*>(input.c_str()), input.length(),
         digest, &digest_len);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < digest_len; ++i) {
        ss << std::setw(2) << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string NeteaseVerify::GenerateSignature(const std::string& method,
                                               const std::string& endpoint,
                                               const std::map<std::string, std::string>& params) {
    std::string param_str;
    for (const auto& [k, v] : params) {
        if (!param_str.empty()) {
            param_str += "&";
        }
        param_str += k + "=" + v;
    }

    std::string sig_src = method + endpoint + param_str;
    return Sha1Hex(secret_key_, sig_src);
}

static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t new_length = size * nmemb;
    try {
        s->append(static_cast<char*>(contents), new_length);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return new_length;
}

std::string NeteaseVerify::HttpPost(const std::string& url,
                                     const std::map<std::string, std::string>& params) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    std::string post_data;
    for (const auto& [k, v] : params) {
        if (!post_data.empty()) {
            post_data += "&";
        }
        char* escaped = curl_easy_escape(curl, v.c_str(), static_cast<int>(v.length()));
        post_data += k + "=" + escaped;
        curl_free(escaped);
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

std::string NeteaseVerify::HttpPostJson(const std::string& url, const nlohmann::json& data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    std::string json_str = data.dump();
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

RealNameVerifyResult NeteaseVerify::VerifyRealName(const std::string& name,
                                                     const std::string& id_card_number,
                                                     const std::string& user_id) {
    RealNameVerifyResult result;
    if (!initialized_) {
        result.success = false;
        result.reason = "Netease Verify not initialized";
        return result;
    }

    std::map<std::string, std::string> params;
    params["secretId"] = secret_id_;
    params["businessId"] = business_id_;
    params["timestamp"] = GenerateTimestamp();
    params["nonce"] = GenerateNonce();
    params["name"] = name;
    params["cardNo"] = id_card_number;
    if (!user_id.empty()) {
        params["userIdentifier"] = user_id;
    }

    params["signature"] = GenerateSignature("POST", REAL_NAME_VERIFY_ENDPOINT, params);

    std::string url = API_HOST + REAL_NAME_VERIFY_ENDPOINT;
    std::string response_str = HttpPost(url, params);

    if (response_str.empty()) {
        result.success = false;
        result.reason = "HTTP request failed";
        return result;
    }

    try {
        nlohmann::json json = nlohmann::json::parse(response_str);
        result.code = json.value("code", -1);
        result.success = (result.code == 200);

        if (json.contains("data")) {
            auto data = json["data"];
            result.task_id = data.value("taskId", "");
            result.status = data.value("status", "");
            result.is_verified = data.value("isVerified", false);
            result.is_matched = data.value("isMatched", false);
            result.confidence_level = data.value("confidenceLevel", 0);
            result.reason = data.value("reason", "");
            result.transaction_id = data.value("transactionId", "");
        } else {
            result.reason = json.value("msg", "Unknown error");
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.reason = std::string("Parse response failed: ") + e.what();
    }

    return result;
}

FaceCompareResult NeteaseVerify::CompareFace(const std::string& image_url1,
                                              const std::string& image_url2,
                                              const std::string& user_id) {
    FaceCompareResult result;
    if (!initialized_) {
        result.success = false;
        return result;
    }

    std::map<std::string, std::string> params;
    params["secretId"] = secret_id_;
    params["businessId"] = business_id_;
    params["timestamp"] = GenerateTimestamp();
    params["nonce"] = GenerateNonce();
    params["imageUrl1"] = image_url1;
    params["imageUrl2"] = image_url2;
    if (!user_id.empty()) {
        params["userIdentifier"] = user_id;
    }

    params["signature"] = GenerateSignature("POST", FACE_COMPARE_ENDPOINT, params);

    std::string url = API_HOST + FACE_COMPARE_ENDPOINT;
    std::string response_str = HttpPost(url, params);

    if (response_str.empty()) {
        result.success = false;
        return result;
    }

    try {
        nlohmann::json json = nlohmann::json::parse(response_str);
        result.code = json.value("code", -1);
        result.success = (result.code == 200);

        if (json.contains("data")) {
            auto data = json["data"];
            result.task_id = data.value("taskId", "");
            result.status = data.value("status", "");
            result.similarity = data.value("similarity", 0.0f);
            result.is_same_person = data.value("isSamePerson", false);
            result.best_face_url = data.value("bestFaceUrl", "");
        }
    } catch (const std::exception& e) {
        result.success = false;
    }

    return result;
}

IdCardOcrResult NeteaseVerify::OcrIdCard(const std::string& image_url,
                                          const std::string& card_side,
                                          const std::string& user_id) {
    IdCardOcrResult result;
    if (!initialized_) {
        result.success = false;
        return result;
    }

    std::map<std::string, std::string> params;
    params["secretId"] = secret_id_;
    params["businessId"] = business_id_;
    params["timestamp"] = GenerateTimestamp();
    params["nonce"] = GenerateNonce();
    params["imageUrl"] = image_url;
    params["side"] = card_side;
    if (!user_id.empty()) {
        params["userIdentifier"] = user_id;
    }

    params["signature"] = GenerateSignature("POST", IDCARD_OCR_ENDPOINT, params);

    std::string url = API_HOST + IDCARD_OCR_ENDPOINT;
    std::string response_str = HttpPost(url, params);

    if (response_str.empty()) {
        result.success = false;
        return result;
    }

    try {
        nlohmann::json json = nlohmann::json::parse(response_str);
        result.code = json.value("code", -1);
        result.success = (result.code == 200);

        if (json.contains("data")) {
            auto data = json["data"];
            result.task_id = data.value("taskId", "");
            result.name = data.value("name", "");
            result.id_card_number = data.value("cardNo", "");
            result.gender = data.value("gender", "");
            result.ethnicity = data.value("ethnicity", "");
            result.birthday = data.value("birthday", "");
            result.address = data.value("address", "");
            result.authority = data.value("authority", "");
            result.valid_date = data.value("validDate", "");
            result.type = data.value("type", 0);
        }
    } catch (const std::exception& e) {
        result.success = false;
    }

    return result;
}

std::optional<RealNameVerifyResult> NeteaseVerify::GetVerifyResult(const std::string& task_id) {
    if (!initialized_) {
        return std::nullopt;
    }

    std::map<std::string, std::string> params;
    params["secretId"] = secret_id_;
    params["businessId"] = business_id_;
    params["timestamp"] = GenerateTimestamp();
    params["nonce"] = GenerateNonce();
    params["taskId"] = task_id;

    params["signature"] = GenerateSignature("POST", GET_RESULT_ENDPOINT, params);

    std::string url = API_HOST + GET_RESULT_ENDPOINT;
    std::string response_str = HttpPost(url, params);

    if (response_str.empty()) {
        return std::nullopt;
    }

    try {
        nlohmann::json json = nlohmann::json::parse(response_str);
        RealNameVerifyResult result;
        result.code = json.value("code", -1);
        result.success = (result.code == 200);

        if (json.contains("data")) {
            auto data = json["data"];
            result.task_id = data.value("taskId", "");
            result.status = data.value("status", "");
            result.is_verified = data.value("isVerified", false);
            result.is_matched = data.value("isMatched", false);
            result.confidence_level = data.value("confidenceLevel", 0);
            result.reason = data.value("reason", "");
        }

        return result;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

} // namespace furbbs::common
