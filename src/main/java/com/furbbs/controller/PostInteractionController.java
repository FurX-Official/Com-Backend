package com.furbbs.controller;

import com.furbbs.service.PostInteractionService;
import com.furbbs.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api/post-interactions")
public class PostInteractionController {
    @Autowired
    private PostInteractionService postInteractionService;

    @Autowired
    private NotificationService notificationService;

    @PostMapping("/like/{postId}")
    public ResponseEntity<Boolean> likePost(@RequestParam Long userId, @PathVariable Long postId) {
        boolean success = postInteractionService.likePost(userId, postId);
        if (success) {
            // 创建通知给帖子作者
            // 注意：这里需要获取帖子作者的ID，实际应用中需要从Post对象中获取
            // notificationService.createNotification(postAuthorId, "LIKE", "有人点赞了你的帖子", postId);
        }
        return ResponseEntity.ok(success);
    }

    @PostMapping("/unlike/{postId}")
    public ResponseEntity<Boolean> unlikePost(@RequestParam Long userId, @PathVariable Long postId) {
        boolean success = postInteractionService.unlikePost(userId, postId);
        return ResponseEntity.ok(success);
    }

    @GetMapping("/is-liked/{postId}")
    public ResponseEntity<Boolean> isPostLiked(@RequestParam Long userId, @PathVariable Long postId) {
        boolean isLiked = postInteractionService.isPostLiked(userId, postId);
        return ResponseEntity.ok(isLiked);
    }

    @GetMapping("/like-count/{postId}")
    public ResponseEntity<Long> getPostLikeCount(@PathVariable Long postId) {
        long count = postInteractionService.getPostLikeCount(postId);
        return ResponseEntity.ok(count);
    }

    @PostMapping("/favorite/{postId}")
    public ResponseEntity<Boolean> favoritePost(@RequestParam Long userId, @PathVariable Long postId) {
        boolean success = postInteractionService.favoritePost(userId, postId);
        return ResponseEntity.ok(success);
    }

    @PostMapping("/unfavorite/{postId}")
    public ResponseEntity<Boolean> unfavoritePost(@RequestParam Long userId, @PathVariable Long postId) {
        boolean success = postInteractionService.unfavoritePost(userId, postId);
        return ResponseEntity.ok(success);
    }

    @GetMapping("/is-favorited/{postId}")
    public ResponseEntity<Boolean> isPostFavorited(@RequestParam Long userId, @PathVariable Long postId) {
        boolean isFavorited = postInteractionService.isPostFavorited(userId, postId);
        return ResponseEntity.ok(isFavorited);
    }
}
