#ifndef FURBBS_COMMON_CONTENT_FILTER_H
#define FURBBS_COMMON_CONTENT_FILTER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

namespace furbbs::common {

struct SensitiveWord {
    std::string word;
    int level;
    std::string replacement;
};

struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    bool is_end = false;
    SensitiveWord* word_info = nullptr;
    ~TrieNode() {
        for (auto& [k, v] : children) delete v;
    }
};

class ContentFilter {
public:
    static ContentFilter& Instance();

    bool LoadFromDatabase();
    
    std::string Filter(const std::string& content);
    bool ContainsSensitive(const std::string& content);
    std::vector<SensitiveWord> FindSensitiveWords(const std::string& content);
    
    void AddWord(const SensitiveWord& word);
    void RemoveWord(const std::string& word);
    void Reload();

private:
    ContentFilter();
    ~ContentFilter();

    void BuildTrie();
    void ClearTrie();

    TrieNode* root_;
    std::unordered_set<std::string> word_set_;
    std::vector<SensitiveWord> words_;
    std::mutex mutex_;
};

class PointSystem {
public:
    static PointSystem& Instance();

    struct LevelConfig {
        int level;
        std::string name;
        int64_t points_required;
        std::string icon;
    };

    static const std::vector<LevelConfig> LEVELS;

    static int GetLevel(int64_t points);
    static std::string GetLevelName(int64_t points);
    static int64_t GetNextLevelPoints(int64_t points);
    static int CalculateRank(int64_t points);
    
    static const int POINTS_POST = 10;
    static const int POINTS_COMMENT = 2;
    static const int POINTS_LIKE = 1;
    static const int POINTS_FOLLOW = 5;
    static const int POINTS_CHECKIN_BASE = 5;
    static const int POINTS_CHECKIN_CONSECUTIVE_BONUS = 1;
    static const int POINTS_GALLERY = 15;
    static const int POINTS_FURSONA = 20;
};

class AuditLogger {
public:
    static AuditLogger& Instance();

    void Log(const std::string& user_id, const std::string& action, 
             const std::string& ip = "", const std::string& details = "");
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_CONTENT_FILTER_H
