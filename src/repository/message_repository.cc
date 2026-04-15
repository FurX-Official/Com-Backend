#include "message_repository.h"
#include <algorithm>

namespace furbbs::repository {

std::string MessageRepository::GetConversationId(const std::string& a, const std::string& b) {
    if (a < b) return a + ":" + b;
    return b + ":" + a;
}

int64_t MessageRepository::SendMessage(const std::string& sender_id, const std::string& receiver_id,
                                        const std::string& content, int32_t message_type) {
    return Execute<int64_t>([&](pqxx::work& txn) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        auto conv_id = GetConversationId(sender_id, receiver_id);

        auto result = txn.exec_params(sql::MESSAGE_SEND, conv_id, sender_id, receiver_id,
                                      content, message_type, timestamp);
        return result[0][0].as<int64_t>();
    });
}

std::vector<MessageEntity> MessageRepository::GetConversationMessages(
    const std::string& user_id, const std::string& peer_id,
    int64_t before_time, int limit) {
    return Execute<std::vector<MessageEntity>>([&](pqxx::work& txn) {
        auto conv_id = GetConversationId(user_id, peer_id);
        auto result = txn.exec_params(sql::MESSAGE_GET_HISTORY, conv_id, before_time, limit);

        std::vector<MessageEntity> list;
        for (const auto& row : result) {
            MessageEntity e;
            e.id = row[0].as<int64_t>();
            e.conversation_id = row[1].as<std::string>();
            e.sender_id = row[2].as<std::string>();
            e.receiver_id = row[3].as<std::string>();
            e.content = row[4].as<std::string>();
            e.message_type = row[5].as<int32_t>();
            e.is_read = row[6].as<bool>();
            e.created_at = row[7].as<int64_t>();
            list.push_back(e);
        }
        return list;
    });
}

std::vector<ConversationEntity> MessageRepository::GetConversations(const std::string& user_id) {
    return Execute<std::vector<ConversationEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::MESSAGE_GET_CONVERSATIONS, user_id);

        std::vector<ConversationEntity> list;
        for (const auto& row : result) {
            ConversationEntity e;
            e.id = row[0].as<std::string>();
            e.peer_id = row[1].as<std::string>();
            e.peer_name = row[2].as<std::string>();
            e.peer_avatar = row[3].as<std::string>();
            e.last_message = row[4].as<std::string>();
            e.last_message_at = row[5].as<int64_t>();
            e.unread_count = row[6].as<int32_t>();
            list.push_back(e);
        }
        return list;
    });
}

int32_t MessageRepository::MarkAsRead(const std::string& user_id, const std::string& peer_id) {
    return Execute<int32_t>([&](pqxx::work& txn) {
        auto conv_id = GetConversationId(user_id, peer_id);
        auto result = txn.exec_params(sql::MESSAGE_MARK_READ, conv_id, user_id);
        return result[0][0].as<int32_t>();
    });
}

bool MessageRepository::DeleteMessage(const std::string& user_id, int64_t message_id) {
    return Execute<bool>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::MESSAGE_DELETE, message_id, user_id);
        return result.affected_rows() > 0;
    });
}

int32_t MessageRepository::GetTotalUnread(const std::string& user_id) {
    return Execute<int32_t>([&](pqxx::work& txn) {
        auto result = txn.exec_params(sql::MESSAGE_GET_UNREAD_COUNT, user_id);
        return result[0][0].as<int32_t>();
    });
}

} // namespace furbbs::repository
