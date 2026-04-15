#include "infrastructure.h"
#include <spdlog/spdlog.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <openssl/md5.h>
#include <curl/curl.h>
#include "../db/database.h"
#include "../config/config.h"

namespace furbbs::common {

FileStorage::FileStorage() {
    storage_path_ = config::Config::Instance().Get<std::string>("storage.path", "./uploads");
    base_url_ = config::Config::Instance().Get<std::string>("storage.base_url", "/uploads");
    
    std::filesystem::create_directories(storage_path_);
    for (const auto& subdir : {"avatar", "post", "gallery", "fursona", "other"}) {
        std::filesystem::create_directories(storage_path_ + "/" + subdir);
    }
}

FileStorage& FileStorage::Instance() {
    static FileStorage instance;
    return instance;
}

std::string FileStorage::GenerateFileId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    ss << std::hex << timestamp;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string FileStorage::GetHash(const std::string& content) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(content.c_str()), content.length(), digest);
    
    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

std::string FileStorage::GetExtension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        return filename.substr(pos);
    }
    return "";
}

bool FileStorage::ValidateSize(size_t size, size_t max_size) {
    return size <= max_size;
}

bool FileStorage::ValidateMimeType(const std::string& mime_type) {
    static const std::vector<std::string> allowed = {
        "image/jpeg", "image/png", "image/gif", "image/webp",
        "application/pdf", "text/plain"
    };
    return std::find(allowed.begin(), allowed.end(), mime_type) != allowed.end();
}

FileStorage::FileInfo FileStorage::Save(const std::string& content,
                                         const std::string& original_name,
                                         const std::string& mime_type,
                                         const std::string& user_id,
                                         const std::string& purpose) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string file_id = GenerateFileId();
    std::string ext = GetExtension(original_name);
    std::string subdir = purpose.empty() ? "other" : purpose;
    std::string filename = file_id + ext;
    std::string relative_path = subdir + "/" + filename;
    std::string full_path = storage_path_ + "/" + relative_path;

    std::ofstream file(full_path, std::ios::binary);
    file.write(content.c_str(), content.size());
    file.close();

    std::string hash = GetHash(content);
    std::string url = base_url_ + "/" + relative_path;

    db::Database::Instance().Execute([&](pqxx::work& txn) {
        txn.exec_params(R"(
            INSERT INTO files (id, user_id, filename, original_name, mime_type, size, 
                              storage_type, path, url, purpose, hash)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
        )", file_id, user_id.empty() ? nullptr : user_id, filename, original_name,
            mime_type.empty() ? nullptr : mime_type, content.size(), "local",
            relative_path, url, purpose.empty() ? nullptr : purpose, hash);
    });

    return {file_id, filename, original_name, mime_type, content.size(), url, relative_path};
}

std::optional<FileStorage::FileInfo> FileStorage::Get(const std::string& file_id) {
    try {
        return db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id, filename, original_name, mime_type, size, url, path, created_at
                FROM files WHERE id = $1
            )", file_id);

            if (result.empty()) {
                return std::optional<FileInfo>();
            }

            const auto& row = result[0];
            FileInfo info;
            info.id = row[0].as<std::string>();
            info.filename = row[1].as<std::string>();
            info.original_name = row[2].as<std::string>();
            if (!row[3].is_null()) info.mime_type = row[3].as<std::string>();
            info.size = row[4].as<size_t>();
            info.url = row[5].as<std::string>();
            info.path = row[6].as<std::string>();
            return std::optional(info);
        });
    } catch (const std::exception& e) {
        spdlog::warn("Failed to get file: {}", e.what());
        return std::nullopt;
    }
}

bool FileStorage::Delete(const std::string& file_id, const std::string& user_id) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            std::string path;
            auto result = txn.exec_params(R"(
                SELECT path FROM files WHERE id = $1
            )", file_id);
            
            if (!result.empty()) {
                path = storage_path_ + "/" + result[0][0].as<std::string>();
                std::filesystem::remove(path);
            }

            if (user_id.empty()) {
                txn.exec_params("DELETE FROM files WHERE id = $1", file_id);
            } else {
                txn.exec_params("DELETE FROM files WHERE id = $1 AND user_id = $2", file_id, user_id);
            }
        });
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to delete file: {}", e.what());
        return false;
    }
}

EmailSender& EmailSender::Instance() {
    static EmailSender instance;
    return instance;
}

EmailSender::EmailSender() {
    smtp_host_ = config::Config::Instance().Get<std::string>("email.smtp.host", "");
    smtp_port_ = config::Config::Instance().Get<int>("email.smtp.port", 587);
    smtp_user_ = config::Config::Instance().Get<std::string>("email.smtp.user", "");
    smtp_pass_ = config::Config::Instance().Get<std::string>("email.smtp.password", "");
    from_ = config::Config::Instance().Get<std::string>("email.from", "noreply@furbbs.com");
}

bool EmailSender::Send(const std::string& to, const std::string& subject, const std::string& body) {
    if (smtp_host_.empty()) {
        spdlog::info("[Mock Email] To: {}, Subject: {}, Body: {}", to, subject, body);
        return true;
    }

    try {
        CURL* curl = curl_easy_init();
        if (!curl) return false;

        std::string url = "smtp://" + smtp_host_ + ":" + std::to_string(smtp_port_);
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, smtp_user_.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp_pass_.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        struct curl_slist* recipients = nullptr;
        recipients = curl_slist_append(recipients, to.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_.c_str());

        std::string payload = "From: " + from_ + "\r\n"
                            "To: " + to + "\r\n"
                            "Subject: " + subject + "\r\n"
                            "\r\n" + body + "\r\n";

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        CURLcode res = curl_easy_perform(curl);
        
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);

        return res == CURLE_OK;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to send email: {}", e.what());
        return false;
    }
}

bool EmailSender::SendTemplate(const std::string& to, const std::string& template_name,
                                const std::map<std::string, std::string>& vars) {
    std::string subject = I18n::Instance().Translate(template_name + "_subject");
    std::string body = I18n::Instance().Format(template_name + "_content", vars);
    return Send(to, subject, body);
}

EmailCodeManager& EmailCodeManager::Instance() {
    static EmailCodeManager instance;
    return instance;
}

std::string EmailCodeManager::GenerateCode(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    
    std::stringstream ss;
    for (int i = 0; i < length; i++) {
        ss << dis(gen);
    }
    return ss.str();
}

bool EmailCodeManager::SendCode(const std::string& email, const std::string& type) {
    auto now = std::chrono::system_clock::now();
    auto expire = now + std::chrono::seconds(EXPIRE_SECONDS);
    auto expire_ms = std::chrono::duration_cast<std::chrono::milliseconds>(expire.time_since_epoch()).count();

    std::string code = GenerateCode();

    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO email_codes (email, code, type, expire_at)
                VALUES ($1, $2, $3, $4)
            )", email, code, type, expire_ms);
        });

        std::map<std::string, std::string> vars = {
            {"code", code},
            {"minutes", std::to_string(EXPIRE_SECONDS / 60)}
        };
        EmailSender::Instance().SendTemplate(email, "email_code", vars);
        
        spdlog::info("Email code sent: {} -> {} ({} min)", email, code, EXPIRE_SECONDS / 60);
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to send email code: {}", e.what());
        return false;
    }
}

bool EmailCodeManager::VerifyCode(const std::string& email, const std::string& code, 
                                   const std::string& type, bool mark_used) {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    try {
        return db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT id FROM email_codes 
                WHERE email = $1 AND code = $2 AND type = $3 
                AND expire_at > $4 AND used = FALSE
            )", email, code, type, now_ms);

            bool valid = !result.empty();
            
            if (valid && mark_used) {
                int64_t id = result[0][0].as<int64_t>();
                txn.exec_params("UPDATE email_codes SET used = TRUE WHERE id = $1", id);
            }

            return valid;
        });
    } catch (const std::exception& e) {
        spdlog::warn("Failed to verify code: {}", e.what());
        return false;
    }
}

void EmailCodeManager::CleanExpired() {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params("DELETE FROM email_codes WHERE expire_at < $1 OR used = TRUE", now_ms);
        });
    } catch (const std::exception& e) {
        spdlog::warn("Failed to clean expired codes: {}", e.what());
    }
}

DistributedLock& DistributedLock::Instance() {
    static DistributedLock instance;
    return instance;
}

bool DistributedLock::TryLock(const std::string& key, const std::string& owner, int ttl_seconds) {
    auto now = std::chrono::system_clock::now();
    auto expire = now + std::chrono::seconds(ttl_seconds);
    auto expire_ms = std::chrono::duration_cast<std::chrono::milliseconds>(expire.time_since_epoch()).count();

    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO distributed_locks (key, owner, expire_at)
                VALUES ($1, $2, $3)
                ON CONFLICT (key) DO UPDATE
                SET owner = $2, expire_at = $3
                WHERE distributed_locks.expire_at < EXTRACT(EPOCH FROM NOW()) * 1000
                   OR distributed_locks.owner = $2
            )", key, owner, expire_ms);
        });
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool DistributedLock::ReleaseLock(const std::string& key, const std::string& owner) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params("DELETE FROM distributed_locks WHERE key = $1 AND owner = $2", key, owner);
        });
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to release lock: {}", e.what());
        return false;
    }
}

bool DistributedLock::IsLocked(const std::string& key) {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    try {
        return db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT 1 FROM distributed_locks 
                WHERE key = $1 AND expire_at > $2
            )", key, now_ms);
            return !result.empty();
        });
    } catch (const std::exception&) {
        return false;
    }
}

Cache& Cache::Instance() {
    static Cache instance;
    return instance;
}

std::optional<std::string> Cache::Get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = items_.find(key);
    if (it == items_.end()) {
        stats_.misses++;
        return std::nullopt;
    }

    auto now = std::chrono::steady_clock::now();
    if (it->second.expire <= now) {
        items_.erase(it);
        stats_.misses++;
        return std::nullopt;
    }

    stats_.hits++;
    return it->second.value;
}

bool Cache::Delete(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return items_.erase(key) > 0;
}

void Cache::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.clear();
}

void Cache::CleanExpired() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = items_.begin(); it != items_.end();) {
        if (it->second.expire <= now) {
            it = items_.erase(it);
            stats_.evictions++;
        } else {
            ++it;
        }
    }
}

Cache::Stats Cache::GetStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    Stats copy = stats_;
    stats_ = {};
    return copy;
}

I18n& I18n::Instance() {
    static I18n instance;
    return instance;
}

I18n::I18n() {
    Load();
}

void I18n::Load() {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec("SELECT lang, key, value FROM i18n_translations");
            
            std::lock_guard<std::mutex> lock(mutex_);
            translations_.clear();
            
            for (const auto& row : result) {
                std::string lang = row[0].as<std::string>();
                std::string key = row[1].as<std::string>();
                std::string value = row[2].as<std::string>();
                translations_[lang][key] = value;
            }
        });
        spdlog::info("Loaded i18n translations");
    } catch (const std::exception& e) {
        spdlog::warn("Failed to load i18n: {}", e.what());
    }
}

std::string I18n::Translate(const std::string& key, const std::string& lang) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto lang_it = translations_.find(lang);
    if (lang_it == translations_.end()) {
        lang_it = translations_.find("zh-CN");
        if (lang_it == translations_.end()) {
            return key;
        }
    }

    auto key_it = lang_it->second.find(key);
    if (key_it == lang_it->second.end()) {
        return key;
    }

    return key_it->second;
}

std::string I18n::Format(const std::string& key, 
                          const std::map<std::string, std::string>& vars,
                          const std::string& lang) {
    std::string result = Translate(key, lang);
    
    for (const auto& [k, v] : vars) {
        size_t pos = 0;
        while ((pos = result.find("{" + k + "}", pos)) != std::string::npos) {
            result.replace(pos, k.length() + 2, v);
            pos += v.length();
        }
    }
    
    return result;
}

Scheduler& Scheduler::Instance() {
    static Scheduler instance;
    return instance;
}

Scheduler::Scheduler() : running_(false) {}

Scheduler::~Scheduler() {
    Stop();
}

void Scheduler::Start() {
    if (running_) return;
    running_ = true;
    worker_ = std::thread(&Scheduler::RunLoop, this);
    spdlog::info("Scheduler started");
}

void Scheduler::Stop() {
    running_ = false;
    if (worker_.joinable()) {
        worker_.join();
    }
    spdlog::info("Scheduler stopped");
}

void Scheduler::Schedule(const std::string& name, const std::string& cron,
                          std::function<void()> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_[name] = {cron, task};
    spdlog::info("Scheduled task: {} {}", name, cron);
}

void Scheduler::RunLoop() {
    while (running_) {
        Cache::Instance().CleanExpired();
        EmailCodeManager::Instance().CleanExpired();
        
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

SystemMonitor& SystemMonitor::Instance() {
    static SystemMonitor instance;
    return instance;
}

double SystemMonitor::GetCpuUsage() {
    return 0.0;
}

double SystemMonitor::GetMemoryUsageMB() {
    return 0.0;
}

double SystemMonitor::GetDiskUsageGB() {
    return 0.0;
}

int64_t SystemMonitor::GetDatabaseConnections() {
    try {
        return db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec("SELECT count(*) FROM pg_stat_activity");
            return result[0][0].as<int64_t>();
        });
    } catch (...) {
        return 0;
    }
}

int64_t SystemMonitor::GetActiveUsers() {
    try {
        auto one_hour_ago = std::chrono::system_clock::now() - std::chrono::hours(1);
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            one_hour_ago.time_since_epoch()).count();
        
        return db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec_params(R"(
                SELECT COUNT(DISTINCT user_id) FROM audit_logs 
                WHERE created_at > $1
            )", timestamp);
            return result[0][0].as<int64_t>();
        });
    } catch (...) {
        return 0;
    }
}

} // namespace furbbs::common
