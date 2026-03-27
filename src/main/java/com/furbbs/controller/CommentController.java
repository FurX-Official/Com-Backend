package com.furbbs.controller;

import com.furbbs.model.Comment;
import com.furbbs.service.CommentService;
import com.furbbs.service.NotificationService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/comments")
public class CommentController {
    @Autowired
    private CommentService commentService;

    @Autowired
    private NotificationService notificationService;

    @PostMapping
    public ResponseEntity<Comment> createComment(@RequestBody Comment comment) {
        Comment createdComment = commentService.createComment(comment);
        // 创建通知给帖子作者
        if (comment.getPost() != null && comment.getPost().getUser() != null) {
            notificationService.createNotification(
                comment.getPost().getUser().getId(),
                "COMMENT",
                "有人评论了你的帖子",
                comment.getPost().getId()
            );
        }
        return ResponseEntity.ok(createdComment);
    }

    @GetMapping("/post/{postId}")
    public ResponseEntity<List<Comment>> getCommentsByPostId(@PathVariable Long postId) {
        List<Comment> comments = commentService.getCommentsByPostId(postId);
        return ResponseEntity.ok(comments);
    }

    @GetMapping("/replies/{commentId}")
    public ResponseEntity<List<Comment>> getRepliesByCommentId(@PathVariable Long commentId) {
        List<Comment> replies = commentService.getRepliesByCommentId(commentId);
        return ResponseEntity.ok(replies);
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deleteComment(@PathVariable Long id) {
        commentService.deleteComment(id);
        return ResponseEntity.noContent().build();
    }
}
