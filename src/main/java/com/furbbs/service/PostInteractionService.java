package com.furbbs.service;

public interface PostInteractionService {
    boolean likePost(Long userId, Long postId);
    boolean unlikePost(Long userId, Long postId);
    boolean isPostLiked(Long userId, Long postId);
    long getPostLikeCount(Long postId);
    boolean favoritePost(Long userId, Long postId);
    boolean unfavoritePost(Long userId, Long postId);
    boolean isPostFavorited(Long userId, Long postId);
}
