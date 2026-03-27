package com.furbbs.service.impl;

import com.furbbs.model.Post;
import com.furbbs.repository.PostRepository;
import com.furbbs.repository.TagRepository;
import com.furbbs.repository.MediaFileRepository;
import com.furbbs.service.PostService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.cache.annotation.Cacheable;
import org.springframework.cache.annotation.CacheEvict;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;

@Service
public class PostServiceImpl implements PostService {

    @Autowired
    private PostRepository postRepository;

    @Autowired
    private TagRepository tagRepository;

    @Autowired
    private MediaFileRepository mediaFileRepository;

    @Override
    @CacheEvict(value = {"posts", "postCount"}, allEntries = true)
    public Post createPost(Post post, List<Long> tagIds) {
        Post savedPost = postRepository.save(post);
        if (tagIds != null && !tagIds.isEmpty()) {
            for (Long tagId : tagIds) {
                tagRepository.addTagToPost(savedPost.getId(), tagId);
            }
        }
        return savedPost;
    }

    @Override
    @CacheEvict(value = {"posts", "postCount"}, allEntries = true)
    public Post updatePost(Post post, List<Long> tagIds) {
        Post updatedPost = postRepository.save(post);
        // Remove existing tags
        tagRepository.removeAllTagsFromPost(updatedPost.getId());
        // Add new tags
        if (tagIds != null && !tagIds.isEmpty()) {
            for (Long tagId : tagIds) {
                tagRepository.addTagToPost(updatedPost.getId(), tagId);
            }
        }
        return updatedPost;
    }

    @Override
    @Cacheable(value = "posts", key = "#id")
    public Optional<Post> getPostById(Long id) {
        return postRepository.findById(id);
    }

    @Override
    @Cacheable(value = "posts", key = "'all_' + #page + '_' + #size + '_' + #sortBy + '_' + #order")
    public List<Post> getAllPosts(int page, int size, String sortBy, String order) {
        return postRepository.findAll(page, size, sortBy, order);
    }

    @Override
    @Cacheable(value = "posts", key = "'category_' + #categoryId + '_' + #page + '_' + #size + '_' + #sortBy + '_' + #order")
    public List<Post> getPostsByCategory(Long categoryId, int page, int size, String sortBy, String order) {
        return postRepository.findByCategory(categoryId, page, size, sortBy, order);
    }

    @Override
    @Cacheable(value = "posts", key = "'user_' + #userId + '_' + #page + '_' + #size + '_' + #sortBy + '_' + #order")
    public List<Post> getPostsByUserId(Long userId, int page, int size, String sortBy, String order) {
        return postRepository.findByUserId(userId, page, size, sortBy, order);
    }

    @Override
    @Cacheable(value = "posts", key = "'search_' + #keyword + '_' + #page + '_' + #size + '_' + #sortBy + '_' + #order")
    public List<Post> searchPosts(String keyword, int page, int size, String sortBy, String order) {
        return postRepository.search(keyword, page, size, sortBy, order);
    }

    @Override
    @CacheEvict(value = {"posts", "postCount"}, allEntries = true)
    public void deletePost(Long id) {
        // Remove tags
        tagRepository.removeAllTagsFromPost(id);
        // Remove media files
        mediaFileRepository.deleteByPostId(id);
        // Delete post
        postRepository.deleteById(id);
    }

    @Override
    @CacheEvict(value = "posts", key = "#postId")
    public void incrementViewCount(Long postId) {
        postRepository.incrementViewCount(postId);
    }

    @Override
    @CacheEvict(value = "posts", key = "#postId")
    public void incrementLikeCount(Long postId) {
        postRepository.incrementLikeCount(postId);
    }

    @Override
    @CacheEvict(value = "posts", key = "#postId")
    public void decrementLikeCount(Long postId) {
        postRepository.decrementLikeCount(postId);
    }

    @Override
    @CacheEvict(value = "posts", key = "#postId")
    public void incrementCommentCount(Long postId) {
        postRepository.incrementCommentCount(postId);
    }

    @Override
    @CacheEvict(value = "posts", key = "#postId")
    public void decrementCommentCount(Long postId) {
        postRepository.decrementCommentCount(postId);
    }

    @Override
    @Cacheable(value = "postCount", key = "'total'")
    public int countPosts() {
        return postRepository.count();
    }

    @Override
    @Cacheable(value = "postCount", key = "'category_' + #categoryId")
    public int countPostsByCategory(Long categoryId) {
        return postRepository.countByCategory(categoryId);
    }

    @Override
    @Cacheable(value = "postCount", key = "'user_' + #userId")
    public int countPostsByUserId(Long userId) {
        return postRepository.countByUserId(userId);
    }

    @Override
    @Cacheable(value = "postCount", key = "'search_' + #keyword")
    public int countPostsBySearch(String keyword) {
        return postRepository.countBySearch(keyword);
    }
}