package com.furbbs.controller;

import com.furbbs.model.Post;
import com.furbbs.service.PostService;
import com.furbbs.util.XssFilterUtil;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/posts")
public class PostController {

    @Autowired
    private PostService postService;

    @PostMapping
    public ResponseEntity<Post> createPost(@RequestBody PostRequest postRequest) {
        Post post = new Post();
        post.setTitle(XssFilterUtil.sanitizeInput(postRequest.getTitle()));
        post.setContent(XssFilterUtil.sanitizeInput(postRequest.getContent()));
        post.setUserId(postRequest.getUserId());
        post.setCategoryId(postRequest.getCategoryId());
        post.setStatus(XssFilterUtil.sanitizeInput(postRequest.getStatus()));
        
        Post createdPost = postService.createPost(post, postRequest.getTagIds());
        return new ResponseEntity<>(createdPost, HttpStatus.CREATED);
    }

    @PutMapping("/{id}")
    public ResponseEntity<Post> updatePost(@PathVariable Long id, @RequestBody PostRequest postRequest) {
        Post post = new Post();
        post.setId(id);
        post.setTitle(XssFilterUtil.sanitizeInput(postRequest.getTitle()));
        post.setContent(XssFilterUtil.sanitizeInput(postRequest.getContent()));
        post.setUserId(postRequest.getUserId());
        post.setCategoryId(postRequest.getCategoryId());
        post.setStatus(XssFilterUtil.sanitizeInput(postRequest.getStatus()));
        
        Post updatedPost = postService.updatePost(post, postRequest.getTagIds());
        return new ResponseEntity<>(updatedPost, HttpStatus.OK);
    }

    @GetMapping("/search")
    public ResponseEntity<List<Post>> searchPosts(
            @RequestParam String keyword,
            @RequestParam(defaultValue = "1") int page,
            @RequestParam(defaultValue = "10") int size,
            @RequestParam(defaultValue = "created_at") String sortBy,
            @RequestParam(defaultValue = "desc") String order) {
        String sanitizedKeyword = XssFilterUtil.sanitizeInput(keyword);
        List<Post> posts = postService.searchPosts(sanitizedKeyword, page, size, sortBy, order);
        return new ResponseEntity<>(posts, HttpStatus.OK);
    }

    @GetMapping("/{id}")
    public ResponseEntity<Post> getPostById(@PathVariable Long id) {
        // Increment view count
        postService.incrementViewCount(id);
        
        return postService.getPostById(id)
                .map(post -> new ResponseEntity<>(post, HttpStatus.OK))
                .orElse(new ResponseEntity<>(HttpStatus.NOT_FOUND));
    }

    @GetMapping
    public ResponseEntity<List<Post>> getAllPosts(
            @RequestParam(defaultValue = "1") int page,
            @RequestParam(defaultValue = "10") int size,
            @RequestParam(defaultValue = "created_at") String sortBy,
            @RequestParam(defaultValue = "desc") String order) {
        List<Post> posts = postService.getAllPosts(page, size, sortBy, order);
        return new ResponseEntity<>(posts, HttpStatus.OK);
    }

    @GetMapping("/category/{categoryId}")
    public ResponseEntity<List<Post>> getPostsByCategory(
            @PathVariable Long categoryId,
            @RequestParam(defaultValue = "1") int page,
            @RequestParam(defaultValue = "10") int size,
            @RequestParam(defaultValue = "created_at") String sortBy,
            @RequestParam(defaultValue = "desc") String order) {
        List<Post> posts = postService.getPostsByCategory(categoryId, page, size, sortBy, order);
        return new ResponseEntity<>(posts, HttpStatus.OK);
    }

    @GetMapping("/user/{userId}")
    public ResponseEntity<List<Post>> getPostsByUserId(
            @PathVariable Long userId,
            @RequestParam(defaultValue = "1") int page,
            @RequestParam(defaultValue = "10") int size,
            @RequestParam(defaultValue = "created_at") String sortBy,
            @RequestParam(defaultValue = "desc") String order) {
        List<Post> posts = postService.getPostsByUserId(userId, page, size, sortBy, order);
        return new ResponseEntity<>(posts, HttpStatus.OK);
    }

    @GetMapping("/search")
    public ResponseEntity<List<Post>> searchPosts(
            @RequestParam String keyword,
            @RequestParam(defaultValue = "1") int page,
            @RequestParam(defaultValue = "10") int size,
            @RequestParam(defaultValue = "created_at") String sortBy,
            @RequestParam(defaultValue = "desc") String order) {
        List<Post> posts = postService.searchPosts(keyword, page, size, sortBy, order);
        return new ResponseEntity<>(posts, HttpStatus.OK);
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deletePost(@PathVariable Long id) {
        postService.deletePost(id);
        return new ResponseEntity<>(HttpStatus.NO_CONTENT);
    }

    @PostMapping("/{id}/like")
    public ResponseEntity<Void> likePost(@PathVariable Long id) {
        postService.incrementLikeCount(id);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @DeleteMapping("/{id}/like")
    public ResponseEntity<Void> unlikePost(@PathVariable Long id) {
        postService.decrementLikeCount(id);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    // Request body class for post creation and update
    public static class PostRequest {
        private String title;
        private String content;
        private Long userId;
        private Long categoryId;
        private String status;
        private List<Long> tagIds;

        public String getTitle() {
            return title;
        }

        public void setTitle(String title) {
            this.title = title;
        }

        public String getContent() {
            return content;
        }

        public void setContent(String content) {
            this.content = content;
        }

        public Long getUserId() {
            return userId;
        }

        public void setUserId(Long userId) {
            this.userId = userId;
        }

        public Long getCategoryId() {
            return categoryId;
        }

        public void setCategoryId(Long categoryId) {
            this.categoryId = categoryId;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }

        public List<Long> getTagIds() {
            return tagIds;
        }

        public void setTagIds(List<Long> tagIds) {
            this.tagIds = tagIds;
        }
    }
}