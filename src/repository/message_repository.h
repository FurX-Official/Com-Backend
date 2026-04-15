#ifndef FURBBS_REPOSITORY_MESSAGE_REPOSITORY_H
#define FURBBS_REPOSITORY_MESSAGE_REPOSITORY_H

#include "base_repository.h"
#include "db/sql_queries.h"
#include <vector>
#include <string>

namespace furbbs::repository {

struct MessageEntity {
    int64_t id = 0;
    std::string conversation_id;
    std::string sender_id;
    std::string sender_name;
    std::string sender_avatar;
    std::string receiver_id;
    std::string receiver_name;
    std::string receiver_avatar;
    std::string content;
    int32_t message_type = 0;
    bool is_read = false;
    int64_t created_at = 0;
};

struct ConversationEntity {
    std::string id;
    std::string peer_id;
    std::string peer_name;
    std::string peer_avatar;
    std::string last_message;
    int64_t last_message_at = 0;
    int32_t unread_count = 0;
    bool is_online = false;
};

class MessageRepository : protected BaseRepository {
public:
    static MessageRepository& Instance() {
        static MessageRepository instance;
        return instance;
    }

    int64_t SendMessage(const std::string& sender_id, const std::string& receiver_id,
                         const std::string& content, int32_t message_type);

    std::vector<MessageEntity> GetConversationMessages(
        const std::string& user_id, const std::string& peer_id,
        int64_t before_time, int limit);

    std::vector<ConversationEntity> GetConversations(const std::string& user_id);

    int32_t MarkAsRead(const std::string& user_id, const std::string& peer_id);

    bool DeleteMessage(const std::string& user_id, int64_t message_id);

    int32_t GetTotalUnread(const std::string& user_id);

private:
    MessageRepository() = default;
    std::string GetConversationId(const std::string& a, const std::string& b);
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_MESSAGE_REPOSITORY_H
