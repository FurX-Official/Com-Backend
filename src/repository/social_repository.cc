#include "social_repository.h"
#include "user_repository.h"
#include <chrono>

namespace furbbs::repository {

bool SocialRepository::SetFollow(const std::string& follower_id, 
                                   const std::string& following_id, bool follow) {
    return Execute<bool>([&](pqxx::work& txn) {
        if (follow) {
            auto result = txn.exec_params(R"(
                INSERT INTO user_follows (follower_id, following_id)
                VALUES ($1, $2) ON CONFLICT DO NOTHING
            )", follower_id, following_id);

            if (result.affected_rows() > 0) {
                txn.exec_params(R"(
                    UPDATE users SET following_count = following_count + 1 WHERE id = $1
                )", follower_id);
                txn.exec_params(R"(
                    UPDATE users SET follower_count = follower_count + 1 WHERE id = $1
                )", following_id);
                return true;
            }
        } else {
            auto result = txn.exec_params(R"(
                DELETE FROM user_follows WHERE follower_id = $1 AND following_id = $2
            )", follower_id, following_id);

            if (result.affected_rows() > 0) {
                txn.exec_params(R"(
                    UPDATE users SET following_count = following_count - 1 WHERE id = $1
                )", follower_id);
                txn.exec_params(R"(
                    UPDATE users SET follower_count = follower_count - 1 WHERE id = $1
                )", following_id);
                return true;
            }
        }
        return false;
    });
}

std::vector<FollowInfoEntity> SocialRepository::GetFollowing(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<FollowInfoEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT f.following_id, u.username, u.avatar, u.bio,
                   EXISTS(SELECT 1 FROM user_follows 
                          WHERE follower_id = f.following_id AND following_id = $1),
                   f.created_at
            FROM user_follows f
            JOIN users u ON f.following_id = u.id
            WHERE f.follower_id = $1
            ORDER BY f.created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<FollowInfoEntity> list;
        for (const auto& row : result) {
            FollowInfoEntity e;
            e.user_id = row[0].as<std::string>();
            e.username = row[1].as<std::string>();
            if (!row[2].is_null()) e.avatar = row[2].as<std::string>();
            if (!row[3].is_null()) e.bio = row[3].as<std::string>();
            e.is_mutual = row[4].as<bool>();
            e.followed_at = row[5].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

std::vector<FollowInfoEntity> SocialRepository::GetFollowers(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<FollowInfoEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT f.follower_id, u.username, u.avatar, u.bio,
                   EXISTS(SELECT 1 FROM user_follows 
                          WHERE follower_id = $1 AND following_id = f.follower_id),
                   f.created_at
            FROM user_follows f
            JOIN users u ON f.follower_id = u.id
            WHERE f.following_id = $1
            ORDER BY f.created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<FollowInfoEntity> list;
        for (const auto& row : result) {
            FollowInfoEntity e;
            e.user_id = row[0].as<std::string>();
            e.username = row[1].as<std::string>();
            if (!row[2].is_null()) e.avatar = row[2].as<std::string>();
            if (!row[3].is_null()) e.bio = row[3].as<std::string>();
            e.is_mutual = row[4].as<bool>();
            e.followed_at = row[5].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

int SocialRepository::GetFollowingCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT following_count FROM users WHERE id = $1
        )", user_id);
        return result.empty() ? 0 : result[0][0].as<int>();
    });
}

int SocialRepository::GetFollowerCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT follower_count FROM users WHERE id = $1
        )", user_id);
        return result.empty() ? 0 : result[0][0].as<int>();
    });
}

std::vector<int64_t> SocialRepository::GetFriendCirclePosts(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<int64_t>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT p.id FROM posts p
            WHERE p.author_id IN (
                SELECT following_id FROM user_follows WHERE follower_id = $1
            ) OR p.author_id = $1
            ORDER BY p.is_sticky DESC, p.sticky_weight DESC, p.created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<int64_t> ids;
        for (const auto& row : result) {
            ids.push_back(row[0].as<int64_t>());
        }
        return ids;
    });
}

bool SocialRepository::SetFursonaFavorite(const std::string& user_id, 
                                            int64_t fursona_id, bool favorite) {
    return Execute<bool>([&](pqxx::work& txn) {
        if (favorite) {
            auto result = txn.exec_params(R"(
                INSERT INTO fursona_favorites (user_id, fursona_id)
                VALUES ($1, $2) ON CONFLICT DO NOTHING
            )", user_id, fursona_id);

            if (result.affected_rows() > 0) {
                txn.exec_params(R"(
                    UPDATE fursonas SET favorite_count = favorite_count + 1 WHERE id = $1
                )", fursona_id);
                return true;
            }
        } else {
            auto result = txn.exec_params(R"(
                DELETE FROM fursona_favorites WHERE user_id = $1 AND fursona_id = $2
            )", user_id, fursona_id);

            if (result.affected_rows() > 0) {
                txn.exec_params(R"(
                    UPDATE fursonas SET favorite_count = favorite_count - 1 WHERE id = $1
                )", fursona_id);
                return true;
            }
        }
        return false;
    });
}

std::vector<int64_t> SocialRepository::GetFavoriteFursonas(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<int64_t>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT fursona_id FROM fursona_favorites
            WHERE user_id = $1
            ORDER BY created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<int64_t> ids;
        for (const auto& row : result) {
            ids.push_back(row[0].as<int64_t>());
        }
        return ids;
    });
}

int SocialRepository::GetFavoriteFursonaCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT COUNT(*) FROM fursona_favorites WHERE user_id = $1
        )", user_id);
        return result[0][0].as<int>();
    });
}

std::vector<GiftEntity> SocialRepository::GetAllGifts() {
    return Execute<std::vector<GiftEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::GIFT_GET_ALL);

        std::vector<GiftEntity> gifts;
        for (const auto& row : result) {
            GiftEntity g;
            g.id = row[0].as<int32_t>();
            g.name = row[1].as<std::string>();
            if (!row[2].is_null()) g.icon = row[2].as<std::string>();
            g.price = row[3].as<int32_t>();
            if (!row[4].is_null()) g.animation = row[4].as<std::string>();
            g.is_animated = row[5].as<bool>();
            g.rarity = row[6].as<std::string>();
            gifts.push_back(g);
        }
        return gifts;
    });
}

void SocialRepository::SendGift(const std::string& from_id, const std::string& to_id,
                                 int32_t gift_id, int32_t quantity, const std::string& message,
                                 bool is_anonymous, int32_t total_value) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(R"(
            INSERT INTO gift_history (from_user_id, to_user_id, gift_id, quantity,
                                     total_value, message, is_anonymous, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
        )", from_id, to_id, gift_id, quantity, total_value, 
           message.empty() ? nullptr : message.c_str(), is_anonymous, timestamp);

        txn.exec_params(R"(
            UPDATE users SET received_gifts_value = received_gifts_value + $1
            WHERE id = $2
        )", total_value, to_id);
    });
}

std::vector<GiftRecordEntity> SocialRepository::GetUserGifts(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<GiftRecordEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT gh.from_user_id, u.username, u.avatar,
                   gh.gift_id, gi.name, gh.quantity, gh.total_value,
                   gh.message, gh.is_anonymous, gh.created_at
            FROM gift_history gh
            JOIN gift_items gi ON gh.gift_id = gi.id
            LEFT JOIN users u ON gh.from_user_id = u.id
            WHERE gh.to_user_id = $1
            ORDER BY gh.created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<GiftRecordEntity> records;
        for (const auto& row : result) {
            GiftRecordEntity r;
            if (!row[8].as<bool>()) {
                r.from_user_id = row[0].as<std::string>();
                r.from_username = row[1].as<std::string>();
                if (!row[2].is_null()) r.from_avatar = row[2].as<std::string>();
            }
            r.gift_id = row[3].as<int32_t>();
            r.gift_name = row[4].as<std::string>();
            r.quantity = row[5].as<int32_t>();
            r.total_value = row[6].as<int32_t>();
            if (!row[7].is_null()) r.message = row[7].as<std::string>();
            r.is_anonymous = row[8].as<bool>();
            r.sent_at = row[9].as<int64_t>();
            records.push_back(r);
        }
        return records;
    });
}

int SocialRepository::GetUserGiftCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT COUNT(*) FROM gift_history WHERE to_user_id = $1
        )", user_id);
        return result[0][0].as<int>();
    });
}

int SocialRepository::GetUserGiftTotalValue(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT received_gifts_value FROM users WHERE id = $1
        )", user_id);
        return result.empty() ? 0 : result[0][0].as<int>();
    });
}

void SocialRepository::AddArtistReview(const std::string& reviewer_id, 
                                        const std::string& artist_id,
                                        int64_t commission_id, int32_t rating,
                                        const std::string& comment, 
                                        const std::vector<std::string>& tags) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        pqxx::array<std::string> tag_arr(tags);
        txn.exec_params(R"(
            INSERT INTO artist_reviews (reviewer_id, artist_id, commission_id, 
                                       rating, comment, tags, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
            ON CONFLICT (reviewer_id, commission_id) DO UPDATE SET
            rating = $4, comment = $5, tags = $6
        )", reviewer_id, artist_id, commission_id, rating,
           comment.empty() ? nullptr : comment.c_str(), tag_arr, timestamp);
    });
    UpdateArtistRating(artist_id);
}

std::vector<ArtistReviewEntity> SocialRepository::GetArtistReviews(
    const std::string& artist_id, int limit, int offset) {
    return Execute<std::vector<ArtistReviewEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT ar.reviewer_id, u.username, u.avatar,
                   ar.rating, ar.comment, ar.tags, ar.created_at
            FROM artist_reviews ar
            JOIN users u ON ar.reviewer_id = u.id
            WHERE ar.artist_id = $1
            ORDER BY ar.created_at DESC
            LIMIT $2 OFFSET $3
        )", artist_id, limit, offset);

        std::vector<ArtistReviewEntity> reviews;
        for (const auto& row : result) {
            ArtistReviewEntity r;
            r.reviewer_id = row[0].as<std::string>();
            r.reviewer_name = row[1].as<std::string>();
            if (!row[2].is_null()) r.reviewer_avatar = row[2].as<std::string>();
            r.rating = row[3].as<int32_t>();
            if (!row[4].is_null()) r.comment = row[4].as<std::string>();
            if (!row[5].is_null()) {
                pqxx::array_parser<std::vector<std::string>> parser(row[5]);
                parser.get(r.tags);
            }
            r.created_at = row[6].as<int64_t>();
            reviews.push_back(r);
        }
        return reviews;
    });
}

int SocialRepository::GetArtistReviewCount(const std::string& artist_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT COUNT(*) FROM artist_reviews WHERE artist_id = $1
        )", artist_id);
        return result[0][0].as<int>();
    });
}

double SocialRepository::GetArtistAverageRating(const std::string& artist_id) {
    return Execute<double>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT artist_rating FROM users WHERE id = $1
        )", artist_id);
        return result.empty() ? 0 : result[0][0].as<double>();
    });
}

void SocialRepository::UpdateArtistRating(const std::string& artist_id) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(R"(
            UPDATE users SET 
                artist_rating = (SELECT ROUND(AVG(rating)::numeric, 2) 
                                FROM artist_reviews WHERE artist_id = $1),
                review_count = (SELECT COUNT(*) FROM artist_reviews WHERE artist_id = $1)
            WHERE id = $1
        )", artist_id);
    });
}

void SocialRepository::ManageArtistBlacklist(const std::string& user_id, 
                                              const std::string& artist_id,
                                              bool add, const std::string& reason) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        if (add) {
            txn.exec_params(R"(
                INSERT INTO artist_blacklist (user_id, artist_id, reason, added_at)
                VALUES ($1, $2, $3, $4) ON CONFLICT DO NOTHING
            )", user_id, artist_id, reason, timestamp);
        } else {
            txn.exec_params(R"(
                DELETE FROM artist_blacklist WHERE user_id = $1 AND artist_id = $2
            )", user_id, artist_id);
        }
    });
}

std::vector<BlacklistEntryEntity> SocialRepository::GetArtistBlacklist(
    const std::string& user_id) {
    return Execute<std::vector<BlacklistEntryEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT ab.artist_id, u.username, ab.reason, ab.added_at
            FROM artist_blacklist ab
            JOIN users u ON ab.artist_id = u.id
            WHERE ab.user_id = $1
            ORDER BY ab.added_at DESC
        )", user_id);

        std::vector<BlacklistEntryEntity> list;
        for (const auto& row : result) {
            BlacklistEntryEntity e;
            e.artist_id = row[0].as<std::string>();
            e.artist_name = row[1].as<std::string>();
            if (!row[2].is_null()) e.reason = row[2].as<std::string>();
            e.added_at = row[3].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

int64_t SocialRepository::CreateQuestionBox(const std::string& owner_id, bool is_public,
                                             bool allow_anonymous, 
                                             const std::string& description) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO question_boxes (owner_id, is_public, allow_anonymous, 
                                       description, created_at)
            VALUES ($1, $2, $3, $4, $5)
            RETURNING id
        )", owner_id, is_public, allow_anonymous, description, timestamp);
        return result[0][0].as<int64_t>();
    });
}

std::vector<QuestionBoxEntity> SocialRepository::GetQuestionBoxes(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<QuestionBoxEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT qb.id, qb.owner_id, u.username, u.avatar,
                   qb.is_public, qb.allow_anonymous, qb.description,
                   qb.question_count, qb.created_at
            FROM question_boxes qb
            JOIN users u ON qb.owner_id = u.id
            WHERE qb.owner_id = $1
            ORDER BY qb.created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<QuestionBoxEntity> boxes;
        for (const auto& row : result) {
            QuestionBoxEntity b;
            b.id = row[0].as<int64_t>();
            b.owner_id = row[1].as<std::string>();
            b.owner_name = row[2].as<std::string>();
            if (!row[3].is_null()) b.owner_avatar = row[3].as<std::string>();
            b.is_public = row[4].as<bool>();
            b.allow_anonymous = row[5].as<bool>();
            if (!row[6].is_null()) b.description = row[6].as<std::string>();
            b.question_count = row[7].as<int32_t>();
            b.created_at = row[8].as<int64_t>();
            boxes.push_back(b);
        }
        return boxes;
    });
}

int SocialRepository::GetQuestionBoxCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT COUNT(*) FROM question_boxes WHERE owner_id = $1
        )", user_id);
        return result[0][0].as<int>();
    });
}

int64_t SocialRepository::AskQuestion(int64_t box_id, const std::string& asker_id,
                                        const std::string& content, bool is_anonymous) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO box_questions (box_id, asker_id, is_anonymous, content, asked_at)
            VALUES ($1, $2, $3, $4, $5)
            RETURNING id
        )", box_id, asker_id.empty() ? nullptr : asker_id.c_str(), 
           is_anonymous, content, timestamp);

        txn.exec_params(R"(
            UPDATE question_boxes SET question_count = question_count + 1 WHERE id = $1
        )", box_id);

        return result[0][0].as<int64_t>();
    });
}

void SocialRepository::AnswerQuestion(int64_t question_id, const std::string& answer, 
                                       bool is_public) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(R"(
            UPDATE box_questions SET answer = $1, is_answered = TRUE, 
                   is_public = $2, answered_at = $3
            WHERE id = $4
        )", answer, is_public, timestamp, question_id);
    });
}

std::vector<QuestionEntity> SocialRepository::GetQuestions(
    int64_t box_id, const std::string& viewer_id, bool only_unanswered,
    int limit, int offset) {
    return Execute<std::vector<QuestionEntity>>([&](pqxx::work& txn) {
        auto owner = GetQuestionBoxOwner(box_id);
        bool is_owner = owner && viewer_id == *owner;

        pqxx::result result;
        if (!is_owner) {
            const std::string& query = only_unanswered
                ? sql::QUESTION_LIST_PUBLIC_UNANSWERED
                : sql::QUESTION_LIST_PUBLIC;
            result = txn.exec_params(query, box_id, limit, offset);
        } else {
            const std::string& query = only_unanswered
                ? sql::QUESTION_LIST_UNANSWERED
                : sql::QUESTION_LIST_BASE;
            result = txn.exec_params(query, box_id, limit, offset);
        }

        std::vector<QuestionEntity> questions;
        for (const auto& row : result) {
            QuestionEntity q;
            q.id = row[0].as<int64_t>();
            bool anon = row[4].as<bool>();
            if (!anon && !row[1].is_null()) {
                q.asker_id = row[1].as<std::string>();
                q.asker_name = row[2].as<std::string>();
                if (!row[3].is_null()) q.asker_avatar = row[3].as<std::string>();
            }
            q.is_anonymous = anon;
            q.content = row[5].as<std::string>();
            if (!row[6].is_null()) q.answer = row[6].as<std::string>();
            q.is_answered = row[7].as<bool>();
            q.is_public = row[8].as<bool>();
            q.asked_at = row[9].as<int64_t>();
            if (!row[10].is_null()) q.answered_at = row[10].as<int64_t>();
            questions.push_back(q);
        }
        return questions;
    });
}

int SocialRepository::GetQuestionCount(int64_t box_id, bool only_unanswered) {
    return Execute<int>([&](pqxx::work& txn) {
        const std::string& query = only_unanswered ? sql::QUESTION_COUNT_UNANSWERED : sql::QUESTION_COUNT_BASE;
        auto result = txn.exec_params(query, box_id);
        return result[0][0].as<int>();
    });
}

std::optional<std::string> SocialRepository::GetQuestionBoxOwner(int64_t box_id) {
    return Execute<std::optional<std::string>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT owner_id FROM question_boxes WHERE id = $1
        )", box_id);
        if (result.empty()) return std::nullopt;
        return result[0][0].as<std::string>();
    });
}

} // namespace furbbs::repository
