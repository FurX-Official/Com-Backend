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
