#include "controller/common.h"
#include "service_impl/message_service.h"

namespace furbbs::controller {

trpc::Status MessageController::SendMessage(::trpc::ServerContextPtr,
                                             const ::furbbs::SendMessageRequest* request,
                                             ::furbbs::SendMessageResponse* response) {
    auto msg_id = service::MessageService::Instance().SendMessage(
        request->token(), request->receiver_id(),
        request->content(), request->message_type());

    response->set_message_id(msg_id);
    response->set_success(msg_id > 0);
    return trpc::kSuccStatus;
}

trpc::Status MessageController::GetConversationMessages(::trpc::ServerContextPtr,
                                                         const ::furbbs::GetConversationMessagesRequest* request,
                                                         ::furbbs::GetConversationMessagesResponse* response) {
    auto list = service::MessageService::Instance().GetConversationMessages(
        request->token(), request->peer_id(), request->before_time(), request->limit());

    for (const auto& item : list) {
        auto* msg = response->add_messages();
        msg->set_id(item.id);
        msg->set_sender_id(item.sender_id);
        msg->set_receiver_id(item.receiver_id);
        msg->set_content(item.content);
        msg->set_message_type(item.message_type);
        msg->set_is_read(item.is_read);
        msg->set_created_at(item.created_at);
    }
    return trpc::kSuccStatus;
}

trpc::Status MessageController::GetConversations(::trpc::ServerContextPtr,
                                                  const ::furbbs::GetConversationsRequest* request,
                                                  ::furbbs::GetConversationsResponse* response) {
    auto list = service::MessageService::Instance().GetConversations(request->token());
    for (const auto& item : list) {
        auto* conv = response->add_conversations();
        conv->set_id(item.id);
        conv->set_peer_id(item.peer_id);
        conv->set_peer_name(item.peer_name);
        conv->set_peer_avatar(item.peer_avatar);
        conv->set_last_message(item.last_message);
        conv->set_last_message_at(item.last_message_at);
        conv->set_unread_count(item.unread_count);
        conv->set_is_online(item.is_online);
    }
    return trpc::kSuccStatus;
}

trpc::Status MessageController::MarkAsRead(::trpc::ServerContextPtr,
                                            const ::furbbs::MarkAsReadRequest* request,
                                            ::furbbs::MarkAsReadResponse* response) {
    auto count = service::MessageService::Instance().MarkAsRead(
        request->token(), request->peer_id());
    response->set_marked_count(count);
    return trpc::kSuccStatus;
}

trpc::Status MessageController::DeleteMessage(::trpc::ServerContextPtr,
                                               const ::furbbs::DeleteMessageRequest* request,
                                               ::furbbs::DeleteMessageResponse* response) {
    bool success = service::MessageService::Instance().DeleteMessage(
        request->token(), request->message_id());
    response->set_success(success);
    return trpc::kSuccStatus;
}

trpc::Status MessageController::GetUnreadCount(::trpc::ServerContextPtr,
                                                const ::furbbs::GetUnreadCountRequest* request,
                                                ::furbbs::GetUnreadCountResponse* response) {
    auto count = service::MessageService::Instance().GetTotalUnread(request->token());
    response->set_total_unread(count);
    return trpc::kSuccStatus;
}

} // namespace furbbs::controller
