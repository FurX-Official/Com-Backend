package com.furbbs.service;

import com.furbbs.model.Notification;

import java.util.List;

public interface NotificationService {
    void createNotification(Long userId, String type, String content, Long relatedId);
    List<Notification> getNotificationsByUserId(Long userId);
    long getUnreadNotificationCount(Long userId);
    void markNotificationAsRead(Long notificationId);
    void markAllNotificationsAsRead(Long userId);
}
