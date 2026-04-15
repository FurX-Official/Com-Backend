#ifndef FURBBS_DB_SQL_QUERIES_H
#define FURBBS_DB_SQL_QUERIES_H

#include <string>

namespace furbbs::db::sql {

const std::string USER_GET_BY_ID = R"(
    SELECT id, username, email, avatar, bio, role, status, created_at, signature
    FROM users WHERE id = $1
)";

const std::string USER_GET_BY_USERNAME = R"(
    SELECT id, username, email, avatar, bio, role, status, created_at
    FROM users WHERE username = $1
)";

const std::string USER_UPDATE_PROFILE = R"(
    UPDATE users SET username = $1, email = $2, avatar = $3, bio = $4, signature = $5
    WHERE id = $6
)";

const std::string USER_GET_STATS = R"(
    SELECT user_id, posts_count, comments_count, likes_count, follower_count, 
           following_count, points, reputation, level
    FROM user_stats WHERE user_id = $1
)";

const std::string USER_UPDATE_STATS_POINTS = R"(
    UPDATE user_stats SET points = points + $1 WHERE user_id = $2
)";

const std::string POST_GET_BY_ID = R"(
    SELECT p.id, p.user_id, p.section_id, p.title, p.content, p.status, 
           p.is_pinned, p.is_essence, p.view_count, p.like_count, p.comment_count,
           p.created_at, p.updated_at, u.username, u.avatar
    FROM posts p
    LEFT JOIN users u ON p.user_id = u.id
    WHERE p.id = $1 AND p.is_deleted = FALSE
)";

const std::string POST_CREATE = R"(
    INSERT INTO posts (user_id, section_id, title, content, content_rating, tags)
    VALUES ($1, $2, $3, $4, $5, $6)
    RETURNING id
)";

const std::string POST_UPDATE = R"(
    UPDATE posts SET title = $1, content = $2, section_id = $3, tags = $4, updated_at = $5
    WHERE id = $6 AND user_id = $7
)";

const std::string POST_DELETE = R"(
    UPDATE posts SET is_deleted = TRUE WHERE id = $1 AND (user_id = $2 OR EXISTS (
        SELECT 1 FROM users WHERE id = $2 AND role IN ('admin', 'moderator')
    ))
)";

const std::string POST_LIST_BY_SECTION = R"(
    SELECT p.id, p.user_id, p.title, p.is_pinned, p.is_essence, p.view_count, 
           p.like_count, p.comment_count, p.created_at, u.username, u.avatar,
           array_agg(t.name) as tags
    FROM posts p
    LEFT JOIN users u ON p.user_id = u.id
    LEFT JOIN post_tags pt ON p.id = pt.post_id
    LEFT JOIN tags t ON pt.tag_id = t.id
    WHERE p.section_id = $1 AND p.is_deleted = FALSE
    GROUP BY p.id, u.username, u.avatar
    ORDER BY p.is_pinned DESC, p.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string COMMENT_CREATE = R"(
    INSERT INTO comments (post_id, user_id, parent_id, content)
    VALUES ($1, $2, $3, $4)
    RETURNING id
)";

const std::string COMMENT_GET_BY_POST = R"(
    SELECT c.id, c.post_id, c.user_id, c.parent_id, c.content, c.like_count, 
           c.created_at, u.username, u.avatar
    FROM comments c
    LEFT JOIN users u ON c.user_id = u.id
    WHERE c.post_id = $1 AND c.is_deleted = FALSE
    ORDER BY c.created_at ASC
)";

const std::string COMMENT_DELETE = R"(
    UPDATE comments SET is_deleted = TRUE WHERE id = $1 AND (user_id = $2 OR EXISTS (
        SELECT 1 FROM users WHERE id = $2 AND role IN ('admin', 'moderator')
    ))
)";

const std::string LIKE_TOGGLE_POST = R"(
    INSERT INTO post_likes (user_id, post_id) VALUES ($1, $2)
    ON CONFLICT DO NOTHING
)";

const std::string LIKE_UNLIKE_POST = R"(
    DELETE FROM post_likes WHERE user_id = $1 AND post_id = $2
)";

const std::string LIKE_COUNT_POST = R"(
    UPDATE posts SET like_count = (SELECT COUNT(*) FROM post_likes WHERE post_id = $1)
    WHERE id = $1
)";

const std::string FAVORITE_TOGGLE = R"(
    INSERT INTO favorites (user_id, target_type, target_id) VALUES ($1, $2, $3)
    ON CONFLICT DO NOTHING
)";

const std::string FAVORITE_REMOVE = R"(
    DELETE FROM favorites WHERE user_id = $1 AND target_type = $2 AND target_id = $3
)";

const std::string FAVORITE_LIST_USER = R"(
    SELECT target_type, target_id, created_at
    FROM favorites WHERE user_id = $1
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string FOLLOW_TOGGLE = R"(
    INSERT INTO user_follows (follower_id, following_id) VALUES ($1, $2)
    ON CONFLICT DO NOTHING
)";

const std::string FOLLOW_UNFOLLOW = R"(
    DELETE FROM user_follows WHERE follower_id = $1 AND following_id = $2
)";

const std::string FOLLOW_LIST_FOLLOWERS = R"(
    SELECT f.follower_id, u.username, u.avatar, f.created_at
    FROM user_follows f
    LEFT JOIN users u ON f.follower_id = u.id
    WHERE f.following_id = $1
    ORDER BY f.created_at DESC
)";

const std::string FOLLOW_LIST_FOLLOWING = R"(
    SELECT f.following_id, u.username, u.avatar, f.created_at
    FROM user_follows f
    LEFT JOIN users u ON f.following_id = u.id
    WHERE f.follower_id = $1
    ORDER BY f.created_at DESC
)";

const std::string NOTIFICATION_GET_UNREAD_COUNT = R"(
    SELECT COUNT(*) FROM notifications WHERE user_id = $1 AND is_read = FALSE
)";

const std::string NOTIFICATION_GET_BY_USER = R"(
    SELECT id, type, actor_id, related_id, related_type, title, content, is_read, created_at
    FROM notifications WHERE user_id = $1
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string NOTIFICATION_MARK_READ = R"(
    UPDATE notifications SET is_read = TRUE WHERE user_id = $1 AND id = $2
)";

const std::string NOTIFICATION_MARK_ALL_READ = R"(
    UPDATE notifications SET is_read = TRUE WHERE user_id = $1
)";

const std::string NOTIFICATION_INSERT = R"(
    INSERT INTO notifications (user_id, type, actor_id, related_id, related_type, title, content)
    VALUES ($1, $2, $3, $4, $5, $6, $7)
)";

const std::string SECTION_LIST_ALL = R"(
    SELECT id, name, description, icon, sort_order, posts_count, parent_id
    FROM sections WHERE is_active = TRUE
    ORDER BY sort_order ASC, id ASC
)";

const std::string TAG_LIST_POPULAR = R"(
    SELECT t.id, t.name, t.slug, COUNT(pt.post_id) as usage_count
    FROM tags t
    LEFT JOIN post_tags pt ON t.id = pt.tag_id
    GROUP BY t.id, t.name, t.slug
    ORDER BY usage_count DESC
    LIMIT $1
)";

const std::string BADGE_LIST_ALL = R"(
    SELECT id, name, description, icon, rarity, requirement
    FROM badges WHERE is_active = TRUE
    ORDER BY rarity ASC, id ASC
)";

const std::string BADGE_GRANT = R"(
    INSERT INTO user_badges (user_id, badge_id, granted_at)
    VALUES ($1, $2, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING
)";

const std::string USER_BADGE_LIST = R"(
    SELECT b.id, b.name, b.description, b.icon, b.rarity, ub.granted_at
    FROM user_badges ub
    LEFT JOIN badges b ON ub.badge_id = b.id
    WHERE ub.user_id = $1
    ORDER BY ub.granted_at DESC
)";

const std::string FURSONA_GET_BY_USER = R"(
    SELECT id, name, species, gender, bio, reference_images, is_public, created_at
    FROM fursonas WHERE user_id = $1
    ORDER BY created_at DESC
)";

const std::string FURSONA_CREATE = R"(
    INSERT INTO fursonas (user_id, name, species, gender, bio, reference_images, is_public)
    VALUES ($1, $2, $3, $4, $5, $6, $7)
    RETURNING id
)";

const std::string COMMISSION_LIST = R"(
    SELECT c.id, c.user_id, c.title, c.description, c.price_min, c.price_max,
           c.status, c.commission_type, u.username, u.avatar
    FROM commissions c
    LEFT JOIN users u ON c.user_id = u.id
    WHERE c.status = 'open'
    ORDER BY c.created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string EVENT_LIST_UPCOMING = R"(
    SELECT id, title, description, location, start_time, end_time, max_attendees,
           attendee_count, is_online, url
    FROM events WHERE start_time > EXTRACT(EPOCH FROM NOW()) * 1000
    ORDER BY start_time ASC
    LIMIT $1
)";

const std::string API_KEY_CREATE = R"(
    INSERT INTO api_keys (user_id, name, key_hash, scopes, rate_limit)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string API_KEY_VERIFY = R"(
    SELECT user_id, scopes, rate_limit FROM api_keys WHERE key_hash = $1 AND is_active = TRUE
)";

const std::string MODERATION_LOG_INSERT = R"(
    INSERT INTO moderation_logs (moderator_id, action, target_type, target_id, reason)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string USER_MUTE_CREATE = R"(
    INSERT INTO user_mutes (user_id, muted_user_id, expires_at)
    VALUES ($1, $2, $3)
)";

const std::string USER_BAN_CREATE = R"(
    INSERT INTO user_bans (user_id, reason, banned_by, expires_at)
    VALUES ($1, $2, $3, $4)
)";

const std::string USER_BAN_REMOVE = R"(
    UPDATE users SET status = 'active' WHERE id = $1;
    DELETE FROM user_bans WHERE user_id = $1
)";

const std::string PM_SEND = R"(
    INSERT INTO private_messages (from_user_id, to_user_id, content)
    VALUES ($1, $2, $3)
)";

const std::string PM_GET_CONVERSATIONS = R"(
    SELECT DISTINCT ON (other_user) * FROM (
        SELECT 
            CASE WHEN from_user_id = $1 THEN to_user_id ELSE from_user_id END as other_user,
            MAX(created_at) as last_message_at,
            COUNT(*) FILTER (WHERE is_read = FALSE AND to_user_id = $1) as unread_count
        FROM private_messages
        WHERE from_user_id = $1 OR to_user_id = $1
        GROUP BY other_user
    ) t
    ORDER BY last_message_at DESC
)";

const std::string PM_MARK_READ = R"(
    UPDATE private_messages SET is_read = TRUE 
    WHERE from_user_id = $1 AND to_user_id = $2
)";

const std::string INVITE_CODE_GENERATE = R"(
    INSERT INTO invite_codes (code, creator_id) VALUES ($1, $2)
)";

const std::string DAILY_CHECKIN = R"(
    INSERT INTO checkin_records (user_id, consecutive_days)
    VALUES ($1, COALESCE((
        SELECT consecutive_days + 1 FROM checkin_records 
        WHERE user_id = $1 
        ORDER BY created_at DESC LIMIT 1
    ), 1))
)";

const std::string LUCKY_DRAW = R"(
    INSERT INTO lucky_draw_records (user_id, prize_id, prize_name, points)
    VALUES ($1, $2, $3, $4)
)";

const std::string FAQ_LIST = R"(
    SELECT id, question, answer, category, sort_order, view_count, is_active, created_at
    FROM faqs WHERE is_active = TRUE
    ORDER BY sort_order ASC, created_at DESC
)";

const std::string FAQ_LIST_BY_CATEGORY = R"(
    SELECT id, question, answer, category, sort_order, view_count, is_active, created_at
    FROM faqs WHERE category = $1 AND is_active = TRUE
    ORDER BY sort_order ASC, created_at DESC
)";

const std::string FAQ_CATEGORIES = R"(
    SELECT DISTINCT category FROM faqs WHERE is_active = TRUE ORDER BY category
)";

const std::string FAQ_CREATE = R"(
    INSERT INTO faqs (question, answer, category, sort_order, is_active)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string FAQ_UPDATE = R"(
    UPDATE faqs SET question = $1, answer = $2, category = $3, sort_order = $4, is_active = $5
    WHERE id = $6
)";

const std::string FAQ_DELETE = R"(
    DELETE FROM faqs WHERE id = $1
)";

const std::string HELP_ARTICLE_LIST = R"(
    SELECT id, title, content, summary, category, cover_image, 
           view_count, like_count, is_published, created_at, updated_at
    FROM help_articles WHERE is_published = TRUE
    ORDER BY created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string FEEDBACK_CREATE = R"(
    INSERT INTO feedbacks (user_id, type, title, content, images, contact)
    VALUES ($1, $2, $3, $4, $5, $6)
)";

const std::string FEEDBACK_LIST = R"(
    SELECT f.id, f.user_id, u.username, f.type, f.title, f.content, 
           f.images, f.contact, f.status, f.admin_reply, f.created_at, f.replied_at
    FROM feedbacks f
    LEFT JOIN users u ON f.user_id = u.id
    ORDER BY f.created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string FEEDBACK_REPLY = R"(
    UPDATE feedbacks SET admin_reply = $1, status = $2, replied_at = $3
    WHERE id = $4
)";

const std::string AD_LIST_ACTIVE = R"(
    SELECT id, name, position, image_url, link_url, sort_order, 
           start_time, end_time, is_active, click_count
    FROM advertisements
    WHERE is_active = TRUE
      AND (start_time IS NULL OR start_time <= $1)
      AND (end_time IS NULL OR end_time >= $1)
    ORDER BY sort_order ASC
)";

const std::string AD_LIST_BY_POSITION = R"(
    SELECT id, name, position, image_url, link_url, sort_order, 
           start_time, end_time, is_active, click_count
    FROM advertisements
    WHERE position = $1 AND is_active = TRUE
      AND (start_time IS NULL OR start_time <= $2)
      AND (end_time IS NULL OR end_time >= $2)
    ORDER BY sort_order ASC
)";

const std::string FRIEND_LINK_LIST = R"(
    SELECT id, name, url, logo, description, sort_order, is_active, created_at
    FROM friend_links WHERE is_active = TRUE
    ORDER BY sort_order ASC, created_at DESC
)";

const std::string STAT_TOTAL_USERS = R"(SELECT COUNT(*) FROM users)";
const std::string STAT_TODAY_USERS = R"(
    SELECT COUNT(*) FROM users 
    WHERE created_at > EXTRACT(EPOCH FROM CURRENT_DATE) * 1000
)";
const std::string STAT_TOTAL_POSTS = R"(SELECT COUNT(*) FROM posts)";
const std::string STAT_TODAY_POSTS = R"(
    SELECT COUNT(*) FROM posts 
    WHERE created_at > EXTRACT(EPOCH FROM CURRENT_DATE) * 1000
)";
const std::string STAT_TOTAL_COMMENTS = R"(SELECT COUNT(*) FROM comments)";
const std::string STAT_TODAY_COMMENTS = R"(
    SELECT COUNT(*) FROM comments 
    WHERE created_at > EXTRACT(EPOCH FROM CURRENT_DATE) * 1000
)";
const std::string STAT_ONLINE_USERS = R"(
    SELECT COUNT(DISTINCT user_id) FROM user_activity_logs
    WHERE created_at > EXTRACT(EPOCH FROM NOW() - INTERVAL '5 minutes') * 1000
)";

const std::string CAPTCHA_SETTINGS_GET = R"(
    SELECT secret_key, verify_url FROM captcha_settings 
    WHERE provider = $1 AND is_active = TRUE
)";

const std::string REPORT_CREATE = R"(
    INSERT INTO content_reports (reporter_id, type, target_type, target_id, reason, evidence)
    VALUES ($1, $2, $3, $4, $5, $6)
)";

const std::string REPORT_LIST = R"(
    SELECT r.id, r.reporter_id, u.username, r.type, r.target_type, r.target_id,
           r.reason, r.evidence, r.status, r.handler_id, r.handler_note,
           r.created_at, r.handled_at
    FROM content_reports r
    LEFT JOIN users u ON r.reporter_id = u.id
    ORDER BY r.created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string REPORT_HANDLE = R"(
    UPDATE content_reports SET status = $1, handler_id = $2, 
    handler_note = $3, handled_at = $4
    WHERE id = $5
)";

const std::string MODERATION_ACTION_INSERT = R"(
    INSERT INTO moderation_actions (moderator_id, action_type, target_type, target_id, reason)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string MODERATION_ACTION_LIST = R"(
    SELECT a.id, a.moderator_id, u.username, a.action_type, 
           a.target_type, a.target_id, a.reason, a.duration, a.created_at
    FROM moderation_actions a
    LEFT JOIN users u ON a.moderator_id = u.id
    ORDER BY a.created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string REDEEM_CARD_INSERT = R"(
    INSERT INTO redeem_cards (code, type, value, item_id, item_name, 
                              max_uses, expiry_date, creator_id, batch_no)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
    ON CONFLICT (code) DO NOTHING
)";

const std::string REDEEM_CARD_GET_BY_CODE = R"(
    SELECT id, type, value, item_name, status, max_uses, used_count, expiry_date
    FROM redeem_cards WHERE code = $1 FOR UPDATE
)";

const std::string REDEEM_CARD_UPDATE = R"(
    UPDATE redeem_cards SET used_count = $1, status = $2, 
    used_by_id = $3, used_at = $4 WHERE id = $5
)";

const std::string REDEEM_CARD_LIST = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards
    ORDER BY created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string REDEEM_CARD_COUNT = R"(
    SELECT COUNT(*) FROM redeem_cards
)";

const std::string USER_TITLE_LIST = R"(
    SELECT id, name, color, bg_color, icon, rarity, is_animated
    FROM user_titles WHERE is_active = TRUE
    ORDER BY rarity ASC, id ASC
)";

const std::string USER_ACTIVE_TITLE_SET = R"(
    INSERT INTO user_active_title (user_id, title_id, updated_at)
    VALUES ($1, $2, $3)
    ON CONFLICT (user_id) DO UPDATE SET title_id = $2, updated_at = $3
)";

const std::string AVATAR_FRAME_LIST = R"(
    SELECT id, name, image_url, rarity, is_animated, price
    FROM avatar_frames WHERE is_active = TRUE
    ORDER BY rarity ASC, id ASC
)";

const std::string USER_ACTIVE_FRAME_SET = R"(
    INSERT INTO user_active_frame (user_id, frame_id, updated_at)
    VALUES ($1, $2, $3)
    ON CONFLICT (user_id) DO UPDATE SET frame_id = $2, updated_at = $3
)";

const std::string NAMEPLATE_STYLE_LIST = R"(
    SELECT id, name, card_bg, text_color, effect, rarity
    FROM nameplate_styles WHERE is_active = TRUE
    ORDER BY rarity ASC, id ASC
)";

const std::string PROFILE_THEME_LIST = R"(
    SELECT id, name, bg_image, primary_color, secondary_color, is_premium
    FROM profile_themes WHERE is_active = TRUE
    ORDER BY is_premium ASC, id ASC
)";

const std::string USER_CUSTOMIZATION_SET = R"(
    INSERT INTO user_customization (user_id, active_nameplate_id, active_theme_id, updated_at)
    VALUES ($1, COALESCE($2, 1), COALESCE($3, 1), $4)
    ON CONFLICT (user_id) DO UPDATE SET 
    active_nameplate_id = COALESCE($2, user_customization.active_nameplate_id),
    active_theme_id = COALESCE($3, user_customization.active_theme_id),
    updated_at = $4
)";

const std::string SHOP_ITEM_LIST = R"(
    SELECT id, type, item_id, name, description, price,
           discount_price, stock, sales, is_hot, is_new,
           start_time, end_time, tags
    FROM shop_items WHERE is_active = TRUE
)";

const std::string SHOP_ITEM_COUNT = R"(
    SELECT COUNT(*) FROM shop_items WHERE is_active = TRUE
)";

const std::string SHOP_ITEM_GET_BY_ID = R"(
    SELECT id, type, item_id, name, price, discount_price, stock
    FROM shop_items WHERE id = $1 AND is_active = TRUE FOR UPDATE
)";

const std::string SHOP_ITEM_DEDUCT_STOCK = R"(
    UPDATE shop_items SET stock = stock - $1, sales = sales + $1
    WHERE id = $2
)";

const std::string PURCHASE_HISTORY_INSERT = R"(
    INSERT INTO purchase_history (user_id, shop_item_id, price_paid, quantity, purchased_at)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string DAILY_TASK_GET_USER = R"(
    SELECT t.id, t.name, t.description, t.target_value, t.points_reward,
           COALESCE(p.current_value, 0),
           COALESCE(p.is_completed, FALSE),
           COALESCE(p.is_claimed, FALSE)
    FROM daily_tasks t
    LEFT JOIN user_task_progress p ON t.id = p.task_id 
        AND p.user_id = $1 AND p.last_updated = CURRENT_DATE
    WHERE t.is_active = TRUE
)";

const std::string TASK_PROGRESS_UPDATE = R"(
    INSERT INTO user_task_progress (user_id, task_id, current_value, is_completed, last_updated)
    VALUES ($1, $2, $3, $4, CURRENT_DATE)
    ON CONFLICT (user_id, task_id, last_updated) DO UPDATE SET
    current_value = $3, is_completed = $4
)";

const std::string TASK_REWARD_CLAIM = R"(
    UPDATE user_task_progress SET is_claimed = TRUE
    WHERE user_id = $1 AND task_id = $2 AND last_updated = CURRENT_DATE
)";

const std::string CHECKIN_TODAY = R"(
    SELECT 1 FROM user_checkins 
    WHERE user_id = $1 AND checkin_date = CURRENT_DATE
)";

const std::string CHECKIN_LAST_CONTINUOUS = R"(
    SELECT continuous_days FROM user_checkins
    WHERE user_id = $1 AND checkin_date = CURRENT_DATE - INTERVAL '1 day'
    ORDER BY checkin_date DESC LIMIT 1
)";

const std::string CHECKIN_INSERT = R"(
    INSERT INTO user_checkins (user_id, continuous_days, points_earned, is_bonus, created_at)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string CHECKIN_HISTORY = R"(
    SELECT checkin_date FROM user_checkins
    WHERE user_id = $1
    ORDER BY checkin_date DESC
)";

const std::string POST_SET_ESSENCE = R"(
    UPDATE posts SET is_essence = $1, essence_level = $2 WHERE id = $3
)";

const std::string POST_SET_STICKY = R"(
    UPDATE posts SET is_sticky = $1, sticky_weight = $2, sticky_expiry = $3
    WHERE id = $4
)";

const std::string FOLLOW_SET = R"(
    INSERT INTO user_follows (follower_id, following_id) VALUES ($1, $2) ON CONFLICT DO NOTHING
)";

const std::string FOLLOW_DELETE = R"(
    DELETE FROM user_follows WHERE follower_id = $1 AND following_id = $2
)";

const std::string FOLLOWING_LIST = R"(
    SELECT f.following_id, u.username, u.avatar, u.bio,
           EXISTS(SELECT 1 FROM user_follows 
                  WHERE follower_id = f.following_id AND following_id = $1),
           f.created_at
    FROM user_follows f
    JOIN users u ON f.following_id = u.id
    WHERE f.follower_id = $1
    ORDER BY f.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string FOLLOWER_LIST = R"(
    SELECT f.follower_id, u.username, u.avatar, u.bio,
           EXISTS(SELECT 1 FROM user_follows 
                  WHERE follower_id = $1 AND following_id = f.follower_id),
           f.created_at
    FROM user_follows f
    JOIN users u ON f.follower_id = u.id
    WHERE f.following_id = $1
    ORDER BY f.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string FRIEND_CIRCLE_POSTS = R"(
    SELECT p.id FROM posts p
    WHERE p.author_id IN (
        SELECT following_id FROM user_follows WHERE follower_id = $1
    ) OR p.author_id = $1
    ORDER BY p.is_sticky DESC, p.sticky_weight DESC, p.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string FURSONA_FAVORITE_ADD = R"(
    INSERT INTO fursona_favorites (user_id, fursona_id) VALUES ($1, $2) ON CONFLICT DO NOTHING
)";

const std::string FURSONA_FAVORITE_REMOVE = R"(
    DELETE FROM fursona_favorites WHERE user_id = $1 AND fursona_id = $2
)";

const std::string GIFT_LIST_ALL = R"(
    SELECT id, name, icon, price, animation, is_animated, rarity
    FROM gift_items WHERE is_active = TRUE
    ORDER BY price ASC
)";

const std::string GIFT_SEND = R"(
    INSERT INTO gift_history (from_user_id, to_user_id, gift_id, quantity,
                             total_value, message, is_anonymous, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
)";

const std::string GIFT_USER_HISTORY = R"(
    SELECT gh.from_user_id, u.username, u.avatar,
           gh.gift_id, gi.name, gh.quantity, gh.total_value,
           gh.message, gh.is_anonymous, gh.created_at
    FROM gift_history gh
    JOIN gift_items gi ON gh.gift_id = gi.id
    LEFT JOIN users u ON gh.from_user_id = u.id
    WHERE gh.to_user_id = $1
    ORDER BY gh.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string ARTIST_REVIEW_ADD = R"(
    INSERT INTO artist_reviews (reviewer_id, artist_id, commission_id, 
                               rating, comment, tags, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7)
    ON CONFLICT (reviewer_id, commission_id) DO UPDATE SET
    rating = $4, comment = $5, tags = $6
)";

const std::string ARTIST_REVIEW_LIST = R"(
    SELECT ar.reviewer_id, u.username, u.avatar,
           ar.rating, ar.comment, ar.tags, ar.created_at
    FROM artist_reviews ar
    JOIN users u ON ar.reviewer_id = u.id
    WHERE ar.artist_id = $1
    ORDER BY ar.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string ARTIST_RATING_UPDATE = R"(
    UPDATE users SET 
        artist_rating = (SELECT ROUND(AVG(rating)::numeric, 2) 
                        FROM artist_reviews WHERE artist_id = $1),
        review_count = (SELECT COUNT(*) FROM artist_reviews WHERE artist_id = $1)
    WHERE id = $1
)";

const std::string QUESTION_BOX_CREATE = R"(
    INSERT INTO question_boxes (owner_id, is_public, allow_anonymous, 
                               description, created_at)
    VALUES ($1, $2, $3, $4, $5)
    RETURNING id
)";

const std::string QUESTION_ASK = R"(
    INSERT INTO box_questions (box_id, asker_id, is_anonymous, content, asked_at)
    VALUES ($1, $2, $3, $4, $5)
    RETURNING id
)";

const std::string QUESTION_ANSWER = R"(
    UPDATE box_questions SET answer = $1, is_answered = TRUE, 
           is_public = $2, answered_at = $3
    WHERE id = $4
)";

const std::string COLLECTION_ADD_ITEM = R"(
    INSERT INTO collection_items (collection_id, gallery_item_id, created_at)
    VALUES ($1, $2, EXTRACT(EPOCH FROM NOW()) * 1000)
)";

const std::string CP_CREATE_RELATION = R"(
    INSERT INTO fursona_relations (fursona_a_id, fursona_b_id, relation_type, anniversary, created_at)
    VALUES ($1, $2, $3, $4, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string CP_CONFIRM_RELATION = R"(
    UPDATE fursona_relations SET
    user_a_confirmed = CASE WHEN (SELECT user_id FROM fursonas WHERE id = fursona_a_id) = $2 THEN true ELSE user_a_confirmed END,
    user_b_confirmed = CASE WHEN (SELECT user_id FROM fursonas WHERE id = fursona_b_id) = $2 THEN true ELSE user_b_confirmed END
    WHERE id = $1
)";
const std::string CP_GET_RELATIONS = R"(
    SELECT r.*,
    a.name as fursona_a_name, a.user_id as owner_a,
    b.name as fursona_b_name, b.user_id as owner_b
    FROM fursona_relations r
    JOIN fursonas a ON r.fursona_a_id = a.id
    JOIN fursonas b ON r.fursona_b_id = b.id
    WHERE r.fursona_a_id = $1 OR r.fursona_b_id = $1
)";
const std::string CP_DELETE_RELATION = R"(
    DELETE FROM fursona_relations r WHERE id = $1
    AND (
        (SELECT user_id FROM fursonas WHERE id = r.fursona_a_id) = $2 OR
        (SELECT user_id FROM fursonas WHERE id = r.fursona_b_id) = $2
    )
)";

const std::string WORLD_CREATE = R"(
    INSERT INTO world_settings (user_id, name, description, cover_image, setting_type, tags, is_public, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string WORLD_UPDATE = R"(
    UPDATE world_settings SET
    name = $3, description = $4, cover_image = $5, setting_type = $6, tags = $7, is_public = $8
    WHERE id = $1 AND user_id = $2
)";
const std::string WORLD_GET_BY_USER = R"(
    SELECT w.*, u.username FROM world_settings w
    JOIN users u ON w.user_id = u.id
    WHERE w.user_id = $1 AND ($2 OR w.is_public)
    ORDER BY created_at DESC LIMIT $3 OFFSET $4
)";
const std::string WORLD_GET_PUBLIC = R"(
    SELECT w.*, u.username FROM world_settings w
    JOIN users u ON w.user_id = u.id
    WHERE w.is_public = true
    ORDER BY created_at DESC LIMIT $1 OFFSET $2
)";
const std::string WORLD_GET_BY_ID = R"(
    SELECT w.*, u.username, u.avatar FROM world_settings w
    JOIN users u ON w.user_id = u.id WHERE w.id = $1
)";
const std::string WORLD_COUNT = R"(
    SELECT COUNT(*) FROM world_settings WHERE user_id = $1 AND ($2 OR is_public)
)";
const std::string WORLD_CHECK_LIKE = R"(
    SELECT 1 FROM world_likes WHERE world_id = $1 AND user_id = $2
)";
const std::string WORLD_LIKE = R"(
    INSERT INTO world_likes (world_id, user_id) VALUES ($1, $2)
    ON CONFLICT DO NOTHING
)";
const std::string WORLD_UNLIKE = R"(
    DELETE FROM world_likes WHERE world_id = $1 AND user_id = $2
)";
const std::string WORLD_DELETE = R"(
    DELETE FROM world_settings WHERE id = $1 AND user_id = $2
)";
const std::string WORLD_ADD_PAGE = R"(
    INSERT INTO world_pages (world_id, title, content, page_type, parent_id, sort_order, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string WORLD_GET_PAGES = R"(
    SELECT * FROM world_pages WHERE world_id = $1 ORDER BY sort_order
)";
const std::string WORLD_DELETE_PAGE = R"(
    DELETE FROM world_pages p WHERE p.id = $1 AND p.world_id = $2
    AND EXISTS (SELECT 1 FROM world_settings w WHERE w.id = p.world_id AND w.user_id = $3)
);

const std::string CARD_CREATE = R"(
    INSERT INTO fursona_cards (fursona_id, user_id, template_id, theme_color,
    background_image, show_stats, show_artist, card_layout, created_at, updated_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, EXTRACT(EPOCH FROM NOW()) * 1000, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (fursona_id) DO UPDATE SET
    template_id = $3, theme_color = $4, background_image = $5,
    show_stats = $6, show_artist = $7, card_layout = $8,
    updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
    RETURNING id
)";
const std::string CARD_GET_BY_FURSONA = R"(
    SELECT c.*, f.name as fursona_name, f.species, f.gender
    FROM fursona_cards c
    JOIN fursonas f ON c.fursona_id = f.id
    WHERE c.fursona_id = $1
)";
const std::string CARD_INC_VIEW = R"(
    UPDATE fursona_cards SET view_count = view_count + 1 WHERE fursona_id = $1
)";

const std::string RATING_SET = R"(
    INSERT INTO content_ratings (content_type, content_id, user_id,
    rating_level, content_warnings, rated_by, rated_at)
    VALUES ($1, $2, $3, $4, $5, $6, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (content_type, content_id) DO UPDATE SET
    rating_level = $4, content_warnings = $5, rated_by = $6,
    rated_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";
const std::string RATING_GET = R"(
    SELECT * FROM content_ratings WHERE content_type = $1 AND content_id = $2
)";

const std::string PREFS_GET = R"(
    SELECT * FROM user_content_preferences WHERE user_id = $1
)";
const std::string PREFS_UPDATE = R"(
    INSERT INTO user_content_preferences
    (user_id, show_safe, show_questionable, show_explicit,
    enabled_warnings, blur_sensitive, age_verified, updated_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (user_id) DO UPDATE SET
    show_safe = $2, show_questionable = $3, show_explicit = $4,
    enabled_warnings = $5, blur_sensitive = $6, age_verified = $7,
    updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";

const std::string PERMISSION_REQUEST = R"(
    INSERT INTO creation_permissions (author_user_id, authorized_user_id,
    fursona_id, permission_type, terms, created_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING
    RETURNING id
)";
const std::string PERMISSION_APPROVE = R"(
    UPDATE creation_permissions SET is_approved = true
    WHERE id = $1 AND author_user_id = $2
)";
const std::string PERMISSION_GET_BY_USER = R"(
    SELECT p.*, f.name as fursona_name
    FROM creation_permissions p
    JOIN fursonas f ON p.fursona_id = f.id
    WHERE p.author_user_id = $1 OR p.authorized_user_id = $1
)";

const std::string INTERACTION_ADD = R"(
    INSERT INTO fursona_interactions
    (from_fursona_id, to_fursona_id, interaction_type,
    user_note, intimacy_score, created_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING
)";
const std::string INTERACTION_GET = R"(
    SELECT i.*, f.name as fursona_name FROM fursona_interactions i
    JOIN fursonas f ON i.to_fursona_id = f.id
    WHERE i.from_fursona_id = $1
    ORDER BY intimacy_score DESC
)";

const std::string MOD_SUBMIT = R"(
    INSERT INTO moderation_queue
    (content_type, content_id, submitter_id, submitted_at)
    VALUES ($1, $2, $3, EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string MOD_REVIEW = R"(
    UPDATE moderation_queue SET
    status = $1, moderator_id = $2, moderator_note = $3,
    violation_type = $4, reviewed_at = EXTRACT(EPOCH FROM NOW()) * 1000
    WHERE id = $5
)";
const std::string MOD_GET_QUEUE = R"(
    SELECT * FROM moderation_queue
    WHERE status = $1
    ORDER BY submitted_at DESC LIMIT $2 OFFSET $3
);

const std::string GALLERY_CREATE_ITEM = R"(
    INSERT INTO gallery_items (user_id, fursona_id, title, description,
    file_url, thumbnail_url, file_type, file_size, image_width, image_height,
    artist_name, artist_url, tags, is_public, is_nsfw, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15,
    EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string GALLERY_GET_BY_USER = R"(
    SELECT g.*, u.username, u.avatar FROM gallery_items g
    JOIN users u ON g.user_id = u.id
    WHERE g.user_id = $1 AND ($2 OR g.is_public)
    ORDER BY created_at DESC LIMIT $3 OFFSET $4
)";
const std::string GALLERY_GET_BY_ID = R"(
    SELECT g.*, u.username, u.avatar FROM gallery_items g
    JOIN users u ON g.user_id = u.id
    WHERE g.id = $1
)";
const std::string GALLERY_INC_VIEW = R"(
    UPDATE gallery_items SET view_count = view_count + 1 WHERE id = $1
)";
const std::string GALLERY_LIKE = R"(
    INSERT INTO gallery_likes (item_id, user_id) VALUES ($1, $2)
    ON CONFLICT DO NOTHING;
    UPDATE gallery_items SET like_count = like_count + 1 WHERE id = $1
)";
const std::string GALLERY_UNLIKE = R"(
    DELETE FROM gallery_likes WHERE item_id = $1 AND user_id = $2;
    UPDATE gallery_items SET like_count = like_count - 1 WHERE id = $1
)";
const std::string GALLERY_DELETE = R"(
    DELETE FROM gallery_items WHERE id = $1 AND user_id = $2
)";

const std::string ALBUM_CREATE = R"(
    INSERT INTO gallery_albums (user_id, title, description, cover_image, is_public, created_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string ALBUM_ADD_ITEM = R"(
    INSERT INTO album_items (album_id, item_id, sort_order, added_at)
    VALUES ($1, $2, $3, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING;
    UPDATE gallery_albums SET item_count = item_count + 1 WHERE id = $1
)";
const std::string ALBUM_GET_BY_USER = R"(
    SELECT * FROM gallery_albums
    WHERE user_id = $1 AND ($2 OR is_public)
    ORDER BY created_at DESC
)";

const std::string SEARCH_INDEX_UPSERT = R"(
    INSERT INTO search_index (content_type, content_id, title, content_text,
    author_id, tags, is_public, weight, updated_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (content_type, content_id) DO UPDATE SET
    title = $3, content_text = $4, tags = $6, is_public = $7, weight = $8,
    updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";
const std::string SEARCH_FULLTEXT = R"(
    SELECT *, ts_rank(to_tsvector('english', title || ' ' || content_text),
           plainto_tsquery('english', $1)) as rank
    FROM search_index
    WHERE to_tsvector('english', title || ' ' || content_text) @@ plainto_tsquery('english', $1)
    AND is_public = true
    ORDER BY rank DESC, weight DESC, updated_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string PRESENCE_UPDATE = R"(
    INSERT INTO user_presence (user_id, status, last_active, last_ip, user_agent)
    VALUES ($1, $2, EXTRACT(EPOCH FROM NOW()) * 1000, $3, $4)
    ON CONFLICT (user_id) DO UPDATE SET
    status = $2, last_active = EXTRACT(EPOCH FROM NOW()) * 1000,
    last_ip = $3, user_agent = $4
)";
const std::string PRESENCE_GET = R"(
    SELECT user_id, CASE WHEN is_invisible THEN 'offline' ELSE status END as status,
    CASE WHEN is_invisible THEN 0 ELSE last_active END as last_active
    FROM user_presence WHERE user_id = $1
)";
const std::string PRESENCE_GET_ONLINE = R"(
    SELECT user_id, status, last_active FROM user_presence
    WHERE status = 'online' AND is_invisible = false
    ORDER BY last_active DESC LIMIT $1
)";

const std::string EXPORT_CREATE_TASK = R"(
    INSERT INTO export_tasks (user_id, task_type, status, created_at)
    VALUES ($1, $2, 'pending', EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string EXPORT_GET_TASKS = R"(
    SELECT * FROM export_tasks WHERE user_id = $1 ORDER BY created_at DESC LIMIT 10
)";

const std::string CONFIG_GET_ALL = R"(
    SELECT config_key, config_value, config_type, is_public FROM system_configs
)";
const std::string CONFIG_SET = R"(
    INSERT INTO system_configs (config_key, config_value, config_type, description, updated_at)
    VALUES ($1, $2, $3, $4, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (config_key) DO UPDATE SET
    config_value = $2, config_type = $3, description = $4,
    updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";

const std::string AUDIT_LOG = R"(
    INSERT INTO audit_logs (user_id, action, resource_type, resource_id,
    old_value, new_value, ip_address, user_agent, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string AUDIT_GET_BY_USER = R"(
    SELECT * FROM audit_logs WHERE user_id = $1
    ORDER BY created_at DESC LIMIT $2 OFFSET $3
);

const std::string IP_BLACKLIST_ADD = R"(
    INSERT INTO ip_blacklist (ip_address, reason, banned_by, expires_at, created_at)
    VALUES ($1, $2, $3, $4, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (ip_address) DO UPDATE SET
    reason = $2, banned_by = $3, expires_at = $4
)";
const std::string IP_BLACKLIST_CHECK = R"(
    SELECT 1 FROM ip_blacklist
    WHERE ip_address = $1 AND (expires_at IS NULL OR expires_at > EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string IP_BLACKLIST_REMOVE = R"(
    DELETE FROM ip_blacklist WHERE ip_address = $1
)";

const std::string RATE_LIMIT_INC = R"(
    INSERT INTO rate_limits (identifier, limit_type, request_count, window_start)
    VALUES ($1, $2, 1, $3)
    ON CONFLICT (identifier, limit_type) DO UPDATE SET
    request_count = rate_limits.request_count + 1
    RETURNING request_count
)";
const std::string RATE_LIMIT_RESET = R"(
    DELETE FROM rate_limits WHERE identifier = $1 AND limit_type = $2
)";

const std::string SESSION_CREATE = R"(
    INSERT INTO user_sessions (session_id, user_id, device_info, ip_address,
    location, user_agent, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string SESSION_GET_BY_USER = R"(
    SELECT * FROM user_sessions WHERE user_id = $1 ORDER BY last_active DESC
)";
const std::string SESSION_REVOKE = R"(
    DELETE FROM user_sessions WHERE session_id = $1 AND user_id = $2
)";
const std::string SESSION_UPDATE = R"(
    UPDATE user_sessions SET last_active = EXTRACT(EPOCH FROM NOW()) * 1000
    WHERE session_id = $1
)";

const std::string LOGIN_HISTORY_ADD = R"(
    INSERT INTO login_history (user_id, ip_address, location, device_info,
    user_agent, was_successful, failure_reason, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string LOGIN_HISTORY_GET = R"(
    SELECT * FROM login_history WHERE user_id = $1
    ORDER BY created_at DESC LIMIT $2 OFFSET $3
)";

const std::string USER_BLOCK_ADD = R"(
    INSERT INTO user_blocks (blocker_id, blocked_id, block_type, reason, created_at)
    VALUES ($1, $2, $3, $4, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING
)";
const std::string USER_BLOCK_REMOVE = R"(
    DELETE FROM user_blocks WHERE blocker_id = $1 AND blocked_id = $2
)";
const std::string USER_BLOCK_CHECK = R"(
    SELECT 1 FROM user_blocks WHERE blocker_id = $1 AND blocked_id = $2
)";
const std::string USER_BLOCK_LIST = R"(
    SELECT blocked_id, block_type, created_at FROM user_blocks WHERE blocker_id = $1
)";

const std::string ANNOUNCEMENT_CREATE = R"(
    INSERT INTO system_announcements (title, content, announcement_type,
    priority, show_until, created_by, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string ANNOUNCEMENT_GET_ACTIVE = R"(
    SELECT * FROM system_announcements
    WHERE is_active = true AND (show_until IS NULL OR show_until > EXTRACT(EPOCH FROM NOW()) * 1000)
    ORDER BY priority DESC, created_at DESC LIMIT 10
)";

const std::string WEBHOOK_CREATE = R"(
    INSERT INTO webhook_endpoints (user_id, endpoint_url, secret, events, created_at)
    VALUES ($1, $2, $3, $4, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string WEBHOOK_GET_BY_USER = R"(
    SELECT * FROM webhook_endpoints WHERE user_id = $1
)";
const std::string WEBHOOK_LOG = R"(
    INSERT INTO webhook_logs (endpoint_id, event_type, request_body,
    response_status, response_body, created_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
)";

const std::string ALERT_CREATE = R"(
    INSERT INTO security_alerts (user_id, alert_type, severity, details,
    ip_address, created_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string ALERT_GET_USER = R"(
    SELECT * FROM security_alerts WHERE user_id = $1 AND is_resolved = false
    ORDER BY created_at DESC
)";
const std::string ALERT_RESOLVE = R"(
    UPDATE security_alerts SET is_resolved = true WHERE id = $1 AND user_id = $2
)";

const std::string REPORT_CREATE = R"(
    INSERT INTO content_reports (reporter_id, content_type, content_id,
    report_reason, report_details, created_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING RETURNING id
)";
const std::string REPORT_GET_PENDING = R"(
    SELECT * FROM content_reports WHERE status = 'pending'
    ORDER BY created_at DESC LIMIT $1 OFFSET $2
)";
const std::string REPORT_HANDLE = R"(
    UPDATE content_reports SET status = $1, handled_by = $2,
    handled_at = EXTRACT(EPOCH FROM NOW()) * 1000, handler_notes = $3
    WHERE id = $4
)";

const std::string REPLY_CREATE = R"(
    INSERT INTO comment_replies (comment_id, parent_reply_id, post_id,
    user_id, reply_to_user_id, content, created_at)
    VALUES ($1, $2, $3, $4, $5, $6, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string REPLY_GET_BY_COMMENT = R"(
    SELECT * FROM comment_replies WHERE comment_id = $1 AND is_deleted = false
    ORDER BY created_at ASC LIMIT 50
)";
const std::string REPLY_DELETE = R"(
    UPDATE comment_replies SET is_deleted = true WHERE id = $1 AND user_id = $2
)";
const std::string REPLY_LIKE = R"(
    UPDATE comment_replies SET like_count = like_count + 1 WHERE id = $1
)";

const std::string STICKY_SET = R"(
    INSERT INTO post_sticky (post_id, section_id, priority, sticky_by)
    VALUES ($1, $2, $3, $4)
    ON CONFLICT (post_id) DO UPDATE SET priority = $3
)";
const std::string STICKY_REMOVE = R"(
    DELETE FROM post_sticky WHERE post_id = $1
)";
const std::string STICKY_GET_BY_SECTION = R"(
    SELECT p.*, ps.priority FROM posts p
    JOIN post_sticky ps ON p.id = ps.post_id
    WHERE ps.section_id = $1 AND p.is_deleted = false
    ORDER BY ps.priority DESC
)";

const std::string DIGEST_SET = R"(
    INSERT INTO post_digest (post_id, digest_level, recommended_by, description)
    VALUES ($1, $2, $3, $4)
    ON CONFLICT (post_id) DO UPDATE SET digest_level = $2
)";
const std::string DIGEST_REMOVE = R"(
    DELETE FROM post_digest WHERE post_id = $1
)";
const std::string DIGEST_GET_BY_LEVEL = R"(
    SELECT p.*, pd.digest_level FROM posts p
    JOIN post_digest pd ON p.id = pd.post_id
    WHERE pd.digest_level = $1 AND p.is_deleted = false
    ORDER BY pd.recommended_at DESC LIMIT $2 OFFSET $3
)";

const std::string FOLDER_CREATE = R"(
    INSERT INTO collection_folders (user_id, name, description, is_public, created_at)
    VALUES ($1, $2, $3, $4, EXTRACT(EPOCH FROM NOW()) * 1000)
    RETURNING id
)";
const std::string FOLDER_GET_BY_USER = R"(
    SELECT * FROM collection_folders WHERE user_id = $1 ORDER BY created_at DESC
)";
const std::string FOLDER_ADD_ITEM = R"(
    INSERT INTO collection_items (folder_id, post_id, saved_at)
    VALUES ($1, $2, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING;
    UPDATE collection_folders SET item_count = item_count + 1 WHERE id = $1
)";
const std::string FOLDER_REMOVE_ITEM = R"(
    DELETE FROM collection_items WHERE folder_id = $1 AND post_id = $2;
    UPDATE collection_folders SET item_count = item_count - 1 WHERE id = $1
)";
const std::string FOLDER_GET_ITEMS = R"(
    SELECT p.* FROM posts p JOIN collection_items ci ON p.id = ci.post_id
    WHERE ci.folder_id = $1 AND p.is_deleted = false
    ORDER BY ci.saved_at DESC LIMIT $2 OFFSET $3
)";

const std::string USER_TAG_ADD = R"(
    INSERT INTO user_tags (tagger_id, tagged_id, tag, created_at)
    VALUES ($1, $2, $3, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING
)";
const std::string USER_TAG_REMOVE = R"(
    DELETE FROM user_tags WHERE tagger_id = $1 AND tagged_id = $2 AND tag = $3
)";
const std::string USER_TAG_GET = R"(
    SELECT tag FROM user_tags WHERE tagger_id = $1 AND tagged_id = $2
)";

const std::string KEYWORD_FILTER_ADD = R"(
    INSERT INTO keyword_filters (user_id, keyword, filter_type, created_at)
    VALUES ($1, $2, $3, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT DO NOTHING
)";
const std::string KEYWORD_FILTER_REMOVE = R"(
    DELETE FROM keyword_filters WHERE user_id = $1 AND id = $2
)";
const std::string KEYWORD_FILTER_GET = R"(
    SELECT keyword, filter_type FROM keyword_filters WHERE user_id = $1
)";

const std::string DRAFT_SAVE = R"(
    INSERT INTO post_drafts (user_id, title, content, section_id,
    tags, fursona_id, is_auto_save, updated_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, EXTRACT(EPOCH FROM NOW()) * 1000)
)";
const std::string DRAFT_GET_BY_USER = R"(
    SELECT * FROM post_drafts WHERE user_id = $1 ORDER BY updated_at DESC LIMIT 20
)";
const std::string DRAFT_DELETE = R"(
    DELETE FROM post_drafts WHERE id = $1 AND user_id = $2
)";

const std::string STATS_INC_VIEW = R"(
    INSERT INTO post_share_stats (post_id, view_count, last_viewed_at)
    VALUES ($1, 1, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (post_id) DO UPDATE SET
    view_count = post_share_stats.view_count + 1,
    last_viewed_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";
const std::string STATS_INC_SHARE = R"(
    INSERT INTO post_share_stats (post_id, share_count) VALUES ($1, 1)
    ON CONFLICT (post_id) DO UPDATE SET
    share_count = post_share_stats.share_count + 1
)";
const std::string STATS_GET = R"(
    SELECT * FROM post_share_stats WHERE post_id = $1
)";

const std::string MOD_ADD = R"(
    INSERT INTO section_moderators (section_id, user_id, assigned_by,
    permission_level, can_manage_posts, can_manage_comments,
    can_manage_users, can_manage_reports)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
    ON CONFLICT DO NOTHING
)";
const std::string MOD_REMOVE = R"(
    DELETE FROM section_moderators WHERE section_id = $1 AND user_id = $2
)";
const std::string MOD_GET_BY_USER = R"(
    SELECT * FROM section_moderators WHERE user_id = $1
)";
const std::string MOD_CHECK_PERM = R"(
    SELECT 1 FROM section_moderators
    WHERE user_id = $1 AND section_id = $2 AND can_manage_posts = true
)";

const std::string PUNISH_CREATE = R"(
    INSERT INTO user_punishments (user_id, punishment_type, reason,
    duration, points_deducted, executed_by, expires_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7)
)";
const std::string PUNISH_GET_ACTIVE = R"(
    SELECT * FROM user_punishments
    WHERE user_id = $1 AND is_active = true
    AND (expires_at IS NULL OR expires_at > EXTRACT(EPOCH FROM NOW()) * 1000)
    ORDER BY created_at DESC
)";
const std::string PUNISH_EXPIRE = R"(
    UPDATE user_punishments SET is_active = false
    WHERE expires_at <= EXTRACT(EPOCH FROM NOW()) * 1000
)";
const std::string PUNISH_HISTORY = R"(
    INSERT INTO punishment_records (user_id, punishment_type, reason,
    points_deducted, executed_by)
    VALUES ($1, $2, $3, $4, $5)
)";

const std::string POLL_CREATE = R"(
    INSERT INTO post_polls (post_id, question, options, is_multiple, end_at)
    VALUES ($1, $2, $3, $4, $5)
)";
const std::string POLL_GET = R"(
    SELECT * FROM post_polls WHERE post_id = $1
)";
const std::string POLL_VOTE = R"(
    INSERT INTO poll_votes (poll_id, user_id, option_index)
    VALUES ($1, $2, $3) ON CONFLICT DO NOTHING;
    UPDATE post_polls SET vote_counts[$4] = vote_counts[$4] + 1
    WHERE post_id = $1
)";
const std::string POLL_CHECK_VOTE = R"(
    SELECT 1 FROM poll_votes WHERE poll_id = $1 AND user_id = $2
)";

const std::string HOT_UPDATE_SCORE = R"(
    INSERT INTO hot_scores (post_id, hot_score, view_weight,
    like_weight, comment_weight, last_updated_at)
    VALUES ($1, $2, $3, $4, $5, EXTRACT(EPOCH FROM NOW()) * 1000)
    ON CONFLICT (post_id) DO UPDATE SET
    hot_score = $2, view_weight = $3, like_weight = $4,
    comment_weight = $5, last_updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";
const std::string HOT_GET_TOP = R"(
    SELECT post_id FROM hot_scores
    ORDER BY hot_score DESC LIMIT $1 OFFSET $2
)";

const std::string FEED_SETTINGS_GET = R"(
    SELECT * FROM user_feed_settings WHERE user_id = $1
)";
const std::string FEED_SETTINGS_SET = R"(
    INSERT INTO user_feed_settings (user_id, feed_type, include_sections, exclude_tags)
    VALUES ($1, $2, $3, $4)
    ON CONFLICT (user_id) DO UPDATE SET
    feed_type = $2, include_sections = $3, exclude_tags = $4,
    updated_at = EXTRACT(EPOCH FROM NOW()) * 1000
)";

const std::string WATERMARK_GET = R"(
    SELECT * FROM media_watermarks WHERE user_id = $1
)";
const std::string WATERMARK_SET = R"(
    INSERT INTO media_watermarks (user_id, watermark_text,
    watermark_position, opacity, is_enabled)
    VALUES ($1, $2, $3, $4, $5)
    ON CONFLICT (user_id) DO UPDATE SET
    watermark_text = $2, watermark_position = $3,
    opacity = $4, is_enabled = $5
)";

const std::string RECOMMEND_LOG = R"(
    INSERT INTO recommendation_logs (user_id, post_id, algorithm, action, score)
    VALUES ($1, $2, $3, $4, $5)
)";

} // namespace furbbs::db::sql

#endif // FURBBS_DB_SQL_QUERIES_H

const std::string COMMENT_LIKE_ADD = R"(
    INSERT INTO comment_likes (comment_id, user_id)
    VALUES ($1, $2) ON CONFLICT DO NOTHING
)";

const std::string COMMENT_LIKE_REMOVE = R"(
    DELETE FROM comment_likes WHERE comment_id = $1 AND user_id = $2
)";

const std::string COMMENT_LIKE_COUNT = R"(
    SELECT COUNT(*) FROM comment_likes WHERE comment_id = $1
)";

const std::string COMMENT_LIKE_HAS = R"(
    SELECT 1 FROM comment_likes WHERE comment_id = $1 AND user_id = $2 LIMIT 1
)";

const std::string POST_APPRECIATION_ADD = R"(
    INSERT INTO post_appreciations (post_id, user_id, score, comment)
    VALUES ($1, $2, $3, $4)
    ON CONFLICT (post_id, user_id) DO UPDATE SET score = $3, comment = $4
)";

const std::string POST_APPRECIATION_GET = R"(
    SELECT pa.id, pa.post_id, pa.user_id, u.username, pa.score,
           pa.comment, pa.created_at
    FROM post_appreciations pa
    JOIN users u ON pa.user_id = u.id
    WHERE pa.post_id = $1
    ORDER BY pa.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string POST_APPRECIATION_STATS = R"(
    SELECT COUNT(*), AVG(score) FROM post_appreciations WHERE post_id = $1
)";

const std::string SHARE_RECORD_ADD = R"(
    INSERT INTO share_records (post_id, user_id, platform)
    VALUES ($1, $2, $3)
)";

const std::string SHARE_RECORD_COUNT = R"(
    SELECT COUNT(*) FROM share_records WHERE post_id = $1
)";

const std::string SHARE_PLATFORM_STATS = R"(
    SELECT platform, COUNT(*) FROM share_records 
    WHERE post_id = $1 GROUP BY platform
)";

const std::string VISIT_RECORD_ADD = R"(
    INSERT INTO visit_records (visitor_id, target_user_id, post_id,
                               ip_address, user_agent, duration)
    VALUES ($1, $2, $3, $4, $5, $6)
)";

const std::string VISIT_GET_PROFILE_VISITORS = R"(
    SELECT DISTINCT ON (visitor_id) v.visitor_id, u.username, u.avatar,
           MAX(v.visited_at) as last_visit, COUNT(*) as visit_count
    FROM visit_records v
    JOIN users u ON v.visitor_id = u.id
    WHERE v.target_user_id = $1 AND v.visitor_id IS NOT NULL
    GROUP BY v.visitor_id, u.username, u.avatar
    ORDER BY last_visit DESC
    LIMIT $2 OFFSET $3
)";

const std::string USER_TAG_ADD = R"(
    INSERT INTO user_tags (user_id, tagged_user_id, tag_name, tag_color)
    VALUES ($1, $2, $3, $4) ON CONFLICT DO NOTHING
)";

const std::string USER_TAG_REMOVE = R"(
    DELETE FROM user_tags WHERE user_id = $1 AND tagged_user_id = $2 AND tag_name = $3
)";

const std::string USER_TAG_GET = R"(
    SELECT tag_name, tag_color FROM user_tags 
    WHERE user_id = $1 AND tagged_user_id = $2
    ORDER BY id
)";

const std::string READING_PROGRESS_SET = R"(
    INSERT INTO reading_progress (user_id, post_id, progress_percent,
                                  last_read_position, total_words,
                                  last_read_at, is_completed)
    VALUES ($1, $2, $3, $4, $5, $6, $7)
    ON CONFLICT (user_id, post_id) DO UPDATE SET
    progress_percent = $3, last_read_position = $4,
    last_read_at = $6, is_completed = $7
)";

const std::string READING_PROGRESS_GET = R"(
    SELECT progress_percent, last_read_position, total_words, is_completed
    FROM reading_progress WHERE user_id = $1 AND post_id = $2
)";

const std::string READING_HISTORY = R"(
    SELECT rp.post_id, p.title, rp.last_read_at, rp.progress_percent
    FROM reading_progress rp
    JOIN posts p ON rp.post_id = p.id
    WHERE rp.user_id = $1
    ORDER BY rp.last_read_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string USER_NOTE_ADD = R"(
    INSERT INTO user_notes (user_id, target_type, target_id, note_content,
                            color, is_pinned, updated_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7)
)";

const std::string USER_NOTE_UPDATE = R"(
    UPDATE user_notes SET note_content = $1, color = $2,
    is_pinned = $3, updated_at = $4 WHERE id = $5 AND user_id = $6
)";

const std::string USER_NOTE_DELETE = R"(
    DELETE FROM user_notes WHERE id = $1 AND user_id = $2
)";

const std::string USER_NOTE_GET = R"(
    SELECT id, target_type, target_id, note_content, color,
           is_pinned, created_at, updated_at
    FROM user_notes WHERE user_id = $1 AND target_type = $2 AND target_id = $3
    ORDER BY is_pinned DESC, updated_at DESC
)";

const std::string CONTENT_HISTORY_ADD = R"(
    INSERT INTO content_history (post_id, comment_id, edited_by,
                                 old_title, old_content, new_title,
                                 new_content, edit_reason)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
)";

const std::string CONTENT_HISTORY_GET = R"(
    SELECT id, edited_by, old_title, old_content, new_title,
           new_content, edit_reason, edited_at
    FROM content_history WHERE post_id = $1 OR comment_id = $2
    ORDER BY edited_at DESC
    LIMIT $3 OFFSET $4
)";

const std::string QUICKACCESS_ADD = R"(
    INSERT INTO quick_access (user_id, item_type, item_id, item_name,
                               item_icon, sort_order)
    VALUES ($1, $2, $3, $4, $5, $6) ON CONFLICT DO NOTHING
)";

const std::string QUICKACCESS_REMOVE = R"(
    DELETE FROM quick_access 
    WHERE user_id = $1 AND item_type = $2 AND item_id = $3
)";

const std::string QUICKACCESS_GET = R"(
    SELECT item_type, item_id, item_name, item_icon, sort_order
    FROM quick_access WHERE user_id = $1
    ORDER BY sort_order, id
)";

const std::string WORDFILTER_GET_ALL = R"(
    SELECT word, replacement, filter_level, is_regex FROM word_filter
    ORDER BY filter_level DESC
)";

const std::string WORDFILTER_ADD = R"(
    INSERT INTO word_filter (word, replacement, filter_level, is_regex, created_by)
    VALUES ($1, $2, $3, $4, $5) ON CONFLICT DO NOTHING
)";

const std::string WORDFILTER_REMOVE = R"(
    DELETE FROM word_filter WHERE id = $1
)";

const std::string SERIES_CREATE = R"(
    INSERT INTO post_series (user_id, title, description, cover_image, is_public)
    VALUES ($1, $2, $3, $4, $5) RETURNING id
)";

const std::string SERIES_UPDATE = R"(
    UPDATE post_series SET title = $1, description = $2, cover_image = $3,
    is_public = $4, updated_at = $5 WHERE id = $6 AND user_id = $7
)";

const std::string SERIES_DELETE = R"(
    DELETE FROM post_series WHERE id = $1 AND user_id = $2
)";

const std::string SERIES_GET_BY_USER = R"(
    SELECT id, title, description, cover_image, is_public,
           post_count, view_count, created_at
    FROM post_series WHERE user_id = $1
    ORDER BY updated_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string SERIES_POST_ADD = R"(
    INSERT INTO series_posts (series_id, post_id, sort_order)
    VALUES ($1, $2, $3) ON CONFLICT DO NOTHING
)";

const std::string SERIES_POST_REMOVE = R"(
    DELETE FROM series_posts WHERE series_id = $1 AND post_id = $2
)";

const std::string SERIES_POSTS_GET = R"(
    SELECT sp.post_id, p.title, sp.sort_order, sp.added_at
    FROM series_posts sp
    JOIN posts p ON sp.post_id = p.id
    WHERE sp.series_id = $1
    ORDER BY sp.sort_order, sp.added_at
)";

const std::string PREFERENCES_GET = R"(
    SELECT content_language, default_sort, show_nsfw, blur_nsfw,
           auto_play_video, infinite_scroll, compact_mode, night_mode,
           font_size, notify_on_like, notify_on_comment, notify_on_follow,
           notify_on_mention, notify_on_message, email_digest,
           show_online_status, show_read_status
    FROM user_preferences WHERE user_id = $1
)";

const std::string PREFERENCES_SET = R"(
    INSERT INTO user_preferences (user_id, content_language, default_sort,
        show_nsfw, blur_nsfw, auto_play_video, infinite_scroll,
        compact_mode, night_mode, font_size, notify_on_like,
        notify_on_comment, notify_on_follow, notify_on_mention,
        notify_on_message, email_digest, show_online_status,
        show_read_status, updated_at)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13,
            $14, $15, $16, $17, $18, $19)
    ON CONFLICT (user_id) DO UPDATE SET
    content_language = $2, default_sort = $3, show_nsfw = $4,
    blur_nsfw = $5, auto_play_video = $6, infinite_scroll = $7,
    compact_mode = $8, night_mode = $9, font_size = $10,
    notify_on_like = $11, notify_on_comment = $12, notify_on_follow = $13,
    notify_on_mention = $14, notify_on_message = $15, email_digest = $16,
    show_online_status = $17, show_read_status = $18, updated_at = $19
)";

const std::string RANKING_UPDATE_CONTRIBUTOR = R"(
    INSERT INTO contributor_ranking (user_id, contribution_score,
        posts_weight, comments_weight, likes_weight, uploads_weight,
        help_weight, last_updated)
    VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
    ON CONFLICT (user_id) DO UPDATE SET
    contribution_score = $2, posts_weight = $3, comments_weight = $4,
    likes_weight = $5, uploads_weight = $6, help_weight = $7,
    last_updated = $8
)";

const std::string RANKING_GET_TOP = R"(
    SELECT cr.user_id, u.username, u.avatar, cr.contribution_score,
           cr.posts_weight, cr.comments_weight, cr.likes_weight
    FROM contributor_ranking cr
    JOIN users u ON cr.user_id = u.id
    ORDER BY cr.contribution_score DESC
    LIMIT $1 OFFSET $2
)";


const std::string GALLERY_COUNT_BASE = R"(SELECT COUNT(*) FROM galleries WHERE user_id = $1)";
const std::string GALLERY_COUNT_PUBLIC = R"(SELECT COUNT(*) FROM galleries WHERE user_id = $1 AND is_public = TRUE)";

const std::string THEME_COUNT_ALL = R"(SELECT COUNT(*) FROM user_themes)";
const std::string THEME_COUNT_PUBLIC = R"(SELECT COUNT(*) FROM user_themes WHERE is_public = TRUE)";
const std::string THEME_COUNT_BY_USER = R"(SELECT COUNT(*) FROM user_themes WHERE creator_id = $1)";
const std::string THEME_COUNT_PUBLIC_BY_USER = R"(SELECT COUNT(*) FROM user_themes WHERE is_public = TRUE AND creator_id = $1)";

const std::string QUESTION_COUNT_BASE = R"(SELECT COUNT(*) FROM box_questions WHERE box_id = $1)";
const std::string QUESTION_COUNT_UNANSWERED = R"(SELECT COUNT(*) FROM box_questions WHERE box_id = $1 AND is_answered = FALSE)";

const std::string SHOP_ITEMS_COUNT_ALL = R"(SELECT COUNT(*) FROM shop_items WHERE is_active = TRUE)";
const std::string SHOP_ITEMS_COUNT_BY_TYPE = R"(SELECT COUNT(*) FROM shop_items WHERE is_active = TRUE AND type = $1)";
const std::string SHOP_ITEMS_COUNT_ON_SALE = R"(SELECT COUNT(*) FROM shop_items WHERE is_active = TRUE AND discount_price IS NOT NULL)";
const std::string SHOP_ITEMS_COUNT_BY_TYPE_ON_SALE = R"(SELECT COUNT(*) FROM shop_items WHERE is_active = TRUE AND type = $1 AND discount_price IS NOT NULL)";

const std::string GIFT_GET_ALL = R"(
    SELECT id, name, icon, price, animation, is_animated, rarity
    FROM gift_items WHERE is_active = TRUE
    ORDER BY price ASC
)";

const std::string GALLERY_LIST_BASE = R"(
    SELECT g.id, g.name, g.description, g.cover_image, g.is_public,
           g.user_id, g.created_at, g.item_count, g.view_count,
           g.like_count, u.username, u.avatar,
           EXISTS(SELECT 1 FROM gallery_favorites WHERE gallery_id = g.id AND user_id = $2) as is_favorited
    FROM galleries g
    JOIN users u ON g.user_id = u.id
    WHERE g.user_id = $1
)";

const std::string GALLERY_LIST_PUBLIC = R"(
    SELECT g.id, g.name, g.description, g.cover_image, g.is_public,
           g.user_id, g.created_at, g.item_count, g.view_count,
           g.like_count, u.username, u.avatar,
           EXISTS(SELECT 1 FROM gallery_favorites WHERE gallery_id = g.id AND user_id = $2) as is_favorited
    FROM galleries g
    JOIN users u ON g.user_id = u.id
    WHERE g.user_id = $1 AND g.is_public = TRUE
    ORDER BY g.created_at DESC
    LIMIT $3 OFFSET $4
)";

const std::string GALLERY_LIST_FULL = R"(
    SELECT g.id, g.name, g.description, g.cover_image, g.is_public,
           g.user_id, g.created_at, g.item_count, g.view_count,
           g.like_count, u.username, u.avatar,
           EXISTS(SELECT 1 FROM gallery_favorites WHERE gallery_id = g.id AND user_id = $2) as is_favorited
    FROM galleries g
    JOIN users u ON g.user_id = u.id
    WHERE g.user_id = $1
    ORDER BY g.created_at DESC
    LIMIT $3 OFFSET $4
)";

const std::string THEMES_LIST_ALL = R"(
    SELECT t.id, t.name, t.creator_id, u.username,
           t.primary_color, t.secondary_color,
           t.accent_color, t.bg_color, t.card_bg_color,
           t.text_color, t.is_public, t.use_count, t.created_at
    FROM user_themes t
    LEFT JOIN users u ON t.creator_id = u.id
    ORDER BY t.use_count DESC, t.created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string THEMES_LIST_PUBLIC = R"(
    SELECT t.id, t.name, t.creator_id, u.username,
           t.primary_color, t.secondary_color,
           t.accent_color, t.bg_color, t.card_bg_color,
           t.text_color, t.is_public, t.use_count, t.created_at
    FROM user_themes t
    LEFT JOIN users u ON t.creator_id = u.id
    WHERE t.is_public = TRUE
    ORDER BY t.use_count DESC, t.created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string THEMES_LIST_BY_USER = R"(
    SELECT t.id, t.name, t.creator_id, u.username,
           t.primary_color, t.secondary_color,
           t.accent_color, t.bg_color, t.card_bg_color,
           t.text_color, t.is_public, t.use_count, t.created_at
    FROM user_themes t
    LEFT JOIN users u ON t.creator_id = u.id
    WHERE t.creator_id = $1
    ORDER BY t.use_count DESC, t.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string THEMES_LIST_PUBLIC_BY_USER = R"(
    SELECT t.id, t.name, t.creator_id, u.username,
           t.primary_color, t.secondary_color,
           t.accent_color, t.bg_color, t.card_bg_color,
           t.text_color, t.is_public, t.use_count, t.created_at
    FROM user_themes t
    LEFT JOIN users u ON t.creator_id = u.id
    WHERE t.is_public = TRUE AND t.creator_id = $1
    ORDER BY t.use_count DESC, t.created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string COLLECTION_LIST_BY_USER = R"(
    SELECT id, name, description, cover_image,
           is_public, item_count, created_at
    FROM collections
    WHERE user_id = $1
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string COLLECTION_LIST_BY_USER_PUBLIC = R"(
    SELECT id, name, description, cover_image,
           is_public, item_count, created_at
    FROM collections
    WHERE user_id = $1 AND is_public = TRUE
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string QUESTION_LIST_BASE = R"(
    SELECT q.id, q.asker_id, u.username, u.avatar,
           q.is_anonymous, q.content, q.answer, q.is_answered,
           q.is_public, q.asked_at, q.answered_at
    FROM box_questions q
    LEFT JOIN users u ON q.asker_id = u.id
    WHERE q.box_id = $1
    ORDER BY q.asked_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string QUESTION_LIST_UNANSWERED = R"(
    SELECT q.id, q.asker_id, u.username, u.avatar,
           q.is_anonymous, q.content, q.answer, q.is_answered,
           q.is_public, q.asked_at, q.answered_at
    FROM box_questions q
    LEFT JOIN users u ON q.asker_id = u.id
    WHERE q.box_id = $1 AND q.is_answered = FALSE
    ORDER BY q.asked_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string QUESTION_LIST_PUBLIC = R"(
    SELECT q.id, q.asker_id, u.username, u.avatar,
           q.is_anonymous, q.content, q.answer, q.is_answered,
           q.is_public, q.asked_at, q.answered_at
    FROM box_questions q
    LEFT JOIN users u ON q.asker_id = u.id
    WHERE q.box_id = $1 AND q.is_public = TRUE
    ORDER BY q.asked_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string QUESTION_LIST_PUBLIC_UNANSWERED = R"(
    SELECT q.id, q.asker_id, u.username, u.avatar,
           q.is_anonymous, q.content, q.answer, q.is_answered,
           q.is_public, q.asked_at, q.answered_at
    FROM box_questions q
    LEFT JOIN users u ON q.asker_id = u.id
    WHERE q.box_id = $1 AND q.is_public = TRUE AND q.is_answered = FALSE
    ORDER BY q.asked_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string SHOP_ITEMS_BASE = R"(
    SELECT id, type, item_id, name, description, price,
           discount_price, stock, sales, is_hot, is_new,
           start_time, end_time, tags
    FROM shop_items WHERE is_active = TRUE
    ORDER BY sales DESC
    LIMIT $1 OFFSET $2
)";

const std::string SHOP_ITEMS_BY_TYPE = R"(
    SELECT id, type, item_id, name, description, price,
           discount_price, stock, sales, is_hot, is_new,
           start_time, end_time, tags
    FROM shop_items WHERE is_active = TRUE AND type = $1
    ORDER BY sales DESC
    LIMIT $2 OFFSET $3
)";

const std::string SHOP_ITEMS_ON_SALE = R"(
    SELECT id, type, item_id, name, description, price,
           discount_price, stock, sales, is_hot, is_new,
           start_time, end_time, tags
    FROM shop_items WHERE is_active = TRUE AND discount_price IS NOT NULL
    ORDER BY sales DESC
    LIMIT $1 OFFSET $2
)";

const std::string SHOP_ITEMS_BY_TYPE_ON_SALE = R"(
    SELECT id, type, item_id, name, description, price,
           discount_price, stock, sales, is_hot, is_new,
           start_time, end_time, tags
    FROM shop_items WHERE is_active = TRUE AND type = $1 AND discount_price IS NOT NULL
    ORDER BY sales DESC
    LIMIT $2 OFFSET $3
)";

const std::string REDEEM_CARD_LIST_ALL = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards
    ORDER BY created_at DESC
    LIMIT $1 OFFSET $2
)";

const std::string REDEEM_CARD_LIST_BY_STATUS = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE status = $1
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string REDEEM_CARD_LIST_BY_TYPE = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE type = $1
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string REDEEM_CARD_LIST_BY_BATCH = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE batch_no = $1
    ORDER BY created_at DESC
    LIMIT $2 OFFSET $3
)";

const std::string REDEEM_CARD_LIST_BY_STATUS_TYPE = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE status = $1 AND type = $2
    ORDER BY created_at DESC
    LIMIT $3 OFFSET $4
)";

const std::string REDEEM_CARD_LIST_BY_STATUS_BATCH = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE status = $1 AND batch_no = $2
    ORDER BY created_at DESC
    LIMIT $3 OFFSET $4
)";

const std::string REDEEM_CARD_LIST_BY_TYPE_BATCH = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE type = $1 AND batch_no = $2
    ORDER BY created_at DESC
    LIMIT $3 OFFSET $4
)";

const std::string REDEEM_CARD_LIST_FULL = R"(
    SELECT id, code, type, value, item_id, item_name, status,
           max_uses, used_count, expiry_date, creator_id, used_by_id,
           created_at, used_at, batch_no
    FROM redeem_cards WHERE status = $1 AND type = $2 AND batch_no = $3
    ORDER BY created_at DESC
    LIMIT $4 OFFSET $5
)";
