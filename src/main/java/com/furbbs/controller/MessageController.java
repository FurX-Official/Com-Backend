package com.furbbs.controller;

import com.furbbs.model.Message;
import com.furbbs.service.MessageService;
import com.furbbs.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/messages")
public class MessageController {
    @Autowired
    private MessageService messageService;

    @Autowired
    private NotificationService notificationService;

    @PostMapping
    public ResponseEntity<Message> sendMessage(@RequestBody Message message) {
        Message sentMessage = messageService.sendMessage(message);
        // 创建通知给接收者
        if (message.getReceiver() != null) {
            notificationService.createNotification(
                message.getReceiver().getId(),
                "MESSAGE",
                "你收到了一条新消息",
                sentMessage.getId()
            );
        }
        return ResponseEntity.ok(sentMessage);
    }

    @GetMapping("/conversation")
    public ResponseEntity<List<Message>> getMessagesBetweenUsers(
            @RequestParam Long senderId, 
            @RequestParam Long receiverId) {
        List<Message> messages = messageService.getMessagesBetweenUsers(senderId, receiverId);
        return ResponseEntity.ok(messages);
    }

    @GetMapping("/inbox")
    public ResponseEntity<List<Message>> getMessagesByReceiverId(@RequestParam Long receiverId) {
        List<Message> messages = messageService.getMessagesByReceiverId(receiverId);
        return ResponseEntity.ok(messages);
    }

    @GetMapping("/unread-count")
    public ResponseEntity<Long> getUnreadMessageCount(@RequestParam Long receiverId) {
        long count = messageService.getUnreadMessageCount(receiverId);
        return ResponseEntity.ok(count);
    }

    @PutMapping("/read/{messageId}")
    public ResponseEntity<Void> markMessageAsRead(@PathVariable Long messageId) {
        messageService.markMessageAsRead(messageId);
        return ResponseEntity.noContent().build();
    }
}
