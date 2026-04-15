#ifndef FURBBS_SERVICE_IMPL_MESSAGE_SERVICE_H
#define FURBBS_SERVICE_IMPL_MESSAGE_SERVICE_H

#include "repository/message_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class MessageService {
public:
    static MessageService& Instance() {
        static MessageService instance;
        return instance;
    }

    int64_t SendMessage(const std::string& token, const std::string& receiver_id,
                        const std::string& content, int32_t message_type);

    std::vector<repository::MessageEntity> GetConversationMessages(
        const std::string& token, const std::string& peer_id,
        int64_t before_time, int limit);

    std::vector<repository::ConversationEntity> GetConversations(const std::string& token);

    int32_t MarkAsRead(const std::string& token, const std::string& peer_id);

    bool DeleteMessage(const std::string& token, int64_t message_id);

    int32_t GetTotalUnread(const std::string& token);

private:
    MessageService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_MESSAGE_SERVICE_H
