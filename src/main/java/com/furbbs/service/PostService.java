package com.furbbs.service;

import com.furbbs.model.Post;
import java.util.List;
import java.util.Optional;

public interface PostService {
    Post createPost(Post post, List<Long> tagIds);
    Post updatePost(Post post, List<Long> tagIds);
    Optional<Post> getPostById(Long id);
    List<Post> getAllPosts(int page, int size, String sortBy, String order);
    List<Post> getPostsByCategory(Long categoryId, int page, int size, String sortBy, String order);
    List<Post> getPostsByUserId(Long userId, int page, int size, String sortBy, String order);
    List<Post> searchPosts(String keyword, int page, int size, String sortBy, String order);
    void deletePost(Long id);
    void incrementViewCount(Long postId);
    void incrementLikeCount(Long postId);
    void decrementLikeCount(Long postId);
    void incrementCommentCount(Long postId);
    void decrementCommentCount(Long postId);
    int countPosts();
    int countPostsByCategory(Long categoryId);
    int countPostsByUserId(Long userId);
    int countPostsBySearch(String keyword);
}