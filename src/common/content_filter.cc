#include "content_filter.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>
#include "../db/database.h"

namespace furbbs::common {

const std::vector<PointSystem::LevelConfig> PointSystem::LEVELS = {
    {1, "新生兽崽", 0, "🐣"},
    {2, "毛茸茸", 100, "🐾"},
    {3, "兽群成员", 300, "🐺"},
    {4, "活跃毛怪", 600, "🦊"},
    {5, "社区明星", 1000, "⭐"},
    {6, "兽师达人", 2000, "🎨"},
    {7, "传说之兽", 5000, "👑"},
    {8, "远古神兽", 10000, "🏆"},
    {9, "次元霸主", 20000, "💎"},
    {10, "毛界传奇", 50000, "🌟"}
};

PointSystem& PointSystem::Instance() {
    static PointSystem instance;
    return instance;
}

int PointSystem::GetLevel(int64_t points) {
    for (int i = LEVELS.size() - 1; i >= 0; i--) {
        if (points >= LEVELS[i].points_required) {
            return LEVELS[i].level;
        }
    }
    return 1;
}

std::string PointSystem::GetLevelName(int64_t points) {
    for (int i = LEVELS.size() - 1; i >= 0; i--) {
        if (points >= LEVELS[i].points_required) {
            return LEVELS[i].name;
        }
    }
    return LEVELS[0].name;
}

int64_t PointSystem::GetNextLevelPoints(int64_t points) {
    for (size_t i = 0; i < LEVELS.size(); i++) {
        if (points < LEVELS[i].points_required) {
            return LEVELS[i].points_required;
        }
    }
    return LEVELS.back().points_required;
}

int PointSystem::CalculateRank(int64_t points) {
    int rank = 1;
    for (size_t i = 0; i < LEVELS.size(); i++) {
        if (points >= LEVELS[i].points_required) {
            rank = LEVELS[i].level;
        }
    }
    return rank;
}

ContentFilter::ContentFilter() : root_(new TrieNode()) {
    LoadFromDatabase();
}

ContentFilter::~ContentFilter() {
    ClearTrie();
}

ContentFilter& ContentFilter::Instance() {
    static ContentFilter instance;
    return instance;
}

void ContentFilter::ClearTrie() {
    delete root_;
    root_ = new TrieNode();
}

bool ContentFilter::LoadFromDatabase() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        words_.clear();
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            auto result = txn.exec("SELECT word, level, replacement FROM sensitive_words");
            for (const auto& row : result) {
                SensitiveWord word;
                word.word = row[0].as<std::string>();
                word.level = row[1].as<int>();
                word.replacement = row[2].as<std::string>();
                words_.push_back(word);
                word_set_.insert(word.word);
            }
        });
        
        BuildTrie();
        spdlog::info("Loaded {} sensitive words", words_.size());
        return true;
    } catch (const std::exception& e) {
        spdlog::warn("Failed to load sensitive words: {}", e.what());
        return false;
    }
}

void ContentFilter::BuildTrie() {
    ClearTrie();
    for (auto& word : words_) {
        TrieNode* current = root_;
        for (char c : word.word) {
            char lower_c = std::tolower(c);
            if (!current->children.count(lower_c)) {
                current->children[lower_c] = new TrieNode();
            }
            current = current->children[lower_c];
        }
        current->is_end = true;
        current->word_info = &word;
    }
}

std::string ContentFilter::Filter(const std::string& content) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string result = content;
    std::string lower_content = content;
    std::transform(lower_content.begin(), lower_content.end(), lower_content.begin(), ::tolower);
    
    for (const auto& word : words_) {
        std::string lower_word = word.word;
        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
        
        size_t pos = 0;
        while ((pos = lower_content.find(lower_word, pos)) != std::string::npos) {
            result.replace(pos, word.word.length(), word.replacement);
            lower_content.replace(pos, word.word.length(), word.replacement);
            pos += word.replacement.length();
        }
    }
    
    return result;
}

bool ContentFilter::ContainsSensitive(const std::string& content) {
    std::string lower_content = content;
    std::transform(lower_content.begin(), lower_content.end(), lower_content.begin(), ::tolower);
    
    for (const auto& word : words_) {
        if (lower_content.find(word.word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<SensitiveWord> ContentFilter::FindSensitiveWords(const std::string& content) {
    std::vector<SensitiveWord> found;
    std::string lower_content = content;
    std::transform(lower_content.begin(), lower_content.end(), lower_content.begin(), ::tolower);
    
    for (const auto& word : words_) {
        if (lower_content.find(word.word) != std::string::npos) {
            found.push_back(word);
        }
    }
    return found;
}

void ContentFilter::AddWord(const SensitiveWord& word) {
    std::lock_guard<std::mutex> lock(mutex_);
    words_.push_back(word);
    word_set_.insert(word.word);
    BuildTrie();
}

void ContentFilter::RemoveWord(const std::string& word) {
    std::lock_guard<std::mutex> lock(mutex_);
    word_set_.erase(word);
    words_.erase(std::remove_if(words_.begin(), words_.end(),
        [&](const SensitiveWord& w) { return w.word == word; }), words_.end());
    BuildTrie();
}

void ContentFilter::Reload() {
    LoadFromDatabase();
}

AuditLogger& AuditLogger::Instance() {
    static AuditLogger instance;
    return instance;
}

void AuditLogger::Log(const std::string& user_id, const std::string& action,
                      const std::string& ip, const std::string& details) {
    try {
        db::Database::Instance().Execute([&](pqxx::work& txn) {
            txn.exec_params(R"(
                INSERT INTO audit_logs (user_id, action, ip, details)
                VALUES ($1, $2, $3, $4)
            )", user_id.empty() ? nullptr : user_id, action, ip.empty() ? nullptr : ip, details.empty() ? nullptr : details);
        });
        spdlog::info("Audit: user={} action={} ip={} details={}", user_id, action, ip, details);
    } catch (const std::exception& e) {
        spdlog::warn("Failed to log audit: {}", e.what());
    }
}

} // namespace furbbs::common
