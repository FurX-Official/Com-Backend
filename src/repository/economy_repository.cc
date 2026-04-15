#include "economy_repository.h"
#include <chrono>
#include <random>
#include <algorithm>

namespace furbbs::repository {

int64_t EconomyRepository::CreatePaidContent(const std::string& author_id,
                                              int64_t post_id, int32_t price,
                                              const std::string& preview,
                                              const std::string& full) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO paid_content (post_id, author_id, price,
                                     preview_content, full_content, created_at)
            VALUES ($1, $2, $3, $4, $5, $6)
            RETURNING id
        )", post_id, author_id, price, preview, full, timestamp);

        return result[0][0].as<int64_t>();
    });
}

std::vector<PaidContentEntity> EconomyRepository::GetUserPaidContents(
    const std::string& user_id, const std::string& viewer_id,
    int limit, int offset) {
    return Execute<std::vector<PaidContentEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT pc.id, pc.post_id, pc.author_id, pc.price,
                   pc.preview_content, pc.purchase_count, pc.revenue, pc.created_at,
                   EXISTS(SELECT 1 FROM content_purchases cp
                          WHERE cp.content_id = pc.id AND cp.buyer_id = $2)
            FROM paid_content pc
            WHERE pc.author_id = $1
            ORDER BY pc.created_at DESC
            LIMIT $3 OFFSET $4
        )", user_id, viewer_id, limit, offset);

        std::vector<PaidContentEntity> list;
        for (const auto& row : result) {
            PaidContentEntity e;
            e.id = row[0].as<int64_t>();
            e.post_id = row[1].as<int64_t>();
            e.price = row[3].as<int32_t>();
            e.preview_content = row[4].as<std::string>();
            e.purchase_count = row[5].as<int32_t>();
            e.purchased = row[8].as<bool>();
            if (e.purchased || user_id == viewer_id) {
                auto full = txn.exec_params(R"(
                    SELECT full_content FROM paid_content WHERE id = $1
                )", e.id);
                if (!full.empty()) {
                    e.full_content = full[0][0].as<std::string>();
                }
            }
            list.push_back(e);
        }
        return list;
    });
}

int EconomyRepository::GetPaidContentCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM paid_content WHERE author_id = $1
        )", user_id);
        return r[0][0].as<int>();
    });
}

std::optional<PaidContentEntity> EconomyRepository::GetPaidContent(
    int64_t content_id, const std::string& viewer_id) {
    return Execute<std::optional<PaidContentEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT pc.id, pc.post_id, pc.author_id, pc.price,
                   pc.preview_content, pc.full_content, pc.purchase_count,
                   EXISTS(SELECT 1 FROM content_purchases cp
                          WHERE cp.content_id = pc.id AND cp.buyer_id = $2)
            FROM paid_content pc
            WHERE pc.id = $1
        )", content_id, viewer_id);

        if (result.empty()) return std::nullopt;

        PaidContentEntity e;
        e.id = result[0][0].as<int64_t>();
        e.author_id = result[0][2].as<std::string>();
        e.price = result[0][3].as<int32_t>();
        e.preview_content = result[0][4].as<std::string>();
        e.purchase_count = result[0][6].as<int32_t>();
        e.purchased = result[0][7].as<bool>();
        if (e.purchased || e.author_id == viewer_id) {
            e.full_content = result[0][5].as<std::string>();
        }
        return e;
    });
}

bool EconomyRepository::PurchaseContent(const std::string& buyer_id,
                                         int64_t content_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto content = txn.exec_params(R"(
            SELECT price, author_id FROM paid_content WHERE id = $1
        )", content_id);
        if (content.empty()) return false;

        int32_t price = content[0][0].as<int32_t>();
        std::string author_id = content[0][1].as<std::string>();

        auto buyer = txn.exec_params(R"(
            SELECT points FROM users WHERE id = $1
        )", buyer_id);
        if (buyer.empty() || buyer[0][0].as<int32_t>() < price) {
            return false;
        }

        txn.exec_params(R"(
            UPDATE users SET points = points - $1 WHERE id = $2
        )", price, buyer_id);

        txn.exec_params(R"(
            UPDATE users SET points = points + $1 WHERE id = $2
        )", price, author_id);

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(R"(
            INSERT INTO content_purchases (content_id, buyer_id, price_paid, purchased_at)
            VALUES ($1, $2, $3, $4)
            ON CONFLICT DO NOTHING
        )", content_id, buyer_id, price, timestamp);

        txn.exec_params(R"(
            UPDATE paid_content SET purchase_count = purchase_count + 1,
                                    revenue = revenue + $1
            WHERE id = $2
        )", price, content_id);

        return true;
    });
}

int64_t EconomyRepository::CreateGallery(const std::string& user_id,
                                          const std::string& name,
                                          const std::string& description,
                                          const std::string& cover,
                                          bool is_public, bool is_nsfw) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO galleries (user_id, name, description, cover_image,
                                   is_public, is_nsfw, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
            RETURNING id
        )", user_id, name, description, cover.empty() ? nullptr : &cover,
           is_public, is_nsfw, timestamp);

        return result[0][0].as<int64_t>();
    });
}

bool EconomyRepository::UpdateGallery(const std::string& user_id,
                                      int64_t gallery_id,
                                      const GalleryEntity& data) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            UPDATE galleries SET name = $1, description = $2,
                                 cover_image = $3, is_public = $4, is_nsfw = $5
            WHERE id = $6 AND user_id = $7
        )", data.name, data.description,
           data.cover_image.empty() ? nullptr : &data.cover_image,
           data.is_public, data.is_nsfw, gallery_id, user_id);
        return r.affected_rows() > 0;
    });
}

bool EconomyRepository::DeleteGallery(const std::string& user_id,
                                      int64_t gallery_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            DELETE FROM galleries WHERE id = $1 AND user_id = $2
        )", gallery_id, user_id);
        return r.affected_rows() > 0;
    });
}

std::vector<GalleryEntity> EconomyRepository::GetGalleries(
    const std::string& user_id, const std::string& viewer_id,
    bool only_public, int limit, int offset) {
    return Execute<std::vector<GalleryEntity>>([&](pqxx::work& txn) {
        std::string sql = R"(
            SELECT g.id, g.name, g.description, g.cover_image,
                   g.is_public, g.is_nsfw, g.item_count, g.view_count,
                   g.like_count, g.created_at, u.username, u.avatar,
                   EXISTS(SELECT 1 FROM gallery_favorites gf
                          WHERE gf.gallery_id = g.id AND gf.user_id = $2)
            FROM galleries g
            JOIN users u ON g.user_id = u.id
            WHERE g.user_id = $1
        )";
        std::string final_sql;
        if (only_public && user_id != viewer_id) {
            sql += " AND g.is_public = TRUE";
        }
        sql += " ORDER BY g.created_at DESC LIMIT $" + std::to_string(3) +
               " OFFSET $" + std::to_string(4);

        final_sql = sql;
        final_sql.replace(final_sql.find("$1"), 2, "'" + txn.esc(user_id) + "'");
        final_sql.replace(final_sql.find("$2"), 2, "'" + txn.esc(viewer_id) + "'");
        final_sql.replace(final_sql.find("$3"), 2, std::to_string(limit));
        final_sql.replace(final_sql.find("$4"), 2, std::to_string(offset));

        auto result = txn.exec(final_sql);

        std::vector<GalleryEntity> list;
        for (const auto& row : result) {
            GalleryEntity e;
            e.id = row[0].as<int64_t>();
            e.name = row[1].as<std::string>();
            e.is_public = row[4].as<bool>();
            e.item_count = row[6].as<int32_t>();
            e.username = row[10].as<std::string>();
            e.is_favorited = row[12].as<bool>();
            list.push_back(e);
        }
        return list;
    });
}

int EconomyRepository::GetGalleryCount(const std::string& user_id, bool only_public) {
    return Execute<int>([&](pqxx::work& txn) {
        std::string sql = "SELECT COUNT(*) FROM galleries WHERE user_id = $1";
        if (only_public) sql += " AND is_public = TRUE";
        auto r = txn.exec_params(sql, user_id);
        return r[0][0].as<int>();
    });
}

std::optional<GalleryEntity> EconomyRepository::GetGallery(
    int64_t gallery_id, const std::string& viewer_id) {
    return Execute<std::optional<GalleryEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT g.id, g.user_id, g.name, g.description, g.cover_image,
                   g.is_public, g.is_nsfw, g.item_count, u.username, u.avatar
            FROM galleries g
            JOIN users u ON g.user_id = u.id
            WHERE g.id = $1
        )", gallery_id);

        if (result.empty()) return std::nullopt;

        const auto& row = result[0];
        std::string owner_id = row[1].as<std::string>();
        bool is_public = row[5].as<bool>();

        if (!is_public && owner_id != viewer_id) {
            return std::nullopt;
        }

        txn.exec_params(R"(
            UPDATE galleries SET view_count = view_count + 1 WHERE id = $1
        )", gallery_id);

        GalleryEntity e;
        e.id = row[0].as<int64_t>();
        e.user_id = owner_id;
        e.name = row[2].as<std::string>();
        e.username = row[8].as<std::string>();
        return e;
    });
}

bool EconomyRepository::SetGalleryFavorite(const std::string& user_id,
                                            int64_t gallery_id, bool favorite) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        if (favorite) {
            auto r = txn.exec_params(R"(
                INSERT INTO gallery_favorites (gallery_id, user_id, created_at)
                VALUES ($1, $2, $3) ON CONFLICT DO NOTHING
            )", gallery_id, user_id, timestamp);
            if (r.affected_rows() > 0) {
                txn.exec_params(R"(
                    UPDATE galleries SET like_count = like_count + 1 WHERE id = $1
                )", gallery_id);
                return true;
            }
        } else {
            auto r = txn.exec_params(R"(
                DELETE FROM gallery_favorites WHERE gallery_id = $1 AND user_id = $2
            )", gallery_id, user_id);
            if (r.affected_rows() > 0) {
                txn.exec_params(R"(
                    UPDATE galleries SET like_count = like_count - 1 WHERE id = $1
                )", gallery_id);
                return true;
            }
        }
        return false;
    });
}

int64_t EconomyRepository::AddGalleryItem(int64_t gallery_id,
                                           const std::string& user_id,
                                           const GalleryItemEntity& item) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto perm = txn.exec_params(R"(
            SELECT 1 FROM galleries WHERE id = $1 AND user_id = $2
        )", gallery_id, user_id);
        if (perm.empty()) return (int64_t)0;

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO gallery_items (gallery_id, title, description,
                                      image_url, thumbnail_url, fursona_id,
                                      artist_name, is_nsfw, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
            RETURNING id
        )", gallery_id, item.title, item.description,
           item.image_url, item.thumbnail_url.empty() ? nullptr : &item.thumbnail_url,
           item.fursona_id > 0 ? &item.fursona_id : nullptr,
           item.artist_name.empty() ? nullptr : &item.artist_name,
           item.is_nsfw, timestamp);

        txn.exec_params(R"(
            UPDATE galleries SET item_count = item_count + 1 WHERE id = $1
        )", gallery_id);

        return result[0][0].as<int64_t>();
    });
}

bool EconomyRepository::RemoveGalleryItem(int64_t gallery_id,
                                          const std::string& user_id,
                                          int64_t item_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto perm = txn.exec_params(R"(
            SELECT 1 FROM galleries WHERE id = $1 AND user_id = $2
        )", gallery_id, user_id);
        if (perm.empty()) return false;

        auto r = txn.exec_params(R"(
            DELETE FROM gallery_items WHERE id = $1 AND gallery_id = $2
        )", item_id, gallery_id);

        if (r.affected_rows() > 0) {
            txn.exec_params(R"(
                UPDATE galleries SET item_count = item_count - 1 WHERE id = $1
            )", gallery_id);
            return true;
        }
        return false;
    });
}

std::vector<GalleryItemEntity> EconomyRepository::GetGalleryItems(
    int64_t gallery_id, const std::string& viewer_id, int limit, int offset) {
    return Execute<std::vector<GalleryItemEntity>>([&](pqxx::work& txn) {
        auto g = txn.exec_params(R"(
            SELECT user_id, is_public, is_nsfw FROM galleries WHERE id = $1
        )", gallery_id);
        if (g.empty()) return {};

        std::string owner_id = g[0][0].as<std::string>();
        bool is_public = g[0][1].as<bool>();
        if (!is_public && owner_id != viewer_id) {
            return {};
        }

        auto result = txn.exec_params(R"(
            SELECT id, title, image_url, thumbnail_url,
                   artist_name, created_at
            FROM gallery_items
            WHERE gallery_id = $1
            ORDER BY created_at DESC
            LIMIT $2 OFFSET $3
        )", gallery_id, limit, offset);

        std::vector<GalleryItemEntity> list;
        for (const auto& row : result) {
            GalleryItemEntity e;
            e.id = row[0].as<int64_t>();
            e.title = row[1].as<std::string>();
            e.image_url = row[2].as<std::string>();
            list.push_back(e);
        }
        return list;
    });
}

int EconomyRepository::GetGalleryItemCount(int64_t gallery_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM gallery_items WHERE gallery_id = $1
        )", gallery_id);
        return r[0][0].as<int>();
    });
}

bool EconomyRepository::TransferPoints(const std::string& from_id,
                                        const std::string& to_id,
                                        int32_t amount,
                                        const std::string& message) {
    return Execute<bool>([&](pqxx::work& txn) {
        if (from_id == to_id || amount <= 0) return false;

        auto from = txn.exec_params(R"(
            SELECT points FROM users WHERE id = $1
        )", from_id);
        if (from.empty() || from[0][0].as<int32_t>() < amount) {
            return false;
        }

        txn.exec_params(R"(
            UPDATE users SET points = points - $1 WHERE id = $2
        )", amount, from_id);

        txn.exec_params(R"(
            UPDATE users SET points = points + $1 WHERE id = $2
        )", amount, to_id);

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(R"(
            INSERT INTO point_transfers (from_user_id, to_user_id,
                                        amount, message, created_at)
            VALUES ($1, $2, $3, $4, $5)
        )", from_id, to_id, amount, message, timestamp);

        return true;
    });
}

std::vector<TransferEntity> EconomyRepository::GetTransferHistory(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<TransferEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT t.id, t.from_user_id, t.to_user_id, t.amount,
                   t.message, t.created_at,
                   uf.username, uf.avatar, ut.username, ut.avatar
            FROM point_transfers t
            JOIN users uf ON t.from_user_id = uf.id
            JOIN users ut ON t.to_user_id = ut.id
            WHERE t.from_user_id = $1 OR t.to_user_id = $1
            ORDER BY t.created_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<TransferEntity> list;
        for (const auto& row : result) {
            TransferEntity e;
            e.id = row[0].as<int64_t>();
            e.from_user_id = row[1].as<std::string>();
            e.to_user_id = row[2].as<std::string>();
            e.amount = row[3].as<int32_t>();
            e.from_username = row[6].as<std::string>();
            e.to_username = row[8].as<std::string>();
            list.push_back(e);
        }
        return list;
    });
}

int64_t EconomyRepository::CreateRedEnvelope(const std::string& sender_id,
                                              int32_t total_amount,
                                              int32_t count,
                                              const std::string& message,
                                              bool is_random) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        if (total_amount < count || count <= 0) return (int64_t)0;

        auto sender = txn.exec_params(R"(
            SELECT points FROM users WHERE id = $1
        )", sender_id);
        if (sender.empty() || sender[0][0].as<int32_t>() < total_amount) {
            return (int64_t)0;
        }

        txn.exec_params(R"(
            UPDATE users SET points = points - $1 WHERE id = $2
        )", total_amount, sender_id);

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        auto expires = timestamp + 24 * 60 * 60 * 1000;

        auto result = txn.exec_params(R"(
            INSERT INTO red_envelopes (sender_id, total_amount, count,
                                      remaining_amount, remaining_count,
                                      message, is_random, expires_at, created_at)
            VALUES ($1, $2, $3, $2, $3, $4, $5, $6, $7)
            RETURNING id
        )", sender_id, total_amount, count, message,
           is_random, expires, timestamp);

        return result[0][0].as<int64_t>();
    });
}

int32_t EconomyRepository::ClaimRedEnvelope(int64_t envelope_id,
                                             const std::string& user_id) {
    return Execute<int32_t>([&](pqxx::work& txn) {
        auto env = txn.exec_params(R"(
            SELECT remaining_amount, remaining_count, is_random,
                   sender_id, expires_at
            FROM red_envelopes WHERE id = $1
        )", envelope_id);
        if (env.empty()) return 0;

        int32_t remaining = env[0][0].as<int32_t>();
        int32_t remain_cnt = env[0][1].as<int32_t>();
        bool is_random = env[0][2].as<bool>();
        std::string sender_id = env[0][3].as<std::string>();
        int64_t expires = env[0][4].as<int64_t>();

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        if (remain_cnt <= 0 || remaining <= 0 || timestamp > expires) {
            return 0;
        }

        auto claimed = txn.exec_params(R"(
            SELECT 1 FROM red_envelope_claims
            WHERE envelope_id = $1 AND user_id = $2
        )", envelope_id, user_id);
        if (!claimed.empty()) return 0;

        int32_t amount = 1;
        if (is_random && remain_cnt > 1) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(1, remaining - (remain_cnt - 1));
            amount = dist(gen);
        } else if (!is_random) {
            amount = remaining / remain_cnt;
        }

        txn.exec_params(R"(
            UPDATE users SET points = points + $1 WHERE id = $2
        )", amount, user_id);

        txn.exec_params(R"(
            INSERT INTO red_envelope_claims (envelope_id, user_id, amount, claimed_at)
            VALUES ($1, $2, $3, $4)
        )", envelope_id, user_id, amount, timestamp);

        txn.exec_params(R"(
            UPDATE red_envelopes SET
                remaining_amount = remaining_amount - $1,
                remaining_count = remaining_count - 1
            WHERE id = $2
        )", amount, envelope_id);

        return amount;
    });
}

std::vector<RedEnvelopeEntity> EconomyRepository::GetActiveRedEnvelopes(
    int limit, int offset) {
    return Execute<std::vector<RedEnvelopeEntity>>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            SELECT e.id, e.sender_id, e.total_amount, e.count,
                   e.remaining_amount, e.remaining_count, e.message,
                   e.is_random, e.created_at, u.username, u.avatar
            FROM red_envelopes e
            JOIN users u ON e.sender_id = u.id
            WHERE e.remaining_count > 0 AND e.expires_at > $1
            ORDER BY e.created_at DESC
            LIMIT $2 OFFSET $3
        )", timestamp, limit, offset);

        std::vector<RedEnvelopeEntity> list;
        for (const auto& row : result) {
            RedEnvelopeEntity e;
            e.id = row[0].as<int64_t>();
            e.sender_id = row[1].as<std::string>();
            e.total_amount = row[2].as<int32_t>();
            e.sender_name = row[9].as<std::string>();
            list.push_back(e);
        }
        return list;
    });
}

bool EconomyRepository::RewardPost(const std::string& sender_id, int64_t post_id,
                                    int32_t amount, const std::string& message,
                                    bool anonymous) {
    return Execute<bool>([&](pqxx::work& txn) {
        if (amount <= 0) return false;

        auto post = txn.exec_params(R"(
            SELECT author_id FROM posts WHERE id = $1
        )", post_id);
        if (post.empty()) return false;

        std::string receiver_id = post[0][0].as<std::string>();

        auto sender = txn.exec_params(R"(
            SELECT points FROM users WHERE id = $1
        )", sender_id);
        if (sender.empty() || sender[0][0].as<int32_t>() < amount) {
            return false;
        }

        txn.exec_params(R"(
            UPDATE users SET points = points - $1 WHERE id = $2
        )", amount, sender_id);

        txn.exec_params(R"(
            UPDATE users SET points = points + $1 WHERE id = $2
        )", amount, receiver_id);

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        txn.exec_params(R"(
            INSERT INTO post_rewards (post_id, sender_id, receiver_id,
                                     amount, message, anonymous, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
        )", post_id, sender_id, receiver_id, amount, message, anonymous, timestamp);

        return true;
    });
}

std::vector<PostRewardEntity> EconomyRepository::GetPostRewards(
    int64_t post_id, int limit, int offset) {
    return Execute<std::vector<PostRewardEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT r.id, r.sender_id, r.amount, r.message,
                   r.anonymous, r.created_at, u.username, u.avatar
            FROM post_rewards r
            LEFT JOIN users u ON r.sender_id = u.id
            WHERE r.post_id = $1
            ORDER BY r.created_at DESC
            LIMIT $2 OFFSET $3
        )", post_id, limit, offset);

        std::vector<PostRewardEntity> list;
        for (const auto& row : result) {
            PostRewardEntity e;
            e.id = row[0].as<int64_t>();
            e.amount = row[2].as<int32_t>();
            if (!row[4].as<bool>()) {
                e.sender_id = row[1].as<std::string>();
                e.sender_name = row[6].as<std::string>();
            }
            list.push_back(e);
        }
        return list;
    });
}

int64_t EconomyRepository::CreateCollection(const std::string& user_id,
                                             const std::string& name,
                                             const std::string& description,
                                             const std::string& cover,
                                             bool is_public) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO collections (user_id, name, description,
                                    cover_image, is_public, created_at)
            VALUES ($1, $2, $3, $4, $5, $6)
            RETURNING id
        )", user_id, name, description, cover.empty() ? nullptr : &cover,
           is_public, timestamp);

        return result[0][0].as<int64_t>();
    });
}

bool EconomyRepository::DeleteCollection(const std::string& user_id,
                                          int64_t collection_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            DELETE FROM collections WHERE id = $1 AND user_id = $2
        )", collection_id, user_id);
        return r.affected_rows() > 0;
    });
}

bool EconomyRepository::AddToCollection(const std::string& user_id,
                                         int64_t collection_id,
                                         int64_t post_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto perm = txn.exec_params(R"(
            SELECT 1 FROM collections WHERE id = $1 AND user_id = $2
        )", collection_id, user_id);
        if (perm.empty()) return false;

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto r = txn.exec_params(R"(
            INSERT INTO collection_items (collection_id, post_id, added_at)
            VALUES ($1, $2, $3) ON CONFLICT DO NOTHING
        )", collection_id, post_id, timestamp);

        if (r.affected_rows() > 0) {
            txn.exec_params(R"(
                UPDATE collections SET item_count = item_count + 1 WHERE id = $1
            )", collection_id);
            return true;
        }
        return false;
    });
}

bool EconomyRepository::RemoveFromCollection(const std::string& user_id,
                                              int64_t collection_id,
                                              int64_t post_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto perm = txn.exec_params(R"(
            SELECT 1 FROM collections WHERE id = $1 AND user_id = $2
        )", collection_id, user_id);
        if (perm.empty()) return false;

        auto r = txn.exec_params(R"(
            DELETE FROM collection_items
            WHERE collection_id = $1 AND post_id = $2
        )", collection_id, post_id);

        if (r.affected_rows() > 0) {
            txn.exec_params(R"(
                UPDATE collections SET item_count = item_count - 1 WHERE id = $1
            )", collection_id);
            return true;
        }
        return false;
    });
}

std::vector<CollectionEntity> EconomyRepository::GetUserCollections(
    const std::string& user_id, const std::string& viewer_id,
    int limit, int offset) {
    return Execute<std::vector<CollectionEntity>>([&](pqxx::work& txn) {
        std::string sql = R"(
            SELECT id, name, description, cover_image,
                   is_public, item_count, created_at
            FROM collections
            WHERE user_id = $1
        )";
        if (user_id != viewer_id) {
            sql += " AND is_public = TRUE";
        }
        sql += " ORDER BY created_at DESC LIMIT $2 OFFSET $3";

        auto result = txn.exec_params(sql, user_id, limit, offset);

        std::vector<CollectionEntity> list;
        for (const auto& row : result) {
            CollectionEntity e;
            e.id = row[0].as<int64_t>();
            e.name = row[1].as<std::string>();
            e.item_count = row[5].as<int32_t>();
            list.push_back(e);
        }
        return list;
    });
}

std::vector<int64_t> EconomyRepository::GetCollectionPosts(
    int64_t collection_id, const std::string& viewer_id,
    int limit, int offset) {
    return Execute<std::vector<int64_t>>([&](pqxx::work& txn) {
        auto c = txn.exec_params(R"(
            SELECT user_id, is_public FROM collections WHERE id = $1
        )", collection_id);
        if (c.empty()) return {};

        std::string owner_id = c[0][0].as<std::string>();
        bool is_public = c[0][1].as<bool>();
        if (!is_public && owner_id != viewer_id) {
            return {};
        }

        auto result = txn.exec_params(R"(
            SELECT post_id FROM collection_items
            WHERE collection_id = $1
            ORDER BY added_at DESC
            LIMIT $2 OFFSET $3
        )", collection_id, limit, offset);

        std::vector<int64_t> ids;
        for (const auto& row : result) {
            ids.push_back(row[0].as<int64_t>());
        }
        return ids;
    });
}

} // namespace furbbs::repository
