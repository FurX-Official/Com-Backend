#ifndef FURBBS_COMMON_INFRASTRUCTURE_H
#define FURBBS_COMMON_INFRASTRUCTURE_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <functional>
#include <optional>
#include <filesystem>

namespace furbbs::common {

class FileStorage {
public:
    static FileStorage& Instance();

    struct FileInfo {
        std::string id;
        std::string filename;
        std::string original_name;
        std::string mime_type;
        size_t size;
        std::string url;
        std::string path;
    };

    FileInfo Save(const std::string& content, 
                  const std::string& original_name,
                  const std::string& mime_type,
                  const std::string& user_id = "",
                  const std::string& purpose = "");

    std::optional<FileInfo> Get(const std::string& file_id);
    bool Delete(const std::string& file_id, const std::string& user_id = "");

    std::string GenerateFileId();
    std::string GetExtension(const std::string& filename);
    bool ValidateSize(size_t size, size_t max_size = 10 * 1024 * 1024);
    bool ValidateMimeType(const std::string& mime_type);

private:
    FileStorage();
    ~FileStorage() = default;

    std::string storage_path_;
    std::string base_url_;
    std::mutex mutex_;

    std::string GetHash(const std::string& content);
};

class EmailSender {
public:
    static EmailSender& Instance();

    bool Send(const std::string& to, const std::string& subject, const std::string& body);
    bool SendTemplate(const std::string& to, const std::string& template_name, 
                      const std::map<std::string, std::string>& vars);

private:
    EmailSender();
    ~EmailSender() = default;

    std::string smtp_host_;
    int smtp_port_;
    std::string smtp_user_;
    std::string smtp_pass_;
    std::string from_;
};

class EmailCodeManager {
public:
    static EmailCodeManager& Instance();

    std::string GenerateCode(int length = 6);
    bool SendCode(const std::string& email, const std::string& type);
    bool VerifyCode(const std::string& email, const std::string& code, const std::string& type, bool mark_used = true);
    void CleanExpired();

    static const int EXPIRE_SECONDS = 300;
};

class DistributedLock {
public:
    static DistributedLock& Instance();

    bool TryLock(const std::string& key, const std::string& owner, int ttl_seconds = 30);
    bool ReleaseLock(const std::string& key, const std::string& owner);
    bool IsLocked(const std::string& key);

    template<typename F>
    bool WithLock(const std::string& key, const std::string& owner, F&& func, int ttl_seconds = 30) {
        if (!TryLock(key, owner, ttl_seconds)) {
            return false;
        }
        try {
            func();
        } catch (...) {
            ReleaseLock(key, owner);
            throw;
        }
        ReleaseLock(key, owner);
        return true;
    }

private:
    DistributedLock() = default;
    ~DistributedLock() = default;
};

class Cache {
public:
    static Cache& Instance();

    template<typename T>
    void Set(const std::string& key, const T& value, int ttl_seconds = 300) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        auto expire = now + std::chrono::seconds(ttl_seconds);
        items_[key] = { std::to_string(value), expire };
    }

    std::optional<std::string> Get(const std::string& key);
    bool Delete(const std::string& key);
    void Clear();
    void CleanExpired();

    struct Stats {
        int64_t hits = 0;
        int64_t misses = 0;
        int64_t evictions = 0;
    };

    Stats GetStats();

private:
    Cache() = default;
    ~Cache() = default;

    struct CacheItem {
        std::string value;
        std::chrono::steady_clock::time_point expire;
    };

    std::map<std::string, CacheItem> items_;
    Stats stats_;
    std::mutex mutex_;
};

class I18n {
public:
    static I18n& Instance();

    void Load();
    std::string Translate(const std::string& key, const std::string& lang = "zh-CN");
    std::string Format(const std::string& key, 
                       const std::map<std::string, std::string>& vars,
                       const std::string& lang = "zh-CN");

private:
    I18n();
    ~I18n() = default;

    std::map<std::string, std::map<std::string, std::string>> translations_;
    std::mutex mutex_;
};

class Scheduler {
public:
    static Scheduler& Instance();

    void Start();
    void Stop();

    void Schedule(const std::string& name, const std::string& cron, 
                  std::function<void()> task);

private:
    Scheduler();
    ~Scheduler();

    void RunLoop();

    std::atomic<bool> running_;
    std::thread worker_;
    std::map<std::string, std::pair<std::string, std::function<void()>>> tasks_;
    std::mutex mutex_;
};

class SystemMonitor {
public:
    static SystemMonitor& Instance();

    double GetCpuUsage();
    double GetMemoryUsageMB();
    double GetDiskUsageGB();
    int64_t GetDatabaseConnections();
    int64_t GetActiveUsers();

private:
    SystemMonitor() = default;
    ~SystemMonitor() = default;
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_INFRASTRUCTURE_H
