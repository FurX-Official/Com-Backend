package com.furbbs.service.impl;

import com.furbbs.model.PostLike;
import com.furbbs.model.PostFavorite;
import com.furbbs.repository.PostLikeRepository;
import com.furbbs.repository.PostFavoriteRepository;
import com.furbbs.service.PostInteractionService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class PostInteractionServiceImpl implements PostInteractionService {
    @Autowired
    private PostLikeRepository postLikeRepository;

    @Autowired
    private PostFavoriteRepository postFavoriteRepository;

    @Override
    public boolean likePost(Long userId, Long postId) {
        if (postLikeRepository.existsByUserIdAndPostId(userId, postId)) {
            return false;
        }
        PostLike postLike = new PostLike();
        postLike.setUserId(userId);
        postLike.setPostId(postId);
        postLikeRepository.save(postLike);
        return true;
    }

    @Override
    public boolean unlikePost(Long userId, Long postId) {
        postLikeRepository.findByUserIdAndPostId(userId, postId)
                .ifPresent(postLikeRepository::delete);
        return true;
    }

    @Override
    public boolean isPostLiked(Long userId, Long postId) {
        return postLikeRepository.existsByUserIdAndPostId(userId, postId);
    }

    @Override
    public long getPostLikeCount(Long postId) {
        return postLikeRepository.countByPostId(postId);
    }

    @Override
    public boolean favoritePost(Long userId, Long postId) {
        if (postFavoriteRepository.existsByUserIdAndPostId(userId, postId)) {
            return false;
        }
        PostFavorite postFavorite = new PostFavorite();
        postFavorite.setUserId(userId);
        postFavorite.setPostId(postId);
        postFavoriteRepository.save(postFavorite);
        return true;
    }

    @Override
    public boolean unfavoritePost(Long userId, Long postId) {
        postFavoriteRepository.findByUserIdAndPostId(userId, postId)
                .ifPresent(postFavoriteRepository::delete);
        return true;
    }

    @Override
    public boolean isPostFavorited(Long userId, Long postId) {
        return postFavoriteRepository.existsByUserIdAndPostId(userId, postId);
    }
}
