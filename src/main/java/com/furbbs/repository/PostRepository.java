package com.furbbs.repository;

import com.furbbs.model.Post;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.time.LocalDateTime;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@Repository
public class PostRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<Post> postRowMapper = new RowMapper<Post>() {
        @Override
        public Post mapRow(ResultSet rs, int rowNum) throws SQLException {
            Post post = new Post();
            post.setId(rs.getLong("id"));
            post.setTitle(rs.getString("title"));
            post.setContent(rs.getString("content"));
            post.setUserId(rs.getLong("user_id"));
            post.setCategoryId(rs.getLong("category_id"));
            post.setViewCount(rs.getInt("view_count"));
            post.setLikeCount(rs.getInt("like_count"));
            post.setCommentCount(rs.getInt("comment_count"));
            post.setStatus(rs.getString("status"));
            post.setCreatedAt(rs.getTimestamp("created_at"));
            post.setUpdatedAt(rs.getTimestamp("updated_at"));
            return post;
        }
    };

    public Post save(Post post) {
        if (post.getId() == null) {
            // Insert new post
            String sql = "INSERT INTO posts (title, content, user_id, category_id, status, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
            Date now = new Date();
            jdbcTemplate.update(sql, post.getTitle(), post.getContent(), post.getUserId(), 
                post.getCategoryId(), post.getStatus(), now, now);
            // Get the generated id
            Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
            post.setId(id);
            post.setCreatedAt(now);
            post.setUpdatedAt(now);
        } else {
            // Update existing post
            String sql = "UPDATE posts SET title = ?, content = ?, category_id = ?, status = ?, updated_at = ? WHERE id = ?";
            Date now = new Date();
            jdbcTemplate.update(sql, post.getTitle(), post.getContent(), post.getCategoryId(), 
                post.getStatus(), now, post.getId());
            post.setUpdatedAt(now);
        }
        return post;
    }

    public Optional<Post> findById(Long id) {
        String sql = "SELECT * FROM posts WHERE id = ?";
        try {
            Post post = jdbcTemplate.queryForObject(sql, postRowMapper, id);
            return Optional.of(post);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public List<Post> findAll(int page, int size, String sortBy, String order) {
        // 验证sortBy和order参数，防止SQL注入
        String validSortBy = validateSortBy(sortBy);
        String validOrder = validateOrder(order);
        String sql = "SELECT * FROM posts ORDER BY " + validSortBy + " " + validOrder + " LIMIT ? OFFSET ?";
        int offset = (page - 1) * size;
        return jdbcTemplate.query(sql, postRowMapper, size, offset);
    }

    public List<Post> findByCategory(Long categoryId, int page, int size, String sortBy, String order) {
        // 验证sortBy和order参数，防止SQL注入
        String validSortBy = validateSortBy(sortBy);
        String validOrder = validateOrder(order);
        String sql = "SELECT * FROM posts WHERE category_id = ? ORDER BY " + validSortBy + " " + validOrder + " LIMIT ? OFFSET ?";
        int offset = (page - 1) * size;
        return jdbcTemplate.query(sql, postRowMapper, categoryId, size, offset);
    }

    public List<Post> findByUserId(Long userId, int page, int size, String sortBy, String order) {
        // 验证sortBy和order参数，防止SQL注入
        String validSortBy = validateSortBy(sortBy);
        String validOrder = validateOrder(order);
        String sql = "SELECT * FROM posts WHERE user_id = ? ORDER BY " + validSortBy + " " + validOrder + " LIMIT ? OFFSET ?";
        int offset = (page - 1) * size;
        return jdbcTemplate.query(sql, postRowMapper, userId, size, offset);
    }

    public List<Post> search(String keyword, int page, int size, String sortBy, String order) {
        // 验证sortBy和order参数，防止SQL注入
        String validSortBy = validateSortBy(sortBy);
        String validOrder = validateOrder(order);
        String sql = "SELECT * FROM posts WHERE title ILIKE ? OR content ILIKE ? ORDER BY " + validSortBy + " " + validOrder + " LIMIT ? OFFSET ?";
        String searchPattern = "%" + keyword + "%";
        int offset = (page - 1) * size;
        return jdbcTemplate.query(sql, postRowMapper, searchPattern, searchPattern, size, offset);
    }

    // 验证sortBy参数，只允许指定的字段
    private String validateSortBy(String sortBy) {
        if (sortBy == null) {
            return "created_at";
        }
        String[] validSortFields = {"id", "title", "created_at", "updated_at", "view_count", "like_count", "comment_count"};
        for (String field : validSortFields) {
            if (field.equals(sortBy)) {
                return field;
            }
        }
        return "created_at";
    }

    // 验证order参数，只允许ASC或DESC
    private String validateOrder(String order) {
        if (order == null) {
            return "DESC";
        }
        if (order.equalsIgnoreCase("ASC")) {
            return "ASC";
        }
        return "DESC";
    }

    public void incrementViewCount(Long postId) {
        String sql = "UPDATE posts SET view_count = view_count + 1 WHERE id = ?";
        jdbcTemplate.update(sql, postId);
    }

    public void incrementLikeCount(Long postId) {
        String sql = "UPDATE posts SET like_count = like_count + 1 WHERE id = ?";
        jdbcTemplate.update(sql, postId);
    }

    public void decrementLikeCount(Long postId) {
        String sql = "UPDATE posts SET like_count = GREATEST(like_count - 1, 0) WHERE id = ?";
        jdbcTemplate.update(sql, postId);
    }

    public void incrementCommentCount(Long postId) {
        String sql = "UPDATE posts SET comment_count = comment_count + 1 WHERE id = ?";
        jdbcTemplate.update(sql, postId);
    }

    public void decrementCommentCount(Long postId) {
        String sql = "UPDATE posts SET comment_count = GREATEST(comment_count - 1, 0) WHERE id = ?";
        jdbcTemplate.update(sql, postId);
    }

    public void deleteById(Long id) {
        String sql = "DELETE FROM posts WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }

    public int count() {
        String sql = "SELECT COUNT(*) FROM posts";
        return jdbcTemplate.queryForObject(sql, Integer.class);
    }

    public int countByCategory(Long categoryId) {
        String sql = "SELECT COUNT(*) FROM posts WHERE category_id = ?";
        return jdbcTemplate.queryForObject(sql, Integer.class, categoryId);
    }

    public int countByUserId(Long userId) {
        String sql = "SELECT COUNT(*) FROM posts WHERE user_id = ?";
        return jdbcTemplate.queryForObject(sql, Integer.class, userId);
    }

    public int countBySearch(String keyword) {
        String sql = "SELECT COUNT(*) FROM posts WHERE title ILIKE ? OR content ILIKE ?";
        String searchPattern = "%" + keyword + "%";
        return jdbcTemplate.queryForObject(sql, Integer.class, searchPattern, searchPattern);
    }
    
    public long countByCreatedAtAfter(LocalDateTime date) {
        String sql = "SELECT COUNT(*) FROM posts WHERE created_at > ?";
        return jdbcTemplate.queryForObject(sql, Long.class, date);
    }
    
    public List<Map<String, Object>> findHotPosts(int limit) {
        String sql = "SELECT p.id, p.title, p.view_count, p.like_count, p.comment_count, " +
                     "(p.view_count + p.like_count * 2 + p.comment_count * 3) as hotness, " +
                     "u.username as author " +
                     "FROM posts p " +
                     "JOIN users u ON p.user_id = u.id " +
                     "WHERE p.status = 'PUBLISHED' " +
                     "ORDER BY hotness DESC " +
                     "LIMIT ?";
        return jdbcTemplate.queryForList(sql, limit);
    }
    
    public long countByCreatedAtBetween(LocalDateTime start, LocalDateTime end) {
        String sql = "SELECT COUNT(*) FROM posts WHERE created_at BETWEEN ? AND ?";
        return jdbcTemplate.queryForObject(sql, Long.class, start, end);
    }
    
    public List<Map<String, Object>> getPostCreationTrend(LocalDateTime start, LocalDateTime end) {
        String sql = "SELECT " +
                     "DATE_TRUNC('day', created_at) as date, " +
                     "COUNT(*) as post_count " +
                     "FROM posts " +
                     "WHERE created_at BETWEEN ? AND ? " +
                     "GROUP BY DATE_TRUNC('day', created_at) " +
                     "ORDER BY date";
        return jdbcTemplate.queryForList(sql, start, end);
    }
}