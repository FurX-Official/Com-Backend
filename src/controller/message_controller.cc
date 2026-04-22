#include "../service_impl/message_service.h"
#include "../common/result.h"

namespace furbbs::controller {

using ::trpc::ServerContextPtr;

::trpc::Status SendMessage(ServerContextPtr context,
                            const ::furbbs::SendMessageRequest* request,
                            ::furbbs::SendMessageResponse* response) {
    try {
        int64_t msg_id = service::MessageService::Instance().SendMessage(
            request->access_token(), request->receiver_id(),
            request->content(), 0);
        response->set_code(msg_id > 0 ? RESULT_OK : RESULT_ERROR);
        response->set_message(msg_id > 0 ? "发送成功" : "发送失败");
        response->set_message_id(msg_id);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("发送失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetMessages(ServerContextPtr context,
                            const ::furbbs::GetMessagesRequest* request,
                            ::furbbs::GetMessagesResponse* response) {
    try {
        auto list = service::MessageService::Instance().GetConversationMessages(
            request->access_token(), request->other_user_id(), 0, request->page_size());
        for (const auto& item : list) {
            auto* msg = response->add_messages();
            msg->set_id(item.id);
            msg->set_sender_id(item.sender_id);
            msg->set_receiver_id(item.receiver_id);
            msg->set_content(item.content);
            msg->set_is_read(item.is_read);
            msg->set_created_at(item.created_at);
        }
        response->set_total(static_cast<int32_t>(list.size()));
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetMessageConversations(ServerContextPtr context,
                                        const ::furbbs::GetMessageConversationsRequest* request,
                                        ::furbbs::GetMessageConversationsResponse* response) {
    try {
        auto list = service::MessageService::Instance().GetConversations(request->access_token());
        for (const auto& item : list) {
            auto* conv = response->add_conversations();
            conv->set_peer_id(item.peer_id);
            conv->set_peer_name(item.peer_name);
            conv->set_peer_avatar(item.peer_avatar);
            conv->set_last_message(item.last_message);
            conv->set_last_message_at(item.last_message_at);
            conv->set_unread_count(item.unread_count);
        }
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status MarkMessageRead(ServerContextPtr context,
                                const ::furbbs::MarkMessageReadRequest* request,
                                ::furbbs::MarkMessageReadResponse* response) {
    try {
        int32_t count = service::MessageService::Instance().MarkAsRead(
            request->access_token(), request->peer_id());
        response->set_marked_count(count);
        response->set_code(RESULT_OK);
        response->set_message("标记成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("操作失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

} // namespace furbbs::controller
