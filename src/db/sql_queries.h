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

} // namespace furbbs::db::sql

#endif // FURBBS_DB_SQL_QUERIES_H
