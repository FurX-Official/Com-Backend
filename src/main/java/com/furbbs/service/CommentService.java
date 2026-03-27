package com.furbbs.service;

import com.furbbs.model.Comment;

import java.util.List;

public interface CommentService {
    Comment createComment(Comment comment);
    Comment getCommentById(Long id);
    List<Comment> getCommentsByPostId(Long postId);
    List<Comment> getRepliesByCommentId(Long commentId);
    void deleteComment(Long id);
}
