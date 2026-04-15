#include "advanced_repository.h"
#include "post_repository.h"
#include <chrono>
#include <regex>

namespace furbbs::repository {

int64_t AdvancedRepository::CreateGroup(const std::string& owner_id, const std::string& name,
                                        const std::string& description, bool is_public,
                                        bool allow_join_request, 
                                        const std::vector<std::string>& tags) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        pqxx::array<std::string> tag_arr(tags);
        auto result = txn.exec_params(R"(
            INSERT INTO user_groups (name, description, owner_id,
                                    is_public, allow_join_request, tags, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7)
            RETURNING id
        )", name, description, owner_id, is_public, allow_join_request, tag_arr, timestamp);

        int64_t group_id = result[0][0].as<int64_t>();

        txn.exec_params(R"(
            INSERT INTO group_members (group_id, user_id, role, joined_at)
            VALUES ($1, $2, 2, $3)
        )", group_id, owner_id, timestamp);

        return group_id;
    });
}

std::vector<GroupEntity> AdvancedRepository::GetGroups(
    const std::string& user_id, const std::string& viewer_id,
    const std::string& tag, const std::string& keyword,
    int limit, int offset) {
    return Execute<std::vector<GroupEntity>>([&](pqxx::work& txn) {
        std::string sql = R"(
            SELECT g.id, g.name, g.description, g.avatar, g.banner,
                   g.owner_id, u.username, g.member_count, g.post_count,
                   g.is_public, g.allow_join_request, g.tags, g.created_at,
                   EXISTS(SELECT 1 FROM group_members 
                          WHERE group_id = g.id AND user_id = $1)
            FROM user_groups g
            JOIN users u ON g.owner_id = u.id
            WHERE 1=1
        )";
        std::vector<std::string> params;
        params.push_back(viewer_id);
        int p = 2;

        if (!user_id.empty()) {
            sql += " AND EXISTS(SELECT 1 FROM group_members "
                    "WHERE group_id = g.id AND user_id = $" + 
                    std::to_string(p++) + ")";
            params.push_back(user_id);
        }
        if (!tag.empty()) {
            sql += " AND $" + std::to_string(p++) + "=ANY(g.tags)";
            params.push_back(tag);
        }
        if (!keyword.empty()) {
            sql += " AND (g.name ILIKE $" + std::to_string(p++) + 
                    " OR g.description ILIKE $" + std::to_string(p++) + ")";
            params.push_back("%" + keyword + "%");
            params.push_back("%" + keyword + "%");
        }
        sql += " ORDER BY g.member_count DESC LIMIT $" + 
               std::to_string(p++) + " OFFSET $" + std::to_string(p++);
        params.push_back(std::to_string(limit));
        params.push_back(std::to_string(offset));

        std::string final_sql;
        for (size_t i = 0; i < params.size(); i++) {
            size_t pos = sql.find("$" + std::to_string(i + 1));
            while (pos != std::string::npos) {
                sql.replace(pos, 2 + std::to_string(i + 1).length(),
                           "'" + txn.esc(params[i]) + "'");
                pos = sql.find("$" + std::to_string(i + 1));
            }
        }

        auto result = txn.exec(sql);

        std::vector<GroupEntity> groups;
        for (const auto& row : result) {
            GroupEntity g;
            g.id = row[0].as<int64_t>();
            g.name = row[1].as<std::string>();
            if (!row[2].is_null()) g.description = row[2].as<std::string>();
            if (!row[3].is_null()) g.avatar = row[3].as<std::string>();
            g.owner_id = row[5].as<std::string>();
            g.owner_name = row[6].as<std::string>();
            g.member_count = row[7].as<int32_t>();
            g.is_public = row[9].as<bool>();
            g.created_at = row[12].as<int64_t>();
            g.is_member = row[13].as<bool>();
            groups.push_back(g);
        }
        return groups;
    });
}

int AdvancedRepository::GetGroupCount(const std::string& user_id, const std::string& tag,
                                    const std::string& keyword) {
    return Execute<int>([&](pqxx::work& txn) {
        std::string sql = "SELECT COUNT(*) FROM user_groups g WHERE 1=1";
        std::vector<std::string> params;
        int p = 1;

        if (!user_id.empty()) {
            sql += " AND EXISTS(SELECT 1 FROM group_members "
                    "WHERE group_id = g.id AND user_id = $" + 
                    std::to_string(p++) + ")";
            params.push_back(user_id);
        }

        std::string final_sql = sql;
        for (size_t i = 0; i < params.size(); i++) {
            size_t pos = sql.find("$" + std::to_string(i + 1));
            while (pos != std::string::npos) {
                sql.replace(pos, 2 + std::to_string(i + 1).length(),
                           "'" + txn.esc(params[i]) + "'");
                pos = sql.find("$" + std::to_string(i + 1));
            }
        }

        auto result = txn.exec(sql);
        return result[0][0].as<int>();
    });
}

std::optional<GroupEntity> AdvancedRepository::GetGroup(int64_t group_id, 
                                                    const std::string& viewer_id) {
    return Execute<std::optional<GroupEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT g.id, g.name, g.description, g.avatar, g.banner,
                   g.owner_id, u.username, g.member_count, g.post_count,
                   g.is_public, g.allow_join_request, g.tags, g.created_at,
                   EXISTS(SELECT 1 FROM group_members 
                          WHERE group_id = g.id AND user_id = $2),
                   COALESCE((SELECT role FROM group_members 
                              WHERE group_id = g.id AND user_id = $2, 0)
            FROM user_groups g
            JOIN users u ON g.owner_id = u.id
            WHERE g.id = $1
        )", group_id, viewer_id);

        if (result.empty()) return std::nullopt;

        GroupEntity g;
        g.id = result[0][0].as<int64_t>();
        g.name = result[0][1].as<std::string>();
        g.owner_id = result[0][5].as<std::string>();
        g.owner_name = result[0][6].as<std::string>();
        g.is_member = result[0][13].as<bool>();
        g.member_role = result[0][14].as<int32_t>();
        g.is_owner = (g.owner_id == viewer_id);
        return g;
    });
}

bool AdvancedRepository::ManageGroupMember(int64_t group_id, const std::string& caller_id,
                                          const std::string& target_user_id, int action, 
                                          int new_role) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto perm = txn.exec_params(R"(
            SELECT role FROM group_members 
            WHERE group_id = $1 AND user_id = $2
        )", group_id, caller_id);

        if (perm.empty() || perm[0][0].as<int>() < 2) {
            return false;
        }

        if (action == 0) {
            txn.exec_params(R"(
                DELETE FROM group_members 
                WHERE group_id = $1 AND user_id = $2
            )", group_id, target_user_id);
            txn.exec_params(R"(
                UPDATE user_groups SET member_count = member_count - 1 
                WHERE id = $1
            )", group_id);
        } else if (action == 1) {
            auto now = std::chrono::system_clock::now();
            auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
            txn.exec_params(R"(
                INSERT INTO group_members (group_id, user_id, role, joined_at)
                VALUES ($1, $2, $3, $4)
                ON CONFLICT DO NOTHING
            )", group_id, target_user_id, 0, ts);
            txn.exec_params(R"(
                UPDATE user_groups SET member_count = member_count + 1 
                WHERE id = $1
            )", group_id);
        } else if (action == 2) {
            txn.exec_params(R"(
                UPDATE group_members SET role = $1
                WHERE group_id = $2 AND user_id = $3
            )", new_role, group_id, target_user_id);
        }
        return true;
    });
}

int64_t AdvancedRepository::CreateGroupPost(int64_t group_id, const std::string& user_id,
                                            const std::string& title, 
                                            const std::string& content,
                                            const std::vector<int64_t>& tag_ids) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto member = txn.exec_params(R"(
            SELECT 1 FROM group_members 
            WHERE group_id = $1 AND user_id = $2
        )", group_id, user_id);

        if (member.empty()) return (int64_t)0;

        int64_t post_id = PostRepository::Instance().CreatePost(
            user_id, 0, title, content, "", 0, tag_ids);

        txn.exec_params(R"(
            INSERT INTO group_posts (group_id, post_id) VALUES ($1, $2)
        )", group_id, post_id);

        txn.exec_params(R"(
            UPDATE user_groups SET post_count = post_count + 1 
            WHERE id = $1
        )", group_id);

        return post_id;
    });
}

int64_t AdvancedRepository::RegisterEvent(int64_t event_id, const std::string& user_id,
                                          int32_t ticket_type, int32_t guest_count,
                                          const std::string& contact) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        auto result = txn.exec_params(R"(
            INSERT INTO event_registrations (event_id, user_id, ticket_type,
                                          guest_count, contact, status, registered_at)
            VALUES ($1, $2, $3, $4, $5, 0, $6)
            ON CONFLICT DO NOTHING
            RETURNING id
        )", event_id, user_id, ticket_type, guest_count, contact, timestamp);

        if (result.empty()) return (int64_t)0;

        txn.exec_params(R"(
            UPDATE events SET registration_count = registration_count + 1 
            WHERE id = $1
        )", event_id);

        return result[0][0].as<int64_t>();
    });
}

std::vector<EventRegEntity> AdvancedRepository::GetEventRegistrations(
    int64_t event_id, const std::string& owner_id, int limit, int offset) {
    return Execute<std::vector<EventRegEntity>>([&](pqxx::work& txn) {
        auto event = txn.exec_params(R"(
            SELECT creator_id FROM events WHERE id = $1
        )", event_id);

        if (event.empty() || event[0][0].as<std::string>() != owner_id) {
            return std::vector<EventRegEntity>();
        }

        auto result = txn.exec_params(R"(
            SELECT er.id, er.event_id, er.user_id, u.username, u.avatar,
                   er.ticket_type, er.guest_count, er.contact,
                   er.status, er.registered_at
            FROM event_registrations er
            JOIN users u ON er.user_id = u.id
            WHERE er.event_id = $1
            ORDER BY er.registered_at DESC
            LIMIT $2 OFFSET $3
        )", event_id, limit, offset);

        std::vector<EventRegEntity> regs;
        for (const auto& row : result) {
            EventRegEntity e;
            e.id = row[0].as<int64_t>();
            e.user_id = row[2].as<std::string>();
            e.username = row[3].as<std::string>();
            e.status = row[8].as<int32_t>();
            regs.push_back(e);
        }
        return regs;
    });
}

int AdvancedRepository::GetRegistrationCount(int64_t event_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM event_registrations WHERE event_id = $1
        )", event_id);
        return r[0][0].as<int>();
    });
}

int AdvancedRepository::GetConfirmedCount(int64_t event_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM event_registrations 
            WHERE event_id = $1 AND status = 1
        )", event_id);
        return r[0][0].as<int>();
    });
}

bool AdvancedRepository::UpdateRegistrationStatus(int64_t reg_id, 
                                              const std::string& owner_id, 
                                              int32_t new_status) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT e.creator_id FROM event_registrations er
            JOIN events e ON er.event_id = e.id
            WHERE er.id = $1
        )", reg_id);

        if (r.empty() || r[0][0].as<std::string>() != owner_id) {
            return false;
        }

        txn.exec_params(R"(
            UPDATE event_registrations SET status = $1 WHERE id = $2
        )", new_status, reg_id);
        return true;
    });
}

void AdvancedRepository::CreateMention(const std::string& from_id,
                                        const std::vector<std::string>& to_ids,
                                        int64_t post_id, int64_t comment_id,
                                        const std::string& preview) {
    Execute([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        for (const auto& to_id : to_ids) {
            if (to_id == from_id) continue;
            txn.exec_params(R"(
                INSERT INTO mentions (from_user_id, to_user_id, post_id,
                                     comment_id, content_preview, created_at)
                VALUES ($1, $2, $3, $4, $5, $6)
            )", from_id, to_id, post_id, 
               comment_id > 0 ? comment_id : nullptr, preview, timestamp);
        }
    });
}

std::vector<MentionEntity> AdvancedRepository::GetUserMentions(
    const std::string& user_id, bool only_unread, int limit, int offset) {
    return Execute<std::vector<MentionEntity>>([&](pqxx::work& txn) {
        std::string sql = R"(
            SELECT m.id, m.from_user_id, u.username, u.avatar,
                   m.post_id, p.title, m.comment_id, m.content_preview,
                   m.is_read, m.created_at
            FROM mentions m
            JOIN users u ON m.from_user_id = u.id
            JOIN posts p ON m.post_id = p.id
            WHERE m.to_user_id = $1
        )";
        if (only_unread) sql += " AND m.is_read = FALSE";
        sql += " ORDER BY m.created_at DESC LIMIT $2 OFFSET $3";

        auto result = txn.exec_params(sql, user_id, limit, offset);

        std::vector<MentionEntity> mentions;
        for (const auto& row : result) {
            MentionEntity m;
            m.id = row[0].as<int64_t>();
            m.from_user_id = row[1].as<std::string>();
            m.from_username = row[2].as<std::string>();
            m.post_id = row[4].as<int64_t>();
            m.post_title = row[5].as<std::string>();
            m.is_read = row[8].as<bool>();
            mentions.push_back(m);
        }
        return mentions;
    });
}

int AdvancedRepository::GetMentionCount(const std::string& user_id, bool only_unread) {
    return Execute<int>([&](pqxx::work& txn) {
        std::string sql = R"(
            SELECT COUNT(*) FROM mentions WHERE to_user_id = $1
        )";
        if (only_unread) sql += " AND is_read = FALSE";
        auto r = txn.exec_params(sql, user_id);
        return r[0][0].as<int>();
    });
}

int AdvancedRepository::GetUnreadMentionCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM mentions 
            WHERE to_user_id = $1 AND is_read = FALSE
        )", user_id);
        return r[0][0].as<int>();
    });
}

void AdvancedRepository::MarkMentionRead(const std::string& user_id,
                                           int64_t mention_id, bool mark_all) {
    Execute([&](pqxx::work& txn) {
        if (mark_all) {
            txn.exec_params(R"(
                UPDATE mentions SET is_read = TRUE WHERE to_user_id = $1
            )", user_id);
        } else {
            txn.exec_params(R"(
                UPDATE mentions SET is_read = TRUE 
                WHERE id = $1 AND to_user_id = $2
            )", mention_id, user_id);
        }
    });
}

bool AdvancedRepository::SetPostFavorite(const std::string& user_id,
                                          int64_t post_id, bool favorite) {
    return Execute<bool>([&](pqxx::work& txn) {
        if (favorite) {
            auto r = txn.exec_params(R"(
                INSERT INTO post_favorites (user_id, post_id)
                VALUES ($1, $2) ON CONFLICT DO NOTHING
            )", user_id, post_id);
            return r.affected_rows() > 0;
        } else {
            auto r = txn.exec_params(R"(
                DELETE FROM post_favorites 
                WHERE user_id = $1 AND post_id = $2
            )", user_id, post_id);
            return r.affected_rows() > 0;
        }
    });
}

std::vector<int64_t> AdvancedRepository::GetFavoritePostIds(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<int64_t>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT post_id FROM post_favorites
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

int AdvancedRepository::GetFavoritePostCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM post_favorites WHERE user_id = $1
        )", user_id);
        return r[0][0].as<int>();
    });
}

int64_t AdvancedRepository::SaveDraft(const std::string& user_id, int64_t draft_id,
                                      const std::string& title, const std::string& content,
                                      int32_t section_id, 
                                      const std::vector<int64_t>& tag_ids,
                                      int64_t group_id) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        pqxx::array<int64_t> tag_arr(tag_ids);

        if (draft_id > 0) {
            txn.exec_params(R"(
                UPDATE drafts SET title = $1, content = $2, section_id = $3,
                       tag_ids = $4, group_id = $5, updated_at = $6
                WHERE id = $7 AND user_id = $8
            )", title, content, section_id > 0 ? section_id : nullptr,
               tag_ids.empty() ? nullptr : &tag_arr,
               group_id > 0 ? group_id : nullptr,
               timestamp, draft_id, user_id);
            return draft_id;
        } else {
            auto result = txn.exec_params(R"(
                INSERT INTO drafts (user_id, title, content, section_id,
                                   tag_ids, group_id, created_at, updated_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                RETURNING id
            )", user_id, title, content,
               section_id > 0 ? section_id : nullptr,
               tag_ids.empty() ? nullptr : &tag_arr,
               group_id > 0 ? group_id : nullptr,
               timestamp, timestamp);
            return result[0][0].as<int64_t>();
        }
    });
}

std::vector<DraftEntity> AdvancedRepository::GetUserDrafts(
    const std::string& user_id, int limit, int offset) {
    return Execute<std::vector<DraftEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(R"(
            SELECT id, title, content, section_id, tag_ids,
                   group_id, created_at, updated_at
            FROM drafts
            WHERE user_id = $1
            ORDER BY updated_at DESC
            LIMIT $2 OFFSET $3
        )", user_id, limit, offset);

        std::vector<DraftEntity> drafts;
        for (const auto& row : result) {
            DraftEntity d;
            d.id = row[0].as<int64_t>();
            d.title = row[1].as<std::string>();
            d.content = row[2].as<std::string>();
            d.created_at = row[6].as<int64_t>();
            d.updated_at = row[7].as<int64_t>();
            drafts.push_back(d);
        }
        return drafts;
    });
}

int AdvancedRepository::GetDraftCount(const std::string& user_id) {
    return Execute<int>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            SELECT COUNT(*) FROM drafts WHERE user_id = $1
        )", user_id);
        return r[0][0].as<int>();
    });
}

bool AdvancedRepository::DeleteDraft(const std::string& user_id, int64_t draft_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto r = txn.exec_params(R"(
            DELETE FROM drafts WHERE id = $1 AND user_id = $2
        )", draft_id, user_id);
        return r.affected_rows() > 0;
    });
}

std::vector<std::string> AdvancedRepository::ExtractMentions(const std::string& content) {
    std::vector<std::string> usernames;
    std::regex re(R"(@(\w+))");
    auto begin = std::sregex_iterator(content.begin(), content.end(), re);
    auto end = std::sregex_iterator();

    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        usernames.push_back(match[1].str());
    }
    return usernames;
}

} // namespace furbbs::repository
