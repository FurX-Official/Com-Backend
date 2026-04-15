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

CREATE TABLE IF NOT EXISTS forum_sections (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    icon VARCHAR(512),
    sort_order INT DEFAULT 0,
    parent_id INT REFERENCES forum_sections(id) ON DELETE CASCADE,
    post_count INT DEFAULT 0,
    thread_count INT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_forum_sections_parent ON forum_sections(parent_id);
CREATE INDEX idx_forum_sections_order ON forum_sections(sort_order);

CREATE TABLE IF NOT EXISTS section_moderators (
    section_id INT REFERENCES forum_sections(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (section_id, user_id)
);

CREATE TABLE IF NOT EXISTS tags (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    color VARCHAR(32) DEFAULT '#3B82F6',
    use_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS post_tags (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    tag_id INT REFERENCES tags(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (post_id, tag_id)
);

CREATE INDEX idx_post_tags_post ON post_tags(post_id);
CREATE INDEX idx_post_tags_tag ON post_tags(tag_id);

CREATE TABLE IF NOT EXISTS badges (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    description TEXT,
    icon VARCHAR(512),
    rarity VARCHAR(32) DEFAULT 'common',
    requirement_type INT DEFAULT 0,
    requirement_value INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS user_badges (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    badge_id INT REFERENCES badges(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, badge_id)
);

CREATE INDEX idx_user_badges_user ON user_badges(user_id);

CREATE TABLE IF NOT EXISTS user_online_status (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    is_online BOOLEAN DEFAULT FALSE,
    last_active BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    status_text VARCHAR(256)
);

CREATE TABLE IF NOT EXISTS reading_history (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    read_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE UNIQUE INDEX idx_reading_history_unique ON reading_history(user_id, post_id);
CREATE INDEX idx_reading_history_user ON reading_history(user_id);
CREATE INDEX idx_reading_history_read_at ON reading_history(read_at DESC);

CREATE TABLE IF NOT EXISTS forum_statistics (
    id SERIAL PRIMARY KEY,
    stat_date DATE UNIQUE,
    new_users INT DEFAULT 0,
    new_posts INT DEFAULT 0,
    new_comments INT DEFAULT 0,
    max_online_users INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS polls (
    id SERIAL PRIMARY KEY,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    question VARCHAR(512) NOT NULL,
    is_multiple BOOLEAN DEFAULT FALSE,
    end_time BIGINT,
    is_closed BOOLEAN DEFAULT FALSE,
    total_votes INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_polls_post ON polls(post_id);

CREATE TABLE IF NOT EXISTS poll_options (
    id SERIAL PRIMARY KEY,
    poll_id INT REFERENCES polls(id) ON DELETE CASCADE,
    text VARCHAR(512) NOT NULL,
    vote_count INT DEFAULT 0
);

CREATE INDEX idx_poll_options_poll ON poll_options(poll_id);

CREATE TABLE IF NOT EXISTS poll_votes (
    poll_id INT REFERENCES polls(id) ON DELETE CASCADE,
    option_id INT REFERENCES poll_options(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    voted_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (poll_id, user_id, option_id)
);

CREATE INDEX idx_poll_votes_user ON poll_votes(user_id);

CREATE TABLE IF NOT EXISTS gifts (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    image VARCHAR(512),
    price INT DEFAULT 0,
    rarity VARCHAR(32) DEFAULT 'common',
    is_available BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS user_gifts (
    id SERIAL PRIMARY KEY,
    from_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    to_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE SET NULL,
    gift_id INT REFERENCES gifts(id) ON DELETE CASCADE,
    quantity INT DEFAULT 1,
    message TEXT,
    is_anonymous BOOLEAN DEFAULT FALSE,
    sent_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_user_gifts_to ON user_gifts(to_user_id);
CREATE INDEX idx_user_gifts_post ON user_gifts(post_id);

INSERT INTO forum_sections (name, description, icon, sort_order) VALUES
('综合讨论', '所有关于Furry文化的综合讨论', '🌍', 1),
('兽设展示', '展示和分享你的兽设作品', '🐺', 2),
('艺术画廊', 'Furry相关的艺术作品交流', '🎨', 3),
('委托交易', '接稿和约稿的交易专区', '💼', 4),
('线下兽聚', '各地兽聚活动信息发布', '🎉', 5),
('技术交流', '编程、绘画等技术讨论', '💻', 6),
('站务公告', '论坛公告和事务处理', '📢', 7);

INSERT INTO tags (name, color, use_count) VALUES
('兽设', '#FF6B6B', 0),
('绘画', '#4ECDC4', 0),
('委托', '#45B7D1', 0),
('兽聚', '#96CEB4', 0),
('教程', '#FFEAA7', 0),
('求助', '#DDA0DD', 0),
('分享', '#74B9FF', 0),
('讨论', '#00B894', 0);

INSERT INTO badges (name, description, icon, rarity, requirement_type, requirement_value) VALUES
('新手上路', '注册并完成首次发帖', '🌟', 'common', 1, 1),
('活跃用户', '发布100篇帖子', '⭐', 'uncommon', 1, 100),
('论坛元老', '注册满一年', '👑', 'rare', 2, 365),
('艺术大师', '上传50张艺术作品', '🎨', 'epic', 3, 50),
('人气王', '获得1000个点赞', '💖', 'legendary', 4, 1000),
('签到达人', '连续签到30天', '📅', 'rare', 5, 30),
('乐于助人', '帮助解决100个问题', '🤝', 'uncommon', 6, 100);

INSERT INTO gifts (name, description, image, price, rarity) VALUES
('爱心', '送给TA一颗暖暖的爱心', '❤️', 10, 'common'),
('鲜花', '美丽的鲜花献给你', '🌸', 50, 'common'),
('鸡腿', '美味的鸡腿', '🍗', 100, 'common'),
('皇冠', '尊贵的皇冠', '👑', 500, 'rare'),
('钻石', '闪耀的钻石', '💎', 1000, 'epic'),
('彩虹', '美丽的彩虹', '🌈', 2000, 'legendary');

CREATE TABLE IF NOT EXISTS user_bans (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    moderator_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    reason TEXT NOT NULL,
    type VARCHAR(32) NOT NULL,
    expire_at BIGINT,
    is_permanent BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_user_bans_user ON user_bans(user_id);
CREATE INDEX idx_user_bans_expire ON user_bans(expire_at);

CREATE TABLE IF NOT EXISTS moderator_logs (
    id SERIAL PRIMARY KEY,
    moderator_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    action VARCHAR(64) NOT NULL,
    target_id BIGINT,
    target_type VARCHAR(32),
    reason TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_mod_logs_moderator ON moderator_logs(moderator_id);
CREATE INDEX idx_mod_logs_action ON moderator_logs(action);
CREATE INDEX idx_mod_logs_created ON moderator_logs(created_at DESC);

CREATE TABLE IF NOT EXISTS shop_items (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    image VARCHAR(512),
    type VARCHAR(32) NOT NULL,
    price INT DEFAULT 0,
    stock INT DEFAULT -1,
    is_available BOOLEAN DEFAULT TRUE,
    properties JSONB DEFAULT '{}'::JSONB,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS user_inventory (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    item_id INT REFERENCES shop_items(id) ON DELETE CASCADE,
    quantity INT DEFAULT 1,
    is_used BOOLEAN DEFAULT FALSE,
    acquired_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_inventory_user ON user_inventory(user_id);
CREATE UNIQUE INDEX idx_inventory_unique ON user_inventory(user_id, item_id, is_used);

CREATE TABLE IF NOT EXISTS avatar_frames (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    image_url VARCHAR(512) NOT NULL,
    rarity VARCHAR(32) DEFAULT 'common',
    price INT DEFAULT 0,
    is_available BOOLEAN DEFAULT TRUE
);

ALTER TABLE users ADD COLUMN IF NOT EXISTS signature TEXT;
ALTER TABLE users ADD COLUMN IF NOT EXISTS active_avatar_frame_id INT REFERENCES avatar_frames(id);
ALTER TABLE users ADD COLUMN IF NOT EXISTS consecutive_checkins INT DEFAULT 0;
ALTER TABLE users ADD COLUMN IF NOT EXISTS max_consecutive_checkins INT DEFAULT 0;
ALTER TABLE users ADD COLUMN IF NOT EXISTS last_checkin_date DATE;

CREATE TABLE IF NOT EXISTS favorite_folders (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    is_public BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_favorite_folders_user ON favorite_folders(user_id);

ALTER TABLE favorites ADD COLUMN IF NOT EXISTS folder_id INT REFERENCES favorite_folders(id);

CREATE TABLE IF NOT EXISTS user_subscriptions (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    target_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, target_user_id)
);

CREATE INDEX idx_subscriptions_user ON user_subscriptions(user_id);
CREATE INDEX idx_subscriptions_target ON user_subscriptions(target_user_id);

CREATE TABLE IF NOT EXISTS daily_tasks (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    reward_points INT DEFAULT 0,
    target_value INT DEFAULT 1,
    task_type VARCHAR(32) NOT NULL
);

CREATE TABLE IF NOT EXISTS user_task_progress (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    task_id INT REFERENCES daily_tasks(id) ON DELETE CASCADE,
    progress INT DEFAULT 0,
    is_completed BOOLEAN DEFAULT FALSE,
    task_date DATE NOT NULL,
    completed_at BIGINT,
    PRIMARY KEY (user_id, task_id, task_date)
);

INSERT INTO shop_items (name, description, type, price, stock) VALUES
('改名卡', '可以修改一次用户名', 'name_change', 500, -1),
('头像框-金色', '尊贵的金色头像框', 'avatar_frame', 1000, -1),
('头像框-彩虹', '绚丽的彩虹头像框', 'avatar_frame', 2000, -1),
('置顶卡', '将帖子置顶24小时', 'pin_post', 300, -1),
('精华卡', '将帖子设为精华', 'essence_post', 800, -1);

INSERT INTO avatar_frames (name, image_url, rarity, price) VALUES
('默认边框', 'https://example.com/frames/default.png', 'common', 0),
('金色边框', 'https://example.com/frames/gold.png', 'rare', 1000),
('彩虹边框', 'https://example.com/frames/rainbow.png', 'epic', 2000),
('钻石边框', 'https://example.com/frames/diamond.png', 'legendary', 5000);

INSERT INTO daily_tasks (name, description, reward_points, target_value, task_type) VALUES
('每日签到', '每日签到获得积分', 10, 1, 'checkin'),
('发布帖子', '发布一篇帖子', 5, 1, 'create_post'),
('发表评论', '发表3条评论', 3, 3, 'create_comment'),
('点赞内容', '点赞5个帖子', 2, 5, 'like_post'),
('访问论坛', '访问论坛', 1, 1, 'visit');

INSERT INTO favorite_folders (user_id, name, is_public) VALUES
('default', '默认收藏夹', TRUE);

CREATE TABLE IF NOT EXISTS private_messages (
    id SERIAL PRIMARY KEY,
    from_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    to_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    content TEXT NOT NULL,
    is_read BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_pm_from ON private_messages(from_user_id);
CREATE INDEX idx_pm_to ON private_messages(to_user_id);
CREATE INDEX idx_pm_conversation ON private_messages(from_user_id, to_user_id);
CREATE INDEX idx_pm_created ON private_messages(created_at DESC);

CREATE TABLE IF NOT EXISTS invite_codes (
    code VARCHAR(32) PRIMARY KEY,
    creator_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    used_by_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    is_used BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    used_at BIGINT,
    reward_points INT DEFAULT 100
);

CREATE INDEX idx_invite_creator ON invite_codes(creator_id);

ALTER TABLE posts ADD COLUMN IF NOT EXISTS moderation_status INT DEFAULT 1;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS risk_level VARCHAR(32) DEFAULT 'low';
ALTER TABLE posts ADD COLUMN IF NOT EXISTS reviewed_by VARCHAR(128) REFERENCES users(id);
ALTER TABLE posts ADD COLUMN IF NOT EXISTS reviewed_at BIGINT;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS review_reason TEXT;

CREATE TABLE IF NOT EXISTS user_activity_logs (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    action_type VARCHAR(32) NOT NULL,
    target_id BIGINT,
    ip_address VARCHAR(64),
    user_agent TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_activity_user ON user_activity_logs(user_id);
CREATE INDEX idx_activity_action ON user_activity_logs(action_type);
CREATE INDEX idx_activity_created ON user_activity_logs(created_at DESC);

CREATE TABLE IF NOT EXISTS lucky_draw_rewards (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    image VARCHAR(512),
    points INT DEFAULT 0,
    item_id INT REFERENCES shop_items(id),
    rarity VARCHAR(32) DEFAULT 'common',
    probability INT DEFAULT 100,
    is_available BOOLEAN DEFAULT TRUE
);

INSERT INTO lucky_draw_rewards (name, image, points, rarity, probability) VALUES
('谢谢参与', '🍀', 0, 'common', 5000),
('10积分', '✨', 10, 'common', 2500),
('50积分', '🌟', 50, 'uncommon', 1500),
('100积分', '💫', 100, 'rare', 700),
('500积分', '⭐', 500, 'epic', 250),
('1000积分', '👑', 1000, 'legendary', 50);

ALTER TABLE users ADD COLUMN IF NOT EXISTS lucky_draws_today INT DEFAULT 0;
ALTER TABLE users ADD COLUMN IF NOT EXISTS last_lucky_draw_date DATE;

CREATE TABLE IF NOT EXISTS checkin_records (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    checkin_date DATE NOT NULL,
    is_repaired BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, checkin_date)
);

CREATE INDEX idx_checkin_user ON checkin_records(user_id);

CREATE TABLE IF NOT EXISTS faqs (
    id SERIAL PRIMARY KEY,
    question VARCHAR(512) NOT NULL,
    answer TEXT NOT NULL,
    category VARCHAR(64) DEFAULT 'general',
    sort_order INT DEFAULT 0,
    view_count INT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_faqs_category ON faqs(category);
CREATE INDEX idx_faqs_active ON faqs(is_active);

INSERT INTO faqs (question, answer, category, sort_order) VALUES
('如何注册账号？', '点击首页右上角的注册按钮，按照提示填写信息即可完成注册。', '账号', 1),
('如何找回密码？', '点击登录页面的忘记密码，通过邮箱验证即可重置密码。', '账号', 2),
('如何发布帖子？', '登录后点击发帖按钮，选择对应版块，填写标题和内容即可发布。', '发帖', 1),
('如何上传头像？', '进入个人设置页面，点击头像区域即可上传新头像。', '个人资料', 1),
('积分有什么用？', '积分可用于商店兑换物品、抽奖、购买道具等。', '积分', 1);

CREATE TABLE IF NOT EXISTS help_articles (
    id SERIAL PRIMARY KEY,
    title VARCHAR(256) NOT NULL,
    content TEXT NOT NULL,
    summary VARCHAR(512),
    category VARCHAR(64) DEFAULT 'general',
    cover_image VARCHAR(512),
    view_count INT DEFAULT 0,
    like_count INT DEFAULT 0,
    is_published BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT
);

CREATE INDEX idx_help_category ON help_articles(category);
CREATE INDEX idx_help_published ON help_articles(is_published);

INSERT INTO help_articles (title, content, summary, category) VALUES
('新手指南', '欢迎加入我们的社区！本教程将带你了解社区的基本功能...', '快速了解社区的各项功能', '入门'),
('论坛使用规则', '请所有用户遵守以下规则：1. 尊重他人 2. 禁止广告 3. 禁止违规内容...', '社区用户必须遵守的行为规范', '规则');

CREATE TABLE IF NOT EXISTS feedbacks (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    type VARCHAR(32) NOT NULL,
    title VARCHAR(256) NOT NULL,
    content TEXT NOT NULL,
    images TEXT[],
    contact VARCHAR(128),
    status INT DEFAULT 0,
    admin_reply TEXT,
    replied_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_feedback_user ON feedbacks(user_id);
CREATE INDEX idx_feedback_status ON feedbacks(status);

CREATE TABLE IF NOT EXISTS advertisements (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    position VARCHAR(64) NOT NULL,
    image_url VARCHAR(512) NOT NULL,
    link_url VARCHAR(512),
    sort_order INT DEFAULT 0,
    start_time BIGINT,
    end_time BIGINT,
    is_active BOOLEAN DEFAULT TRUE,
    click_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_ad_position ON advertisements(position);
CREATE INDEX idx_ad_active ON advertisements(is_active);

INSERT INTO advertisements (name, position, image_url, link_url, sort_order) VALUES
('首页横幅1', 'banner', 'https://example.com/banner1.jpg', 'https://example.com', 1),
('侧边栏广告', 'sidebar', 'https://example.com/sidebar.jpg', 'https://example.com', 1);

CREATE TABLE IF NOT EXISTS friend_links (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    url VARCHAR(512) NOT NULL,
    logo VARCHAR(512),
    description VARCHAR(512),
    sort_order INT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_fl_active ON friend_links(is_active);

INSERT INTO friend_links (name, url, logo, description, sort_order) VALUES
('Furry社区', 'https://example.com', 'https://example.com/logo.png', '友好的Furry交流社区', 1);

CREATE TABLE IF NOT EXISTS captcha_settings (
    id SERIAL PRIMARY KEY,
    provider VARCHAR(32) NOT NULL,
    site_key VARCHAR(256) NOT NULL,
    secret_key VARCHAR(256) NOT NULL,
    is_active BOOLEAN DEFAULT FALSE,
    verify_url VARCHAR(512) NOT NULL,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE UNIQUE INDEX idx_captcha_provider ON captcha_settings(provider);

INSERT INTO captcha_settings (provider, site_key, secret_key, is_active, verify_url) VALUES
('recaptcha_v2', 'your-site-key', 'your-secret-key', false, 'https://www.google.com/recaptcha/api/siteverify'),
('recaptcha_v3', 'your-site-key', 'your-secret-key', false, 'https://www.google.com/recaptcha/api/siteverify'),
('hcaptcha', 'your-site-key', 'your-secret-key', false, 'https://hcaptcha.com/siteverify'),
('geetest', 'your-id', 'your-key', false, 'https://gcaptcha4.geetest.com/validate');

CREATE TABLE IF NOT EXISTS content_reports (
    id SERIAL PRIMARY KEY,
    reporter_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    type INT DEFAULT 0,
    target_type VARCHAR(32) NOT NULL,
    target_id BIGINT NOT NULL,
    reason TEXT,
    evidence TEXT[],
    status INT DEFAULT 0,
    handler_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    handler_note TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    handled_at BIGINT
);

CREATE INDEX idx_report_reporter ON content_reports(reporter_id);
CREATE INDEX idx_report_status ON content_reports(status);
CREATE INDEX idx_report_target ON content_reports(target_type, target_id);

CREATE TABLE IF NOT EXISTS moderation_actions (
    id SERIAL PRIMARY KEY,
    moderator_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    action_type VARCHAR(64) NOT NULL,
    target_type VARCHAR(32) NOT NULL,
    target_id BIGINT NOT NULL,
    reason TEXT,
    duration BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_mod_moderator ON moderation_actions(moderator_id);
CREATE INDEX idx_mod_action ON moderation_actions(action_type);
CREATE INDEX idx_mod_target ON moderation_actions(target_type, target_id);
