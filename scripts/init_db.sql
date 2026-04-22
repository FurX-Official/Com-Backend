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

CREATE TABLE IF NOT EXISTS membership_plans (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    tier INT NOT NULL,
    price_monthly INT NOT NULL,
    price_yearly INT NOT NULL,
    benefits TEXT[],
    description TEXT,
    discount_percent INT DEFAULT 0,
    is_popular BOOLEAN DEFAULT FALSE,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO membership_plans (name, tier, price_monthly, price_yearly, benefits, description, discount_percent, is_popular) VALUES
('免费会员', 0, 0, 0, ARRAY['基础发帖功能', '评论功能', '普通头像'], '免费用户基础功能', 0, FALSE),
('青铜会员', 1, 990, 9980, ARRAY['青铜专属勋章', '每日积分+50%', '自定义头像框', '发帖免审核'], '解锁更多实用功能', 0, FALSE),
('白银会员', 2, 1990, 19980, ARRAY['白银专属勋章', '每日积分+100%', '专属皮肤', '无广告', '超大空间'], '享受优质体验', 0, TRUE),
('黄金会员', 3, 3990, 39980, ARRAY['黄金专属勋章', '每日积分+200%', '全皮肤解锁', '昵称高亮', '专属客服'], '尊贵黄金体验', 0, FALSE),
('铂金会员', 4, 7990, 79980, ARRAY['铂金专属勋章', '每日积分+300%', '专属身份标识', '优先推荐', '所有功能解锁'], '至尊体验', 0, FALSE);

CREATE TABLE IF NOT EXISTS user_memberships (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    tier INT DEFAULT 0,
    start_date BIGINT,
    expiry_date BIGINT,
    auto_renew BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS membership_orders (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    plan_id INT REFERENCES membership_plans(id),
    amount INT NOT NULL,
    payment_method VARCHAR(32),
    status INT DEFAULT 0,
    transaction_id VARCHAR(256),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    paid_at BIGINT
);

CREATE INDEX idx_membership_user ON user_memberships(user_id);
CREATE INDEX idx_order_user ON membership_orders(user_id);

CREATE TABLE IF NOT EXISTS achievements (
    id SERIAL PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    description VARCHAR(512) NOT NULL,
    icon VARCHAR(512),
    rarity INT DEFAULT 1,
    points_reward INT DEFAULT 0,
    requirement_type VARCHAR(64) NOT NULL,
    requirement_value INT NOT NULL,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO achievements (name, description, icon, rarity, points_reward, requirement_type, requirement_value) VALUES
('初出茅庐', '发布第一篇帖子', 'achievement_first_post.png', 1, 50, 'posts', 1),
('活跃用户', '发布10篇帖子', 'achievement_active.png', 2, 200, 'posts', 10),
('社区达人', '发布100篇帖子', 'achievement_expert.png', 3, 1000, 'posts', 100),
('善于交流', '发表100条评论', 'achievement_chatty.png', 2, 200, 'comments', 100),
('万人迷', '获得1000个赞', 'achievement_popular.png', 4, 2000, 'likes_received', 1000),
('签到达人', '连续签到30天', 'achievement_checkin.png', 3, 500, 'consecutive_checkin', 30),
('元老', '注册满1年', 'achievement_veteran.png', 4, 1000, 'days_registered', 365),
('乐于助人', '获得100次最佳答案', 'achievement_helpful.png', 3, 500, 'best_answer', 100);

CREATE TABLE IF NOT EXISTS user_achievements (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    achievement_id INT REFERENCES achievements(id),
    unlocked_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, achievement_id)
);

CREATE INDEX idx_user_achievement ON user_achievements(user_id, achievement_id);

CREATE TABLE IF NOT EXISTS redeem_cards (
    id SERIAL PRIMARY KEY,
    code VARCHAR(64) UNIQUE NOT NULL,
    type INT NOT NULL,
    value INT DEFAULT 0,
    item_id VARCHAR(64),
    item_name VARCHAR(128),
    status INT DEFAULT 0,
    max_uses INT DEFAULT 1,
    used_count INT DEFAULT 0,
    expiry_date BIGINT,
    creator_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    used_by_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    used_at BIGINT,
    batch_no VARCHAR(64)
);

CREATE UNIQUE INDEX idx_redeem_code ON redeem_cards(code);
CREATE INDEX idx_redeem_status ON redeem_cards(status);
CREATE INDEX idx_redeem_type ON redeem_cards(type);
CREATE INDEX idx_redeem_batch ON redeem_cards(batch_no);

CREATE TABLE IF NOT EXISTS user_titles (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    color VARCHAR(32) DEFAULT '#FFFFFF',
    bg_color VARCHAR(32),
    icon VARCHAR(512),
    rarity INT DEFAULT 1,
    is_animated BOOLEAN DEFAULT FALSE,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO user_titles (name, color, bg_color, icon, rarity, is_animated) VALUES
('萌新', '#95E1D3', NULL, NULL, 1, FALSE),
('社区达人', '#F38181', NULL, NULL, 2, FALSE),
('大佬', '#AA96DA', NULL, NULL, 3, FALSE),
('元老', '#FFD93D', NULL, NULL, 4, FALSE),
('传奇', '#FF6B6B', 'linear-gradient(45deg, #FF6B6B, #4ECDC4)', NULL, 5, TRUE),
('管理员', '#E74C3C', NULL, '👑', 1, FALSE),
('版主', '#3498DB', NULL, '🛡️', 1, FALSE);

CREATE TABLE IF NOT EXISTS user_owned_titles (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    title_id INT REFERENCES user_titles(id),
    obtained_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, title_id)
);

CREATE TABLE IF NOT EXISTS user_active_title (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    title_id INT REFERENCES user_titles(id),
    updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS avatar_frames (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    image_url VARCHAR(512) NOT NULL,
    rarity INT DEFAULT 1,
    is_animated BOOLEAN DEFAULT FALSE,
    price INT DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO avatar_frames (name, image_url, rarity, is_animated, price) VALUES
('默认边框', 'frames/default.png', 1, FALSE, 0),
('金色边框', 'frames/gold.png', 2, FALSE, 1000),
('钻石边框', 'frames/diamond.png', 3, FALSE, 3000),
('彩虹边框', 'frames/rainbow.png', 4, TRUE, 5000),
('传奇边框', 'frames/legendary.png', 5, TRUE, 10000);

CREATE TABLE IF NOT EXISTS user_owned_frames (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    frame_id INT REFERENCES avatar_frames(id),
    obtained_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, frame_id)
);

CREATE TABLE IF NOT EXISTS user_active_frame (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    frame_id INT REFERENCES avatar_frames(id),
    updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS nameplate_styles (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    card_bg VARCHAR(512),
    text_color VARCHAR(32),
    effect VARCHAR(128),
    rarity INT DEFAULT 1,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO nameplate_styles (name, card_bg, text_color, effect, rarity) VALUES
('简约风格', NULL, '#333333', NULL, 1),
('暗夜风格', '#1a1a2e', '#e0e0e0', NULL, 2),
('渐变风格', 'linear-gradient(135deg, #667eea 0%, #764ba2 100%)', '#ffffff', 'glow', 3),
('节日风格', '#ff6b9d', '#ffffff', 'sparkle', 4);

CREATE TABLE IF NOT EXISTS profile_themes (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    bg_image VARCHAR(512),
    primary_color VARCHAR(32),
    secondary_color VARCHAR(32),
    is_premium BOOLEAN DEFAULT FALSE,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO profile_themes (name, bg_image, primary_color, secondary_color, is_premium) VALUES
('默认主题', NULL, '#6366f1', '#8b5cf6', FALSE),
('森林主题', 'themes/forest.jpg', '#22c55e', '#16a34a', FALSE),
('海洋主题', 'themes/ocean.jpg', '#0ea5e9', '#0284c7', FALSE),
('夕阳主题', 'themes/sunset.jpg', '#f97316', '#ea580c', TRUE),
('星空主题', 'themes/galaxy.jpg', '#8b5cf6', '#7c3aed', TRUE);

CREATE TABLE IF NOT EXISTS user_customization (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    active_nameplate_id INT DEFAULT 1,
    active_theme_id INT DEFAULT 1,
    owned_nameplates INT[] DEFAULT '{1}',
    owned_themes INT[] DEFAULT '{1,2,3}',
    updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS shop_items (
    id SERIAL PRIMARY KEY,
    type INT NOT NULL,
    item_id INT NOT NULL,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    price INT NOT NULL,
    discount_price INT,
    stock INT DEFAULT -1,
    sales INT DEFAULT 0,
    is_hot BOOLEAN DEFAULT FALSE,
    is_new BOOLEAN DEFAULT FALSE,
    start_time BIGINT,
    end_time BIGINT,
    tags VARCHAR(64)[],
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO shop_items (type, item_id, name, description, price, discount_price, is_hot, is_new, tags) VALUES
(1, 2, '金色头像框', '尊贵金色边框，彰显贵族气质', 1000, 800, TRUE, FALSE, '{"热门","装扮"}'),
(1, 3, '钻石头像框', '闪耀钻石边框，社区达人专属', 3000, 2500, TRUE, FALSE, '{"热门","装扮"}'),
(1, 4, '彩虹头像框', '动态渐变彩虹特效', 5000, 4500, FALSE, TRUE, '{"新品","动态"}'),
(2, 2, '社区达人头衔', '解锁橙色专属头衔', 2000, 1800, FALSE, FALSE, '{"头衔"}'),
(2, 3, '大佬头衔', '解锁紫色专属头衔', 5000, 4500, FALSE, FALSE, '{"头衔"}'),
(3, 2, '暗夜名牌样式', '护眼深色主题名牌', 1500, 1200, FALSE, FALSE, '{"名牌"}'),
(3, 3, '渐变名牌样式', '炫酷渐变发光名牌', 3000, 2800, TRUE, FALSE, '{"热门","动态"}');

CREATE TABLE IF NOT EXISTS purchase_history (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    shop_item_id INT REFERENCES shop_items(id),
    price_paid INT NOT NULL,
    quantity INT DEFAULT 1,
    purchased_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_shop_type ON shop_items(type);
CREATE INDEX idx_shop_active ON shop_items(is_active);

CREATE TABLE IF NOT EXISTS daily_tasks (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    description VARCHAR(256) NOT NULL,
    target_value INT NOT NULL,
    points_reward INT NOT NULL,
    task_type VARCHAR(32) NOT NULL,
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO daily_tasks (name, description, target_value, points_reward, task_type) VALUES
('每日签到', '完成每日签到', 1, 10, 'checkin'),
('发布帖子', '发布1篇新帖子', 1, 20, 'post'),
('发表评论', '发表3条评论', 3, 15, 'comment'),
('点赞内容', '给5个内容点赞', 5, 10, 'like'),
('访问社区', '每日访问社区', 1, 5, 'visit');

CREATE TABLE IF NOT EXISTS user_task_progress (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    task_id INT REFERENCES daily_tasks(id),
    current_value INT DEFAULT 0,
    is_completed BOOLEAN DEFAULT FALSE,
    is_claimed BOOLEAN DEFAULT FALSE,
    last_updated DATE DEFAULT CURRENT_DATE,
    PRIMARY KEY (user_id, task_id, last_updated)
);

CREATE TABLE IF NOT EXISTS user_checkins (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    checkin_date DATE PRIMARY KEY,
    continuous_days INT DEFAULT 1,
    points_earned INT DEFAULT 10,
    is_bonus BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_checkin_user ON user_checkins(user_id, checkin_date);

ALTER TABLE posts ADD COLUMN IF NOT EXISTS is_essence BOOLEAN DEFAULT FALSE;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS essence_level INT DEFAULT 0;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS is_sticky BOOLEAN DEFAULT FALSE;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS sticky_weight INT DEFAULT 0;
ALTER TABLE posts ADD COLUMN IF NOT EXISTS sticky_expiry BIGINT;

CREATE INDEX idx_post_essence ON posts(is_essence);
CREATE INDEX idx_post_sticky ON posts(is_sticky, sticky_weight DESC);

CREATE TABLE IF NOT EXISTS user_follows (
    follower_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    following_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (follower_id, following_id)
);

CREATE INDEX idx_follows_follower ON user_follows(follower_id);
CREATE INDEX idx_follows_following ON user_follows(following_id);

ALTER TABLE users ADD COLUMN IF NOT EXISTS following_count INT DEFAULT 0;
ALTER TABLE users ADD COLUMN IF NOT EXISTS follower_count INT DEFAULT 0;

CREATE TABLE IF NOT EXISTS fursona_favorites (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    fursona_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, fursona_id)
);

CREATE INDEX idx_favorite_fursona ON fursona_favorites(fursona_id);
ALTER TABLE fursonas ADD COLUMN IF NOT EXISTS favorite_count INT DEFAULT 0;

CREATE TABLE IF NOT EXISTS gift_items (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    icon VARCHAR(256),
    price INT NOT NULL,
    animation VARCHAR(256),
    is_animated BOOLEAN DEFAULT FALSE,
    rarity VARCHAR(16) DEFAULT 'common',
    is_active BOOLEAN DEFAULT TRUE
);

INSERT INTO gift_items (name, icon, price, animation, is_animated, rarity) VALUES
('爱心', '❤️', 10, NULL, FALSE, 'common'),
('玫瑰', '🌹', 20, NULL, FALSE, 'common'),
('星星', '⭐', 30, NULL, FALSE, 'common'),
('蛋糕', '🎂', 50, NULL, FALSE, 'uncommon'),
('彩虹', '🌈', 88, 'rainbow_effect', TRUE, 'uncommon'),
('皇冠', '👑', 188, 'crown_effect', TRUE, 'rare'),
('钻石', '💎', 288, 'diamond_shine', TRUE, 'rare'),
('神兽', '🐉', 520, 'dragon_spirit', TRUE, 'legendary'),
('兽装抱枕', '🧸', 1314, 'hug_animation', TRUE, 'legendary');

CREATE TABLE IF NOT EXISTS gift_history (
    id SERIAL PRIMARY KEY,
    from_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    to_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    gift_id INT REFERENCES gift_items(id),
    quantity INT DEFAULT 1,
    total_value INT NOT NULL,
    message TEXT,
    is_anonymous BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_gift_receiver ON gift_history(to_user_id, created_at DESC);
CREATE INDEX idx_gift_sender ON gift_history(from_user_id);

ALTER TABLE users ADD COLUMN IF NOT EXISTS received_gifts_value INT DEFAULT 0;

CREATE TABLE IF NOT EXISTS artist_reviews (
    id SERIAL PRIMARY KEY,
    reviewer_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    artist_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    commission_id BIGINT REFERENCES commissions(id),
    rating INT NOT NULL CHECK (rating BETWEEN 1 AND 5),
    comment TEXT,
    tags VARCHAR(32)[],
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(reviewer_id, commission_id)
);

CREATE INDEX idx_artist_reviews ON artist_reviews(artist_id, created_at DESC);

ALTER TABLE users ADD COLUMN IF NOT EXISTS artist_rating NUMERIC(3,2) DEFAULT 0;
ALTER TABLE users ADD COLUMN IF NOT EXISTS review_count INT DEFAULT 0;

CREATE TABLE IF NOT EXISTS artist_blacklist (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    artist_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    reason TEXT,
    added_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, artist_id)
);

CREATE TABLE IF NOT EXISTS question_boxes (
    id SERIAL PRIMARY KEY,
    owner_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    is_public BOOLEAN DEFAULT TRUE,
    allow_anonymous BOOLEAN DEFAULT TRUE,
    description TEXT,
    question_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_question_box_owner ON question_boxes(owner_id);

CREATE TABLE IF NOT EXISTS box_questions (
    id SERIAL PRIMARY KEY,
    box_id INT REFERENCES question_boxes(id) ON DELETE CASCADE,
    asker_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    is_anonymous BOOLEAN DEFAULT FALSE,
    content TEXT NOT NULL,
    answer TEXT,
    is_answered BOOLEAN DEFAULT FALSE,
    is_public BOOLEAN DEFAULT TRUE,
    asked_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    answered_at BIGINT
);

CREATE INDEX idx_question_box ON box_questions(box_id, is_answered, asked_at DESC);

CREATE TABLE IF NOT EXISTS user_groups (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL UNIQUE,
    description TEXT,
    avatar VARCHAR(256),
    banner VARCHAR(256),
    owner_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    member_count INT DEFAULT 1,
    post_count INT DEFAULT 0,
    is_public BOOLEAN DEFAULT TRUE,
    allow_join_request BOOLEAN DEFAULT TRUE,
    tags VARCHAR(32)[],
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS group_members (
    group_id INT REFERENCES user_groups(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    role INT DEFAULT 0,
    joined_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (group_id, user_id)
);

CREATE INDEX idx_group_member ON group_members(user_id);

CREATE TABLE IF NOT EXISTS group_posts (
    id SERIAL PRIMARY KEY,
    group_id INT REFERENCES user_groups(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_group_posts ON group_posts(group_id, created_at DESC);

CREATE TABLE IF NOT EXISTS event_registrations (
    id SERIAL PRIMARY KEY,
    event_id BIGINT REFERENCES events(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    ticket_type INT DEFAULT 0,
    guest_count INT DEFAULT 0,
    contact VARCHAR(128),
    status INT DEFAULT 0,
    registered_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(event_id, user_id)
);

CREATE INDEX idx_event_reg ON event_registrations(event_id, status);
ALTER TABLE events ADD COLUMN IF NOT EXISTS registration_count INT DEFAULT 0;

CREATE TABLE IF NOT EXISTS mentions (
    id SERIAL PRIMARY KEY,
    from_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    to_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    comment_id BIGINT,
    content_preview VARCHAR(256),
    is_read BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_mentions ON mentions(to_user_id, is_read, created_at DESC);

CREATE TABLE IF NOT EXISTS post_favorites (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, post_id)
);

CREATE INDEX idx_post_fav_user ON post_favorites(user_id);

CREATE TABLE IF NOT EXISTS drafts (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    title VARCHAR(256),
    content TEXT,
    section_id INT,
    tag_ids BIGINT[],
    group_id INT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_drafts_user ON drafts(user_id);

CREATE TABLE IF NOT EXISTS user_profile_custom (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    theme VARCHAR(32) DEFAULT 'default',
    bg_color VARCHAR(16),
    bg_image VARCHAR(256),
    card_style INT DEFAULT 0,
    layout_type INT DEFAULT 0,
    show_fursona_first BOOLEAN DEFAULT TRUE,
    show_badges BOOLEAN DEFAULT TRUE,
    show_achievement BOOLEAN DEFAULT TRUE,
    music_url VARCHAR(256),
    custom_css TEXT,
    sidebar_widgets VARCHAR(64)[],
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS fursona_card_custom (
    fursona_id BIGINT PRIMARY KEY REFERENCES fursonas(id) ON DELETE CASCADE,
    card_theme VARCHAR(32) DEFAULT 'classic',
    border_color VARCHAR(16),
    bg_pattern INT DEFAULT 0,
    accent_color VARCHAR(16),
    font_style INT DEFAULT 0,
    show_stats BOOLEAN DEFAULT TRUE,
    show_artwork BOOLEAN DEFAULT TRUE,
    custom_fields JSONB,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS notification_settings (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    mention_email BOOLEAN DEFAULT TRUE,
    mention_push BOOLEAN DEFAULT TRUE,
    comment_email BOOLEAN DEFAULT TRUE,
    comment_push BOOLEAN DEFAULT TRUE,
    follow_email BOOLEAN DEFAULT TRUE,
    follow_push BOOLEAN DEFAULT TRUE,
    like_email BOOLEAN DEFAULT FALSE,
    like_push BOOLEAN DEFAULT TRUE,
    gift_email BOOLEAN DEFAULT TRUE,
    gift_push BOOLEAN DEFAULT TRUE,
    message_email BOOLEAN DEFAULT TRUE,
    message_push BOOLEAN DEFAULT TRUE,
    event_email BOOLEAN DEFAULT TRUE,
    event_push BOOLEAN DEFAULT TRUE,
    weekly_digest BOOLEAN DEFAULT TRUE,
    marketing_email BOOLEAN DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS feed_settings (
    user_id VARCHAR(128) PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
    default_sort VARCHAR(32) DEFAULT 'hot',
    show_avatars BOOLEAN DEFAULT TRUE,
    show_signatures BOOLEAN DEFAULT TRUE,
    compact_mode BOOLEAN DEFAULT FALSE,
    posts_per_page INT DEFAULT 20,
    auto_load_more BOOLEAN DEFAULT TRUE,
    blur_nsfw BOOLEAN DEFAULT TRUE,
    hide_nsfw BOOLEAN DEFAULT FALSE,
    blocked_tags VARCHAR(64)[],
    blocked_users VARCHAR(128)[]
);

CREATE TABLE IF NOT EXISTS group_custom_settings (
    group_id INT PRIMARY KEY REFERENCES user_groups(id) ON DELETE CASCADE,
    entry_message TEXT,
    require_approval BOOLEAN DEFAULT FALSE,
    approval_questions TEXT[],
    group_icon VARCHAR(256),
    group_color VARCHAR(16),
    custom_rules TEXT,
    allow_image_posts BOOLEAN DEFAULT TRUE,
    allow_link_posts BOOLEAN DEFAULT TRUE,
    post_cooldown INT DEFAULT 0,
    mod_can_delete BOOLEAN DEFAULT TRUE,
    mod_can_ban BOOLEAN DEFAULT FALSE,
    visible_members BOOLEAN DEFAULT TRUE
);

CREATE TABLE IF NOT EXISTS user_themes (
    id SERIAL PRIMARY KEY,
    name VARCHAR(64) NOT NULL,
    creator_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    primary_color VARCHAR(16),
    secondary_color VARCHAR(16),
    accent_color VARCHAR(16),
    bg_color VARCHAR(16),
    card_bg_color VARCHAR(16),
    text_color VARCHAR(16),
    is_public BOOLEAN DEFAULT FALSE,
    use_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS paid_content (
    id SERIAL PRIMARY KEY,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    author_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    price INT NOT NULL DEFAULT 100,
    preview_content TEXT,
    full_content TEXT,
    purchase_count INT DEFAULT 0,
    revenue INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS content_purchases (
    id SERIAL PRIMARY KEY,
    content_id BIGINT REFERENCES paid_content(id) ON DELETE CASCADE,
    buyer_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    price_paid INT NOT NULL,
    purchased_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(content_id, buyer_id)
);

CREATE TABLE IF NOT EXISTS galleries (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(64) NOT NULL,
    description TEXT,
    cover_image VARCHAR(256),
    is_public BOOLEAN DEFAULT TRUE,
    is_nsfw BOOLEAN DEFAULT FALSE,
    item_count INT DEFAULT 0,
    view_count INT DEFAULT 0,
    like_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS gallery_items (
    id SERIAL PRIMARY KEY,
    gallery_id INT REFERENCES galleries(id) ON DELETE CASCADE,
    title VARCHAR(128),
    description TEXT,
    image_url VARCHAR(256) NOT NULL,
    thumbnail_url VARCHAR(256),
    fursona_id BIGINT REFERENCES fursonas(id) ON DELETE SET NULL,
    artist_name VARCHAR(64),
    is_nsfw BOOLEAN DEFAULT FALSE,
    view_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS gallery_favorites (
    gallery_id INT REFERENCES galleries(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (gallery_id, user_id)
);

CREATE TABLE IF NOT EXISTS point_transfers (
    id SERIAL PRIMARY KEY,
    from_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    to_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    amount INT NOT NULL,
    message VARCHAR(256),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS red_envelopes (
    id SERIAL PRIMARY KEY,
    sender_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    total_amount INT NOT NULL,
    count INT NOT NULL,
    remaining_amount INT NOT NULL,
    remaining_count INT NOT NULL,
    message VARCHAR(256),
    is_random BOOLEAN DEFAULT TRUE,
    expires_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS red_envelope_claims (
    envelope_id INT REFERENCES red_envelopes(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    amount INT NOT NULL,
    claimed_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (envelope_id, user_id)
);

CREATE TABLE IF NOT EXISTS post_rewards (
    id SERIAL PRIMARY KEY,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    sender_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    receiver_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    amount INT NOT NULL,
    message VARCHAR(256),
    anonymous BOOLEAN DEFAULT FALSE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS collections (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(64) NOT NULL,
    description TEXT,
    cover_image VARCHAR(256),
    is_public BOOLEAN DEFAULT TRUE,
    item_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS collection_items (
    collection_id INT REFERENCES collections(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    added_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (collection_id, post_id)
);

CREATE INDEX idx_paid_content ON paid_content(author_id, created_at DESC);
CREATE INDEX idx_purchases ON content_purchases(buyer_id, purchased_at DESC);
CREATE INDEX idx_galleries ON galleries(user_id, is_public, created_at DESC);
CREATE INDEX idx_gallery_items ON gallery_items(gallery_id, created_at DESC);
CREATE INDEX idx_transfers ON point_transfers(from_user_id, created_at DESC);
CREATE INDEX idx_rewards ON post_rewards(post_id, created_at DESC);
CREATE INDEX idx_collections ON collections(user_id, created_at DESC);

CREATE TABLE IF NOT EXISTS fursona_relations (
    id SERIAL PRIMARY KEY,
    fursona_a_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    fursona_b_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    relation_type VARCHAR(32) NOT NULL,
    user_a_confirmed BOOLEAN DEFAULT FALSE,
    user_b_confirmed BOOLEAN DEFAULT FALSE,
    anniversary BIGINT,
    relation_data JSONB,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(fursona_a_id, fursona_b_id)
);

CREATE TABLE IF NOT EXISTS world_settings (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    description TEXT,
    cover_image VARCHAR(256),
    setting_type VARCHAR(32),
    tags VARCHAR(64)[],
    is_public BOOLEAN DEFAULT FALSE,
    view_count INT DEFAULT 0,
    like_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS world_pages (
    id SERIAL PRIMARY KEY,
    world_id INT REFERENCES world_settings(id) ON DELETE CASCADE,
    title VARCHAR(128) NOT NULL,
    content TEXT,
    page_type VARCHAR(32),
    parent_id INT,
    sort_order INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS world_likes (
    world_id INT REFERENCES world_settings(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (world_id, user_id)
);

CREATE INDEX idx_fursona_relations ON fursona_relations(fursona_a_id, fursona_b_id);
CREATE INDEX idx_world_settings ON world_settings(user_id, is_public, created_at DESC);
CREATE INDEX idx_world_pages ON world_pages(world_id, sort_order);

CREATE TABLE IF NOT EXISTS fursona_cards (
    id SERIAL PRIMARY KEY,
    fursona_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    template_id VARCHAR(32) DEFAULT 'default',
    theme_color VARCHAR(16) DEFAULT '#6B9EFF',
    background_image VARCHAR(256),
    show_stats BOOLEAN DEFAULT true,
    show_artist BOOLEAN DEFAULT true,
    card_layout VARCHAR(16) DEFAULT 'classic',
    font_family VARCHAR(64),
    custom_fields JSONB,
    view_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(fursona_id)
);

CREATE TABLE IF NOT EXISTS content_ratings (
    id SERIAL PRIMARY KEY,
    content_type VARCHAR(32) NOT NULL,
    content_id BIGINT NOT NULL,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    rating_level VARCHAR(16) NOT NULL,
    content_warnings VARCHAR(64)[],
    is_age_verified BOOLEAN DEFAULT false,
    rated_by VARCHAR(128),
    rated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(content_type, content_id)
);

CREATE TABLE IF NOT EXISTS creation_permissions (
    id SERIAL PRIMARY KEY,
    author_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    authorized_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    fursona_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    permission_type VARCHAR(32) NOT NULL,
    terms TEXT,
    is_approved BOOLEAN DEFAULT false,
    expires_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(author_user_id, authorized_user_id, fursona_id, permission_type)
);

CREATE TABLE IF NOT EXISTS fursona_interactions (
    id SERIAL PRIMARY KEY,
    from_fursona_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    to_fursona_id BIGINT REFERENCES fursonas(id) ON DELETE CASCADE,
    interaction_type VARCHAR(32) NOT NULL,
    user_note TEXT,
    intimacy_score INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(from_fursona_id, to_fursona_id, interaction_type)
);

CREATE TABLE IF NOT EXISTS moderation_queue (
    id SERIAL PRIMARY KEY,
    content_type VARCHAR(32) NOT NULL,
    content_id BIGINT NOT NULL,
    submitter_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    status VARCHAR(16) DEFAULT 'pending',
    moderator_id VARCHAR(128),
    moderator_note TEXT,
    violation_type VARCHAR(64),
    submitted_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    reviewed_at BIGINT
);

CREATE TABLE IF NOT EXISTS user_content_preferences (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE PRIMARY KEY,
    show_safe BOOLEAN DEFAULT true,
    show_questionable BOOLEAN DEFAULT false,
    show_explicit BOOLEAN DEFAULT false,
    enabled_warnings VARCHAR(64)[],
    blur_sensitive BOOLEAN DEFAULT true,
    age_verified BOOLEAN DEFAULT false,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_fursona_cards ON fursona_cards(fursona_id, user_id);
CREATE INDEX idx_content_ratings ON content_ratings(content_type, content_id);
CREATE INDEX idx_creation_perms ON creation_permissions(author_user_id, authorized_user_id);
CREATE INDEX idx_interactions ON fursona_interactions(from_fursona_id, to_fursona_id);
CREATE INDEX idx_moderation_queue ON moderation_queue(status, submitted_at DESC);

CREATE TABLE IF NOT EXISTS gallery_items (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    fursona_id BIGINT REFERENCES fursonas(id) ON DELETE SET NULL,
    title VARCHAR(128) NOT NULL,
    description TEXT,
    file_url VARCHAR(256) NOT NULL,
    thumbnail_url VARCHAR(256),
    file_type VARCHAR(32) NOT NULL,
    file_size BIGINT DEFAULT 0,
    image_width INT,
    image_height INT,
    artist_name VARCHAR(64),
    artist_url VARCHAR(256),
    tags VARCHAR(64)[],
    is_public BOOLEAN DEFAULT true,
    is_nsfw BOOLEAN DEFAULT false,
    view_count INT DEFAULT 0,
    like_count INT DEFAULT 0,
    comment_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS gallery_albums (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    title VARCHAR(128) NOT NULL,
    description TEXT,
    cover_image VARCHAR(256),
    is_public BOOLEAN DEFAULT true,
    item_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS album_items (
    album_id BIGINT REFERENCES gallery_albums(id) ON DELETE CASCADE,
    item_id BIGINT REFERENCES gallery_items(id) ON DELETE CASCADE,
    sort_order INT DEFAULT 0,
    added_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (album_id, item_id)
);

CREATE TABLE IF NOT EXISTS search_index (
    id SERIAL PRIMARY KEY,
    content_type VARCHAR(32) NOT NULL,
    content_id BIGINT NOT NULL,
    title VARCHAR(256) NOT NULL,
    content_text TEXT,
    author_id VARCHAR(128),
    tags VARCHAR(64)[],
    is_public BOOLEAN DEFAULT true,
    weight INT DEFAULT 1,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(content_type, content_id)
);

CREATE TABLE IF NOT EXISTS user_presence (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE PRIMARY KEY,
    status VARCHAR(16) DEFAULT 'offline',
    last_active BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    last_ip VARCHAR(64),
    user_agent TEXT,
    is_invisible BOOLEAN DEFAULT false
);

CREATE TABLE IF NOT EXISTS export_tasks (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    task_type VARCHAR(32) NOT NULL,
    status VARCHAR(16) DEFAULT 'pending',
    file_url VARCHAR(256),
    file_size BIGINT DEFAULT 0,
    expires_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    completed_at BIGINT
);

CREATE TABLE IF NOT EXISTS system_configs (
    config_key VARCHAR(64) PRIMARY KEY,
    config_value TEXT,
    config_type VARCHAR(32) DEFAULT 'string',
    description TEXT,
    is_public BOOLEAN DEFAULT false,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS audit_logs (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    action VARCHAR(64) NOT NULL,
    resource_type VARCHAR(32),
    resource_id BIGINT,
    old_value TEXT,
    new_value TEXT,
    ip_address VARCHAR(64),
    user_agent TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS gallery_likes (
    item_id BIGINT REFERENCES gallery_items(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (item_id, user_id)
);

CREATE INDEX idx_gallery_user ON gallery_items(user_id, created_at DESC);
CREATE INDEX idx_gallery_fursona ON gallery_items(fursona_id);
CREATE INDEX idx_gallery_tags ON gallery_items USING GIN(tags);
CREATE INDEX idx_search_fulltext ON search_index USING GIN(to_tsvector('english', title || ' ' || content_text));
CREATE INDEX idx_search_tags ON search_index USING GIN(tags);
CREATE INDEX idx_albums_user ON gallery_albums(user_id);
CREATE INDEX idx_presence_status ON user_presence(status, last_active DESC);
CREATE INDEX idx_audit_user ON audit_logs(user_id, created_at DESC);
CREATE INDEX idx_audit_action ON audit_logs(action, created_at DESC);

CREATE TABLE IF NOT EXISTS ip_blacklist (
    id SERIAL PRIMARY KEY,
    ip_address VARCHAR(64) UNIQUE NOT NULL,
    reason VARCHAR(256),
    banned_by VARCHAR(128),
    expires_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS rate_limits (
    id SERIAL PRIMARY KEY,
    identifier VARCHAR(128) NOT NULL,
    limit_type VARCHAR(32) NOT NULL,
    request_count INT DEFAULT 0,
    window_start BIGINT NOT NULL,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(identifier, limit_type)
);

CREATE TABLE IF NOT EXISTS api_signatures (
    api_key VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    secret_key VARCHAR(128) NOT NULL,
    algorithm VARCHAR(32) DEFAULT 'HMAC-SHA256',
    is_active BOOLEAN DEFAULT true,
    expires_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS user_sessions (
    session_id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    device_info JSONB,
    ip_address VARCHAR(64),
    location VARCHAR(128),
    user_agent TEXT,
    last_active BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS login_history (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    ip_address VARCHAR(64),
    location VARCHAR(128),
    device_info JSONB,
    user_agent TEXT,
    was_successful BOOLEAN DEFAULT true,
    failure_reason VARCHAR(64),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS user_blocks (
    blocker_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    blocked_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    block_type VARCHAR(32) DEFAULT 'all',
    reason TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (blocker_id, blocked_id)
);

CREATE TABLE IF NOT EXISTS system_announcements (
    id SERIAL PRIMARY KEY,
    title VARCHAR(128) NOT NULL,
    content TEXT NOT NULL,
    announcement_type VARCHAR(32) DEFAULT 'normal',
    priority INT DEFAULT 0,
    is_active BOOLEAN DEFAULT true,
    show_until BIGINT,
    created_by VARCHAR(128),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS webhook_endpoints (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    endpoint_url VARCHAR(256) NOT NULL,
    secret VARCHAR(128),
    events VARCHAR(64)[],
    is_active BOOLEAN DEFAULT true,
    last_called_at BIGINT,
    failure_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS webhook_logs (
    id SERIAL PRIMARY KEY,
    endpoint_id BIGINT REFERENCES webhook_endpoints(id) ON DELETE CASCADE,
    event_type VARCHAR(64) NOT NULL,
    request_body TEXT,
    response_status INT,
    response_body TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS security_alerts (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    alert_type VARCHAR(64) NOT NULL,
    severity VARCHAR(16) DEFAULT 'medium',
    details JSONB,
    ip_address VARCHAR(64),
    is_resolved BOOLEAN DEFAULT false,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_ip_blacklist ON ip_blacklist(ip_address, expires_at);
CREATE INDEX idx_rate_limits ON rate_limits(identifier, limit_type);
CREATE INDEX idx_sessions_user ON user_sessions(user_id, last_active DESC);
CREATE INDEX idx_login_user ON login_history(user_id, created_at DESC);
CREATE INDEX idx_blocks_blocker ON user_blocks(blocker_id);
CREATE INDEX idx_announcements_active ON system_announcements(is_active, priority DESC);
CREATE INDEX idx_webhooks_user ON webhook_endpoints(user_id);
CREATE INDEX idx_alerts_user ON security_alerts(user_id, is_resolved);

CREATE TABLE IF NOT EXISTS content_reports (
    id SERIAL PRIMARY KEY,
    reporter_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    content_type VARCHAR(32) NOT NULL,
    content_id BIGINT NOT NULL,
    report_reason VARCHAR(64) NOT NULL,
    report_details TEXT,
    status VARCHAR(32) DEFAULT 'pending',
    handled_by VARCHAR(128),
    handled_at BIGINT,
    handler_notes TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(reporter_id, content_type, content_id)
);

CREATE TABLE IF NOT EXISTS comment_replies (
    id SERIAL PRIMARY KEY,
    comment_id BIGINT REFERENCES comments(id) ON DELETE CASCADE,
    parent_reply_id BIGINT REFERENCES comment_replies(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    reply_to_user_id VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    content TEXT NOT NULL,
    like_count INT DEFAULT 0,
    is_deleted BOOLEAN DEFAULT false,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS post_sticky (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE PRIMARY KEY,
    section_id BIGINT,
    priority INT DEFAULT 0,
    sticky_by VARCHAR(128),
    sticky_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS post_digest (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE PRIMARY KEY,
    digest_level VARCHAR(32) DEFAULT 'bronze',
    recommended_by VARCHAR(128),
    description TEXT,
    recommended_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS collection_folders (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(64) NOT NULL,
    description TEXT,
    is_public BOOLEAN DEFAULT false,
    item_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS collection_items (
    folder_id BIGINT REFERENCES collection_folders(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    saved_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (folder_id, post_id)
);

CREATE TABLE IF NOT EXISTS user_tags (
    tagger_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    tagged_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    tag VARCHAR(32) NOT NULL,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (tagger_id, tagged_id, tag)
);

CREATE TABLE IF NOT EXISTS keyword_filters (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    keyword VARCHAR(64) NOT NULL,
    filter_type VARCHAR(32) DEFAULT 'post_title',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(user_id, keyword, filter_type)
);

CREATE TABLE IF NOT EXISTS post_drafts (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    title VARCHAR(128),
    content TEXT,
    section_id BIGINT,
    tags VARCHAR(64)[],
    fursona_id BIGINT,
    is_auto_save BOOLEAN DEFAULT false,
    updated_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS post_share_stats (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE PRIMARY KEY,
    view_count INT DEFAULT 0,
    share_count INT DEFAULT 0,
    download_count INT DEFAULT 0,
    last_viewed_at BIGINT
);

CREATE INDEX idx_reports_status ON content_reports(status, created_at DESC);
CREATE INDEX idx_reports_content ON content_reports(content_type, content_id);
CREATE INDEX idx_comment_replies ON comment_replies(comment_id, created_at);
CREATE INDEX idx_sticky_section ON post_sticky(section_id, priority DESC);
CREATE INDEX idx_digest_level ON post_digest(digest_level);
CREATE INDEX idx_collection_user ON collection_folders(user_id);
CREATE INDEX idx_tags_tagger ON user_tags(tagger_id);
CREATE INDEX idx_filter_user ON keyword_filters(user_id);
CREATE INDEX idx_drafts_user ON post_drafts(user_id, updated_at DESC);

CREATE TABLE IF NOT EXISTS section_moderators (
    id SERIAL PRIMARY KEY,
    section_id BIGINT NOT NULL,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    assigned_by VARCHAR(128),
    permission_level VARCHAR(32) DEFAULT 'full',
    can_manage_posts BOOLEAN DEFAULT true,
    can_manage_comments BOOLEAN DEFAULT true,
    can_manage_users BOOLEAN DEFAULT true,
    can_manage_reports BOOLEAN DEFAULT true,
    assigned_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(section_id, user_id)
);

CREATE TABLE IF NOT EXISTS user_punishments (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    punishment_type VARCHAR(32) NOT NULL,
    reason TEXT,
    duration BIGINT,
    points_deducted INT DEFAULT 0,
    executed_by VARCHAR(128),
    executed_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    expires_at BIGINT,
    is_active BOOLEAN DEFAULT true
);

CREATE TABLE IF NOT EXISTS punishment_records (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    punishment_type VARCHAR(32) NOT NULL,
    reason TEXT,
    points_deducted INT DEFAULT 0,
    executed_by VARCHAR(128),
    executed_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS poll_votes (
    id SERIAL PRIMARY KEY,
    poll_id BIGINT NOT NULL,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    option_index INT NOT NULL,
    voted_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(poll_id, user_id)
);

CREATE TABLE IF NOT EXISTS post_polls (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE PRIMARY KEY,
    question VARCHAR(256) NOT NULL,
    options TEXT[],
    vote_counts INT[],
    is_multiple BOOLEAN DEFAULT false,
    end_at BIGINT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS hot_scores (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE PRIMARY KEY,
    hot_score DOUBLE PRECISION DEFAULT 0,
    view_weight DOUBLE PRECISION DEFAULT 0,
    like_weight DOUBLE PRECISION DEFAULT 0,
    comment_weight DOUBLE PRECISION DEFAULT 0,
    last_updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS related_posts (
    source_post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    target_post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    similarity DOUBLE PRECISION DEFAULT 0,
    PRIMARY KEY (source_post_id, target_post_id)
);

CREATE TABLE IF NOT EXISTS media_watermarks (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    watermark_text VARCHAR(128),
    watermark_position VARCHAR(32) DEFAULT 'bottom_right',
    opacity DOUBLE PRECISION DEFAULT 0.3,
    is_enabled BOOLEAN DEFAULT true,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS image_process_queue (
    id SERIAL PRIMARY KEY,
    file_path VARCHAR(256) NOT NULL,
    user_id VARCHAR(128),
    task_type VARCHAR(32) DEFAULT 'compress',
    status VARCHAR(32) DEFAULT 'pending',
    result_path VARCHAR(256),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE TABLE IF NOT EXISTS user_feed_settings (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE PRIMARY KEY,
    feed_type VARCHAR(32) DEFAULT 'hot',
    include_sections BIGINT[],
    exclude_tags VARCHAR(64)[],
    updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS recommendation_logs (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT,
    algorithm VARCHAR(64),
    action VARCHAR(32),
    score DOUBLE PRECISION,
    created_at BIGINT
);

CREATE INDEX idx_mod_section ON section_moderators(section_id);
CREATE INDEX idx_mod_user ON section_moderators(user_id);
CREATE INDEX idx_punishment_user ON user_punishments(user_id, is_active);
CREATE INDEX idx_poll_post ON post_polls(post_id);
CREATE INDEX idx_hot_score ON hot_scores(hot_score DESC);
CREATE INDEX idx_hot_updated ON hot_scores(last_updated_at DESC);

CREATE TABLE IF NOT EXISTS comment_likes (
    id SERIAL PRIMARY KEY,
    comment_id BIGINT NOT NULL REFERENCES comments(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(comment_id, user_id)
);

CREATE INDEX idx_comment_like_comment ON comment_likes(comment_id);
CREATE INDEX idx_comment_like_user ON comment_likes(user_id);

CREATE TABLE IF NOT EXISTS post_appreciations (
    id SERIAL PRIMARY KEY,
    post_id BIGINT NOT NULL REFERENCES posts(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    score INT NOT NULL DEFAULT 5,
    comment TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(post_id, user_id)
);

CREATE INDEX idx_appreciation_post ON post_appreciations(post_id);
CREATE INDEX idx_appreciation_user ON post_appreciations(user_id);

CREATE TABLE IF NOT EXISTS share_records (
    id SERIAL PRIMARY KEY,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    platform VARCHAR(64) NOT NULL,
    shared_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_share_post ON share_records(post_id);
CREATE INDEX idx_share_user ON share_records(user_id);
CREATE INDEX idx_share_platform ON share_records(platform);

CREATE TABLE IF NOT EXISTS visit_records (
    id SERIAL PRIMARY KEY,
    visitor_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    target_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    ip_address VARCHAR(64),
    user_agent VARCHAR(512),
    duration INT DEFAULT 0,
    visited_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_visit_visitor ON visit_records(visitor_id);
CREATE INDEX idx_visit_target ON visit_records(target_user_id);
CREATE INDEX idx_visit_post ON visit_records(post_id);
CREATE INDEX idx_visit_time ON visit_records(visited_at);

CREATE TABLE IF NOT EXISTS user_tags (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    tagged_user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    tag_name VARCHAR(64) NOT NULL,
    tag_color VARCHAR(7) DEFAULT '#808080',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(user_id, tagged_user_id, tag_name)
);

CREATE INDEX idx_tag_owner ON user_tags(user_id);
CREATE INDEX idx_tagged_user ON user_tags(tagged_user_id);

CREATE TABLE IF NOT EXISTS similar_posts (
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    similar_post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    similarity_score DOUBLE PRECISION DEFAULT 0,
    algorithm VARCHAR(64) DEFAULT 'tfidf',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (post_id, similar_post_id)
);

CREATE INDEX idx_similar_post ON similar_posts(post_id);
CREATE INDEX idx_similar_score ON similar_posts(similarity_score);

CREATE TABLE IF NOT EXISTS weekly_summaries (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    week_number INT NOT NULL,
    year INT NOT NULL,
    posts_count INT DEFAULT 0,
    comments_count INT DEFAULT 0,
    likes_received INT DEFAULT 0,
    followers_gained INT DEFAULT 0,
    views_gained INT DEFAULT 0,
    most_popular_post_id BIGINT,
    summary TEXT,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (user_id, week_number, year)
);

CREATE INDEX idx_summary_user ON weekly_summaries(user_id);
CREATE INDEX idx_summary_week ON weekly_summaries(week_number, year);

CREATE TABLE IF NOT EXISTS contributor_ranking (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE PRIMARY KEY,
    contribution_score DOUBLE PRECISION DEFAULT 0,
    posts_weight INT DEFAULT 0,
    comments_weight INT DEFAULT 0,
    likes_weight INT DEFAULT 0,
    uploads_weight INT DEFAULT 0,
    help_weight INT DEFAULT 0,
    last_updated BIGINT
);

CREATE INDEX idx_ranking_score ON contributor_ranking(contribution_score);

CREATE TABLE IF NOT EXISTS user_preferences (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE PRIMARY KEY,
    content_language VARCHAR(8) DEFAULT 'zh',
    default_sort VARCHAR(32) DEFAULT 'hot',
    show_nsfw BOOLEAN DEFAULT false,
    blur_nsfw BOOLEAN DEFAULT true,
    auto_play_video BOOLEAN DEFAULT true,
    infinite_scroll BOOLEAN DEFAULT true,
    compact_mode BOOLEAN DEFAULT false,
    night_mode VARCHAR(32) DEFAULT 'auto',
    font_size VARCHAR(32) DEFAULT 'medium',
    notify_on_like BOOLEAN DEFAULT true,
    notify_on_comment BOOLEAN DEFAULT true,
    notify_on_follow BOOLEAN DEFAULT true,
    notify_on_mention BOOLEAN DEFAULT true,
    notify_on_message BOOLEAN DEFAULT true,
    email_digest BOOLEAN DEFAULT true,
    show_online_status BOOLEAN DEFAULT true,
    show_read_status BOOLEAN DEFAULT true,
    updated_at BIGINT
);

CREATE TABLE IF NOT EXISTS reading_progress (
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    progress_percent INT DEFAULT 0,
    last_read_position INT DEFAULT 0,
    total_words INT DEFAULT 0,
    last_read_at BIGINT,
    is_completed BOOLEAN DEFAULT false,
    PRIMARY KEY (user_id, post_id)
);

CREATE INDEX idx_reading_user ON reading_progress(user_id);
CREATE INDEX idx_reading_post ON reading_progress(post_id);
CREATE INDEX idx_reading_complete ON reading_progress(is_completed);

CREATE TABLE IF NOT EXISTS user_notes (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    target_type VARCHAR(32) NOT NULL,
    target_id BIGINT NOT NULL,
    note_content TEXT NOT NULL,
    color VARCHAR(7) DEFAULT '#FFFF00',
    is_pinned BOOLEAN DEFAULT false,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT
);

CREATE INDEX idx_note_user ON user_notes(user_id);
CREATE INDEX idx_note_target ON user_notes(target_type, target_id);
CREATE INDEX idx_note_pinned ON user_notes(is_pinned);

CREATE TABLE IF NOT EXISTS content_history (
    id SERIAL PRIMARY KEY,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    comment_id BIGINT REFERENCES comments(id) ON DELETE CASCADE,
    edited_by VARCHAR(128) REFERENCES users(id) ON DELETE SET NULL,
    old_title VARCHAR(512),
    old_content TEXT,
    new_title VARCHAR(512),
    new_content TEXT,
    edit_reason VARCHAR(512),
    edited_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_history_post ON content_history(post_id);
CREATE INDEX idx_history_comment ON content_history(comment_id);
CREATE INDEX idx_history_editor ON content_history(edited_by);

CREATE TABLE IF NOT EXISTS quick_access (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    item_type VARCHAR(32) NOT NULL,
    item_id BIGINT NOT NULL,
    item_name VARCHAR(256) NOT NULL,
    item_icon VARCHAR(64),
    sort_order INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    UNIQUE(user_id, item_type, item_id)
);

CREATE INDEX idx_quickaccess_user ON quick_access(user_id);

CREATE TABLE IF NOT EXISTS word_filter (
    id SERIAL PRIMARY KEY,
    word VARCHAR(128) NOT NULL UNIQUE,
    replacement VARCHAR(128) DEFAULT '***',
    filter_level INT DEFAULT 1,
    is_regex BOOLEAN DEFAULT false,
    created_by VARCHAR(128),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_filter_word ON word_filter(word);
CREATE INDEX idx_filter_level ON word_filter(filter_level);

INSERT INTO word_filter (word, replacement, filter_level) VALUES
('�������', '***', 1),
('Υ������', '***', 1),
('��������', '***', 2),
('ɫ��', '***', 2),
('�Ĳ�', '***', 2);

CREATE TABLE IF NOT EXISTS post_series (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) REFERENCES users(id) ON DELETE CASCADE,
    title VARCHAR(256) NOT NULL,
    description TEXT,
    cover_image VARCHAR(512),
    is_public BOOLEAN DEFAULT true,
    post_count INT DEFAULT 0,
    view_count INT DEFAULT 0,
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    updated_at BIGINT
);

CREATE INDEX idx_series_user ON post_series(user_id);
CREATE INDEX idx_series_public ON post_series(is_public);

CREATE TABLE IF NOT EXISTS series_posts (
    series_id BIGINT REFERENCES post_series(id) ON DELETE CASCADE,
    post_id BIGINT REFERENCES posts(id) ON DELETE CASCADE,
    sort_order INT DEFAULT 0,
    added_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    PRIMARY KEY (series_id, post_id)
);

CREATE INDEX idx_series_post ON series_posts(series_id);
CREATE INDEX idx_series_sort ON series_posts(sort_order);


-- ==================== ʵ����֤��ر� ====================

CREATE TABLE IF NOT EXISTS real_name_verifications (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    id_card_number VARCHAR(64) NOT NULL,
    is_verified BOOLEAN DEFAULT FALSE,
    is_matched BOOLEAN DEFAULT FALSE,
    confidence_level INTEGER DEFAULT 0,
    task_id VARCHAR(128),
    transaction_id VARCHAR(128),
    status VARCHAR(64) DEFAULT 'pending',
    reason VARCHAR(512),
    face_verified BOOLEAN DEFAULT FALSE,
    face_similarity FLOAT DEFAULT 0.0,
    verify_provider VARCHAR(64) DEFAULT 'netease',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    verified_at BIGINT,
    retry_count INTEGER DEFAULT 0
);

CREATE INDEX idx_verification_user ON real_name_verifications(user_id);
CREATE INDEX idx_verification_task ON real_name_verifications(task_id);
CREATE INDEX idx_verification_status ON real_name_verifications(status);

CREATE TABLE IF NOT EXISTS face_verification_records (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    task_id VARCHAR(128),
    image_url VARCHAR(512),
    similarity FLOAT DEFAULT 0.0,
    is_passed BOOLEAN DEFAULT FALSE,
    best_face_url VARCHAR(512),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX idx_face_verify_user ON face_verification_records(user_id);
CREATE INDEX idx_face_verify_task ON face_verification_records(task_id);

