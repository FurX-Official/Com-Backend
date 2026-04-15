#include "database.h"
#include <spdlog/spdlog.h>
#include "../config/config.h"

namespace furbbs::db {

Database& Database::Instance() {
    static Database instance;
    return instance;
}

bool Database::Init(const std::string& host, uint16_t port, 
                    const std::string& dbname, const std::string& user, 
                    const std::string& password, uint32_t max_connections) {
    connection_string_ = "host=" + host + " port=" + std::to_string(port) +
                        " dbname=" + dbname + " user=" + user + 
                        " password=" + password;
    max_connections_ = max_connections;
    
    try {
        for (uint32_t i = 0; i < max_connections_; ++i) {
            auto conn = std::make_shared<pqxx::connection>(connection_string_);
            if (conn->is_open()) {
                connections_.push(conn);
                current_connections_++;
            }
        }
        
        spdlog::info("Database connection pool initialized with {} connections", current_connections_);
        return InitTables();
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize database: {}", e.what());
        return false;
    }
}

std::shared_ptr<pqxx::connection> Database::GetConnection() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connections_.empty()) {
        throw std::runtime_error("No available database connections");
    }
    auto conn = connections_.front();
    connections_.pop();
    return conn;
}

void Database::ReleaseConnection(std::shared_ptr<pqxx::connection> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.push(conn);
}

bool Database::InitTables() {
    try {
        Execute([](pqxx::work& txn) {
            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS users (
                    id VARCHAR(128) PRIMARY KEY,
                    name VARCHAR(128) NOT NULL UNIQUE,
                    display_name VARCHAR(128),
                    avatar VARCHAR(512),
                    email VARCHAR(256),
                    bio TEXT,
                    follower_count INTEGER DEFAULT 0,
                    following_count INTEGER DEFAULT 0,
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS categories (
                    id SERIAL PRIMARY KEY,
                    name VARCHAR(128) NOT NULL UNIQUE,
                    description TEXT,
                    icon VARCHAR(256),
                    post_count INTEGER DEFAULT 0,
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS tags (
                    id SERIAL PRIMARY KEY,
                    name VARCHAR(64) NOT NULL UNIQUE,
                    color VARCHAR(32) DEFAULT '#3B82F6',
                    use_count INTEGER DEFAULT 0
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS posts (
                    id SERIAL PRIMARY KEY,
                    title VARCHAR(256) NOT NULL,
                    content TEXT NOT NULL,
                    author_id VARCHAR(128) NOT NULL REFERENCES users(id),
                    category_id INTEGER REFERENCES categories(id),
                    view_count INTEGER DEFAULT 0,
                    like_count INTEGER DEFAULT 0,
                    comment_count INTEGER DEFAULT 0,
                    is_pinned BOOLEAN DEFAULT FALSE,
                    is_locked BOOLEAN DEFAULT FALSE,
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
                    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS post_images (
                    id SERIAL PRIMARY KEY,
                    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
                    url VARCHAR(512) NOT NULL
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS post_tags (
                    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
                    tag_id INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
                    PRIMARY KEY (post_id, tag_id)
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS comments (
                    id SERIAL PRIMARY KEY,
                    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
                    content TEXT NOT NULL,
                    author_id VARCHAR(128) NOT NULL REFERENCES users(id),
                    parent_id INTEGER REFERENCES comments(id) ON DELETE CASCADE,
                    like_count INTEGER DEFAULT 0,
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
                    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS post_likes (
                    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
                    user_id VARCHAR(128) NOT NULL REFERENCES users(id),
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
                    PRIMARY KEY (post_id, user_id)
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS follows (
                    follower_id VARCHAR(128) NOT NULL REFERENCES users(id),
                    following_id VARCHAR(128) NOT NULL REFERENCES users(id),
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
                    PRIMARY KEY (follower_id, following_id)
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS gallery (
                    id SERIAL PRIMARY KEY,
                    title VARCHAR(256) NOT NULL,
                    description TEXT,
                    image_url VARCHAR(512) NOT NULL,
                    author_id VARCHAR(128) NOT NULL REFERENCES users(id),
                    like_count INTEGER DEFAULT 0,
                    view_count INTEGER DEFAULT 0,
                    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
                )
            )");

            txn.exec(R"(
                CREATE TABLE IF NOT EXISTS gallery_tags (
                    gallery_id INTEGER NOT NULL REFERENCES gallery(id) ON DELETE CASCADE,
                    tag_id INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
                    PRIMARY KEY (gallery_id, tag_id)
                )
            )");

            txn.exec(R"(
                INSERT INTO categories (name, description, icon) VALUES
                ('综合讨论', '关于Furry的各种话题', '💬'),
                ('兽设展示', '展示和分享你的兽设', '🎨'),
                ('绘图画廊', '艺术作品展示区', '🖼️'),
                ('写作创作', '小说和故事创作', '📝'),
                ('毛聚活动', '各地线下聚会信息', '🎉'),
                ('技术交流', '编程和技术讨论', '💻'),
                ('音乐分享', 'Furry相关音乐', '🎵')
                ON CONFLICT (name) DO NOTHING
            )");

            txn.exec(R"(
                INSERT INTO tags (name, color) VALUES
                ('狼', '#6366F1'),
                ('狐狸', '#F59E0B'),
                ('龙', '#EF4444'),
                ('猫', '#8B5CF6'),
                ('犬', '#10B981'),
                ('原创', '#3B82F6'),
                ('委托', '#EC4899'),
                ('免费', '#14B8A6')
                ON CONFLICT (name) DO NOTHING
            )");
        });

        spdlog::info("Database tables initialized successfully");
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize tables: {}", e.what());
        return false;
    }
}

} // namespace furbbs::db
