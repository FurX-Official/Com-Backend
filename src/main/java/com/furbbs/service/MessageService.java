package com.furbbs.service;

import com.furbbs.model.Message;

import java.util.List;

public interface MessageService {
    Message sendMessage(Message message);
    List<Message> getMessagesBetweenUsers(Long senderId, Long receiverId);
    List<Message> getMessagesByReceiverId(Long receiverId);
    long getUnreadMessageCount(Long receiverId);
    void markMessageAsRead(Long messageId);
}
