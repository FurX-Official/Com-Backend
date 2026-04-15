#include "casdoor_auth.h"
#include <spdlog/spdlog.h>
#include <curl/curl.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <cstring>
#include "../db/database.h"

namespace furbbs::auth {

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;
}

static std::string base64_url_decode(const std::string& input) {
    std::string encoded = input;
    std::replace(encoded.begin(), encoded.end(), '-', '+');
    std::replace(encoded.begin(), encoded.end(), '_', '/');
    
    while (encoded.size() % 4 != 0) {
        encoded += '=';
    }
    
    BIO* bio = BIO_new_mem_buf(encoded.data(), static_cast<int>(encoded.size()));
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    std::vector<unsigned char> output(encoded.size());
    int len = BIO_read(bio, output.data(), static_cast<int>(output.size()));
    
    BIO_free_all(bio);
    return std::string(output.begin(), output.begin() + len);
}

CasdoorAuth& CasdoorAuth::Instance() {
    static CasdoorAuth instance;
    return instance;
}

bool CasdoorAuth::Init(const std::string& endpoint, const std::string& client_id,
                       const std::string& client_secret, const std::string& certificate,
                       const std::string& org_name, const std::string& app_name) {
    endpoint_ = endpoint;
    client_id_ = client_id;
    client_secret_ = client_secret;
    certificate_ = certificate;
    org_name_ = org_name;
    app_name_ = app_name;
    
    curl_global_init(CURL_GLOBAL_ALL);
    spdlog::info("Casdoor auth initialized with endpoint: {}", endpoint);
    return true;
}

std::optional<nlohmann::json> CasdoorAuth::ParseJwtToken(const std::string& token) {
    size_t dot1 = token.find('.');
    size_t dot2 = token.find('.', dot1 + 1);
    
    if (dot1 == std::string::npos || dot2 == std::string::npos) {
        return std::nullopt;
    }
    
    std::string payload = token.substr(dot1 + 1, dot2 - dot1 - 1);
    std::string decoded = base64_url_decode(payload);
    
    try {
        return nlohmann::json::parse(decoded);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<CasdoorUser> CasdoorAuth::VerifyToken(const std::string& token) {
    auto jwt = ParseJwtToken(token);
    if (!jwt) {
        return std::nullopt;
    }
    
    try {
        CasdoorUser user;
        user.id = jwt->value("sub", "");
        user.name = jwt->value("name", "");
        user.display_name = jwt->value("displayName", "");
        user.avatar = jwt->value("avatar", "");
        user.email = jwt->value("email", "");
        
        if (user.id.empty()) {
            return std::nullopt;
        }
        
        SyncUserToDB(user);
        return user;
    } catch (const std::exception& e) {
        spdlog::error("Failed to verify token: {}", e.what());
        return std::nullopt;
    }
}

std::optional<CasdoorUser> CasdoorAuth::GetUserInfo(const std::string& user_id) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return std::nullopt;
    }
    
    std::string url = endpoint_ + "/api/get-user?id=" + org_name_ + "/" + user_id;
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return std::nullopt;
    }
    
    try {
        auto json = nlohmann::json::parse(response);
        if (json.contains("data")) {
            auto data = json["data"];
            CasdoorUser user;
            user.id = data.value("id", "");
            user.name = data.value("name", "");
            user.display_name = data.value("displayName", "");
            user.avatar = data.value("avatar", "");
            user.email = data.value("email", "");
            user.bio = data.value("bio", "");
            
            SyncUserToDB(user);
            return user;
        }
    } catch (...) {}
    
    return std::nullopt;
}

bool CasdoorAuth::SyncUserToDB(const CasdoorUser& user) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO users (id, name, display_name, avatar, email, bio)
                VALUES ($1, $2, $3, $4, $5, $6)
                ON CONFLICT (id) DO UPDATE SET
                    name = EXCLUDED.name,
                    display_name = EXCLUDED.display_name,
                    avatar = EXCLUDED.avatar,
                    email = EXCLUDED.email,
                    bio = EXCLUDED.bio
            )", user.id, user.name, user.display_name, user.avatar, user.email, user.bio);
        });
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to sync user to DB: {}", e.what());
        return false;
    }
}

} // namespace furbbs::auth
