#include "message_service.h"
#include "service_impl/user_service.h"

namespace furbbs::service {

int64_t MessageService::SendMessage(const std::string& token, const std::string& receiver_id,
                                     const std::string& content, int32_t message_type) {
    auto sender_id = auth::CasdoorAuth::ValidateToken(token);
    if (!sender_id || receiver_id.empty() || content.empty()) return 0;

    auto msg_id = repository::MessageRepository::Instance().SendMessage(
        *sender_id, receiver_id, content, message_type);

    if (msg_id > 0) {
        UserService::Instance().AddActivityPoints(*sender_id, 1);
    }

    return msg_id;
}

std::vector<repository::MessageEntity> MessageService::GetConversationMessages(
    const std::string& token, const std::string& peer_id,
    int64_t before_time, int limit) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return {};

    if (before_time == 0) {
        auto now = std::chrono::system_clock::now();
        before_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
    }

    return repository::MessageRepository::Instance().GetConversationMessages(
        *user_id, peer_id, before_time, limit > 0 ? limit : 50);
}

std::vector<repository::ConversationEntity> MessageService::GetConversations(const std::string& token) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return {};

    return repository::MessageRepository::Instance().GetConversations(*user_id);
}

int32_t MessageService::MarkAsRead(const std::string& token, const std::string& peer_id) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return 0;

    return repository::MessageRepository::Instance().MarkAsRead(*user_id, peer_id);
}

bool MessageService::DeleteMessage(const std::string& token, int64_t message_id) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return false;

    return repository::MessageRepository::Instance().DeleteMessage(*user_id, message_id);
}

int32_t MessageService::GetTotalUnread(const std::string& token) {
    auto user_id = auth::CasdoorAuth::ValidateToken(token);
    if (!user_id) return 0;

    return repository::MessageRepository::Instance().GetTotalUnread(*user_id);
}

} // namespace furbbs::service
