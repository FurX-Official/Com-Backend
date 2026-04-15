CREATE DATABASE furbbs;
\c furbbs;

CREATE TYPE content_rating AS ENUM ('SAFE', 'MATURE', 'EXPLICIT');
CREATE TYPE commission_status AS ENUM ('OPEN', 'CLOSED', 'ONLY_FRIENDS');
CREATE TYPE order_status AS ENUM ('PENDING', 'ACCEPTED', 'IN_PROGRESS', 'FINISHED', 'CANCELLED');
CREATE TYPE event_status AS ENUM ('PLANNING', 'CONFIRMED', 'ONGOING', 'ENDED', 'CANCELLED');

CREATE TABLE IF NOT EXISTS users (
    id VARCHAR(128) PRIMARY KEY,
    name VARCHAR(128) NOT NULL UNIQUE,
    display_name VARCHAR(128),
    avatar VARCHAR(512),
    email VARCHAR(256),
    bio TEXT,
    follower_count INTEGER DEFAULT 0,
    following_count INTEGER DEFAULT 0,
    commission_status commission_status DEFAULT 'CLOSED',
    max_allowed_rating content_rating DEFAULT 'SAFE',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS badges (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    icon VARCHAR(128),
    description TEXT
);

CREATE TABLE IF NOT EXISTS user_badges (
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    badge_id INTEGER NOT NULL REFERENCES badges(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, badge_id)
);

CREATE TABLE IF NOT EXISTS fursonas (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    species VARCHAR(128),
    gender VARCHAR(64),
    pronouns VARCHAR(64),
    description TEXT,
    reference_image VARCHAR(512),
    colors VARCHAR(32)[],
    is_main BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS categories (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL UNIQUE,
    description TEXT,
    icon VARCHAR(256),
    post_count INTEGER DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS tags (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    color VARCHAR(32) DEFAULT '#3B82F6',
    use_count INTEGER DEFAULT 0
);

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
    is_removed BOOLEAN DEFAULT FALSE,
    moderation_reason VARCHAR(512),
    moderated_by VARCHAR(128) REFERENCES users(id),
    rating content_rating DEFAULT 'SAFE',
    requires_verification BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS post_images (
    id SERIAL PRIMARY KEY,
    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
    url VARCHAR(512) NOT NULL
);

CREATE TABLE IF NOT EXISTS post_tags (
    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
    tag_id INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
    PRIMARY KEY (post_id, tag_id)
);

CREATE TABLE IF NOT EXISTS comments (
    id SERIAL PRIMARY KEY,
    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
    content TEXT NOT NULL,
    author_id VARCHAR(128) NOT NULL REFERENCES users(id),
    parent_id INTEGER REFERENCES comments(id) ON DELETE CASCADE,
    like_count INTEGER DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS post_likes (
    post_id INTEGER NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (post_id, user_id)
);

CREATE TABLE IF NOT EXISTS follows (
    follower_id VARCHAR(128) NOT NULL REFERENCES users(id),
    following_id VARCHAR(128) NOT NULL REFERENCES users(id),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (follower_id, following_id)
);

CREATE TABLE IF NOT EXISTS gallery (
    id SERIAL PRIMARY KEY,
    title VARCHAR(256) NOT NULL,
    description TEXT,
    image_url VARCHAR(512) NOT NULL,
    author_id VARCHAR(128) NOT NULL REFERENCES users(id),
    like_count INTEGER DEFAULT 0,
    view_count INTEGER DEFAULT 0,
    rating content_rating DEFAULT 'SAFE',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS gallery_tags (
    gallery_id INTEGER NOT NULL REFERENCES gallery(id) ON DELETE CASCADE,
    tag_id INTEGER NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
    PRIMARY KEY (gallery_id, tag_id)
);

CREATE TABLE IF NOT EXISTS commission_types (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    base_price DECIMAL(10, 2) NOT NULL,
    currency VARCHAR(16) DEFAULT 'USD',
    delivery_days INTEGER,
    examples VARCHAR(512)[],
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS commission_orders (
    id SERIAL PRIMARY KEY,
    commission_type_id INTEGER NOT NULL REFERENCES commission_types(id) ON DELETE CASCADE,
    buyer_id VARCHAR(128) NOT NULL REFERENCES users(id),
    seller_id VARCHAR(128) NOT NULL REFERENCES users(id),
    requirements TEXT,
    reference_url VARCHAR(512),
    price DECIMAL(10, 2) NOT NULL,
    status order_status DEFAULT 'PENDING',
    progress_updates TEXT[],
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS events (
    id SERIAL PRIMARY KEY,
    title VARCHAR(256) NOT NULL,
    description TEXT,
    location VARCHAR(512),
    organizer_id VARCHAR(128) NOT NULL REFERENCES users(id),
    start_time BIGINT NOT NULL,
    end_time BIGINT NOT NULL,
    max_attendees INTEGER,
    attendee_count INTEGER DEFAULT 0,
    status event_status DEFAULT 'PLANNING',
    images VARCHAR(512)[],
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS event_attendees (
    event_id INTEGER NOT NULL REFERENCES events(id) ON DELETE CASCADE,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (event_id, user_id)
);

CREATE TABLE IF NOT EXISTS messages (
    id SERIAL PRIMARY KEY,
    sender_id VARCHAR(128) NOT NULL REFERENCES users(id),
    receiver_id VARCHAR(128) NOT NULL REFERENCES users(id),
    content TEXT NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

INSERT INTO badges (name, icon, description) VALUES
('新兽报到', '🐾', '刚加入社区的新成员'),
('兽师认证', '🎨', '经过认证的绘画师'),
('委托达人', '💼', '完成50笔委托的卖家'),
('毛聚组织者', '🎉', '组织过毛聚活动'),
('兽装拥有者', '🧸', '拥有兽装的毛毛'),
('元老成员', '🏆', '加入社区超过1年'),
('画廊之星', '⭐', '画廊作品获得1000赞'),
('超级粉丝', '💖', '关注超过100人');

INSERT INTO categories (name, description, icon) VALUES
('综合讨论', '关于Furry的各种话题', '💬'),
('兽设展示', '展示和分享你的兽设', '🎨'),
('绘图画廊', '艺术作品展示区', '🖼️'),
('写作创作', '小说和故事创作', '📝'),
('毛聚活动', '各地线下聚会信息', '🎉'),
('技术交流', '编程和技术讨论', '💻'),
('音乐分享', 'Furry相关音乐', '🎵'),
('委托交易', '约稿和接稿信息板', '💼'),
('兽装制作', '兽装相关讨论和展示', '🧸');

INSERT INTO tags (name, color) VALUES
('狼', '#6366F1'),
('狐狸', '#F59E0B'),
('龙', '#EF4444'),
('猫', '#8B5CF6'),
('犬', '#10B981'),
('虎', '#F97316'),
('兔', '#EC4899'),
('鹿', '#14B8A6'),
('原创', '#3B82F6'),
('委托', '#EC4899'),
('免费', '#14B8A6'),
('兽装', '#F97316'),
('NSFW', '#EF4444');

CREATE TABLE IF NOT EXISTS notifications (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    type VARCHAR(32) NOT NULL,
    actor_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    related_id BIGINT,
    related_type VARCHAR(32),
    title VARCHAR(256),
    content TEXT,
    is_read BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_notifications_user_id ON notifications(user_id);
CREATE INDEX idx_notifications_unread ON notifications(user_id, is_read);

CREATE TABLE IF NOT EXISTS favorites (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    target_id BIGINT NOT NULL,
    target_type VARCHAR(32) NOT NULL,
    title VARCHAR(256),
    thumbnail VARCHAR(512),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(user_id, target_id, target_type)
);

CREATE INDEX idx_favorites_user_id ON favorites(user_id);

CREATE TABLE IF NOT EXISTS reports (
    id SERIAL PRIMARY KEY,
    reporter_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    target_type VARCHAR(32) NOT NULL,
    target_id BIGINT NOT NULL,
    type VARCHAR(32) NOT NULL,
    description TEXT,
    status VARCHAR(32) DEFAULT 'PENDING',
    handled_by VARCHAR(128) REFERENCES users(id),
    handle_note TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    handled_at BIGINT
);

CREATE INDEX idx_reports_status ON reports(status);

CREATE TABLE IF NOT EXISTS user_blocks (
    id SERIAL PRIMARY KEY,
    blocker_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    blocked_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(blocker_id, blocked_id)
);

CREATE INDEX idx_user_blocks_blocker ON user_blocks(blocker_id);

CREATE TABLE IF NOT EXISTS drafts (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    type VARCHAR(32) NOT NULL DEFAULT 'post',
    title VARCHAR(256),
    content TEXT,
    category_id INTEGER REFERENCES categories(id),
    tags VARCHAR(128)[],
    metadata JSONB DEFAULT '{}'::JSONB,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_drafts_user_id ON drafts(user_id);

CREATE TABLE IF NOT EXISTS user_points (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    points INTEGER DEFAULT 0,
    level INTEGER DEFAULT 1,
    last_check_in DATE,
    consecutive_check_ins INTEGER DEFAULT 0,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS point_transactions (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    type VARCHAR(32) NOT NULL,
    amount INTEGER NOT NULL,
    description VARCHAR(256),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_point_transactions_user_id ON point_transactions(user_id);

INSERT INTO user_points (user_id, points, level)
SELECT id, 0, 1 FROM users
WHERE NOT EXISTS (SELECT 1 FROM user_points WHERE user_points.user_id = users.id);

CREATE TABLE IF NOT EXISTS friend_requests (
    id SERIAL PRIMARY KEY,
    sender_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    receiver_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    message VARCHAR(512),
    status VARCHAR(32) DEFAULT 'PENDING',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(sender_id, receiver_id)
);

CREATE INDEX idx_friend_requests_receiver ON friend_requests(receiver_id);
CREATE INDEX idx_friend_requests_sender ON friend_requests(sender_id);

CREATE TABLE IF NOT EXISTS friendships (
    id SERIAL PRIMARY KEY,
    user1_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    user2_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(user1_id, user2_id)
);

CREATE INDEX idx_friendships_user1 ON friendships(user1_id);
CREATE INDEX idx_friendships_user2 ON friendships(user2_id);

CREATE TABLE IF NOT EXISTS mentions (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    mentioned_by VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    comment_id BIGINT REFERENCES comments(id) ON DELETE CASCADE,
    is_read BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_mentions_user_id ON mentions(user_id);

CREATE TABLE IF NOT EXISTS sensitive_words (
    id SERIAL PRIMARY KEY,
    word VARCHAR(128) NOT NULL UNIQUE,
    level INTEGER DEFAULT 1,
    replacement VARCHAR(128) DEFAULT '***',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS audit_logs (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    action VARCHAR(64) NOT NULL,
    ip VARCHAR(64),
    details TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_audit_logs_user_id ON audit_logs(user_id);
CREATE INDEX idx_audit_logs_action ON audit_logs(action);

INSERT INTO sensitive_words (word, level, replacement) VALUES
('脏话1', 2, '***'),
('脏话2', 2, '***'),
('广告', 1, '**');

CREATE TABLE IF NOT EXISTS open_apps (
    id SERIAL PRIMARY KEY,
    owner_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    icon VARCHAR(512),
    website VARCHAR(512),
    callback_url VARCHAR(512),
    client_id VARCHAR(64) UNIQUE NOT NULL,
    client_secret VARCHAR(128) NOT NULL,
    scopes VARCHAR(64)[] DEFAULT '{}',
    is_active BOOLEAN DEFAULT TRUE,
    rate_limit INTEGER DEFAULT 1000,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_open_apps_owner ON open_apps(owner_id);
CREATE INDEX idx_open_apps_client_id ON open_apps(client_id);

CREATE TABLE IF NOT EXISTS api_stats (
    id SERIAL PRIMARY KEY,
    client_id VARCHAR(64) NOT NULL,
    endpoint VARCHAR(128) NOT NULL,
    call_count INTEGER DEFAULT 1,
    call_date DATE NOT NULL DEFAULT CURRENT_DATE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(client_id, endpoint, call_date)
);

CREATE INDEX idx_api_stats_client_date ON api_stats(client_id, call_date);

CREATE TABLE IF NOT EXISTS webhooks (
    id SERIAL PRIMARY KEY,
    app_id INTEGER NOT NULL REFERENCES open_apps(id) ON DELETE CASCADE,
    endpoint VARCHAR(512) NOT NULL,
    events VARCHAR(64)[] DEFAULT '{}',
    secret VARCHAR(128) NOT NULL,
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_webhooks_app_id ON webhooks(app_id);

CREATE TABLE IF NOT EXISTS announcements (
    id SERIAL PRIMARY KEY,
    title VARCHAR(256) NOT NULL,
    content TEXT,
    type VARCHAR(32) DEFAULT 'normal',
    is_active BOOLEAN DEFAULT TRUE,
    expire_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_announcements_active ON announcements(is_active);

CREATE TABLE IF NOT EXISTS banners (
    id SERIAL PRIMARY KEY,
    title VARCHAR(128),
    image VARCHAR(512) NOT NULL,
    link VARCHAR(512),
    "order" INTEGER DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_banners_active_order ON banners(is_active, "order");

INSERT INTO announcements (title, content, type) VALUES
('欢迎来到FurBBS', '这是一个毛茸茸的Furry文化社区！', 'system'),
('社区规范', '请遵守社区规范，共同维护友好的社区环境', 'important');

INSERT INTO banners (title, image, link, "order") VALUES
('兽设画廊', 'https://example.com/banner1.jpg', '/gallery', 1),
('委托交易', 'https://example.com/banner2.jpg', '/commissions', 2),
('线下兽聚', 'https://example.com/banner3.jpg', '/events', 3);

CREATE TABLE IF NOT EXISTS files (
    id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    filename VARCHAR(512) NOT NULL,
    original_name VARCHAR(512) NOT NULL,
    mime_type VARCHAR(128),
    size BIGINT NOT NULL,
    storage_type VARCHAR(32) DEFAULT 'local',
    path VARCHAR(1024) NOT NULL,
    url VARCHAR(1024),
    purpose VARCHAR(64),
    hash VARCHAR(128),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_files_user_id ON files(user_id);
CREATE INDEX idx_files_purpose ON files(purpose);

CREATE TABLE IF NOT EXISTS email_codes (
    id SERIAL PRIMARY KEY,
    email VARCHAR(255) NOT NULL,
    code VARCHAR(16) NOT NULL,
    type VARCHAR(32) NOT NULL,
    expire_at BIGINT NOT NULL,
    used BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_email_codes_email ON email_codes(email);
CREATE INDEX idx_email_codes_expire ON email_codes(expire_at);

CREATE TABLE IF NOT EXISTS distributed_locks (
    key VARCHAR(128) PRIMARY KEY,
    owner VARCHAR(128) NOT NULL,
    expire_at BIGINT NOT NULL,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS scheduled_tasks (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    cron VARCHAR(64) NOT NULL,
    last_run BIGINT,
    next_run BIGINT,
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE UNIQUE INDEX idx_scheduled_tasks_name ON scheduled_tasks(name);

CREATE TABLE IF NOT EXISTS i18n_translations (
    id SERIAL PRIMARY KEY,
    lang VARCHAR(8) NOT NULL DEFAULT 'zh-CN',
    key VARCHAR(256) NOT NULL,
    value TEXT NOT NULL,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(lang, key)
);

CREATE INDEX idx_i18n_lang ON i18n_translations(lang);

INSERT INTO i18n_translations (lang, key, value) VALUES
('zh-CN', 'welcome', '欢迎来到FurBBS'),
('zh-CN', 'email_code_subject', '您的验证码'),
('zh-CN', 'email_code_content', '您的验证码是：{code}，有效期{minutes}分钟'),
('en', 'welcome', 'Welcome to FurBBS'),
('en', 'email_code_subject', 'Your verification code'),
('en', 'email_code_content', 'Your verification code is: {code}, valid for {minutes} minutes');
