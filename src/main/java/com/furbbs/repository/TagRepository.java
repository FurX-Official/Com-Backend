package com.furbbs.repository;

import com.furbbs.model.Tag;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@Repository
public class TagRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<Tag> tagRowMapper = new RowMapper<Tag>() {
        @Override
        public Tag mapRow(ResultSet rs, int rowNum) throws SQLException {
            Tag tag = new Tag();
            tag.setId(rs.getLong("id"));
            tag.setName(rs.getString("name"));
            tag.setCreatedAt(rs.getTimestamp("created_at"));
            tag.setUpdatedAt(rs.getTimestamp("updated_at"));
            return tag;
        }
    };

    public Tag save(Tag tag) {
        if (tag.getId() == null) {
            // Insert new tag
            String sql = "INSERT INTO tags (name, created_at, updated_at) VALUES (?, ?, ?)";
            Date now = new Date();
            jdbcTemplate.update(sql, tag.getName(), now, now);
            // Get the generated id
            Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
            tag.setId(id);
            tag.setCreatedAt(now);
            tag.setUpdatedAt(now);
        } else {
            // Update existing tag
            String sql = "UPDATE tags SET name = ?, updated_at = ? WHERE id = ?";
            Date now = new Date();
            jdbcTemplate.update(sql, tag.getName(), now, tag.getId());
            tag.setUpdatedAt(now);
        }
        return tag;
    }

    public Optional<Tag> findById(Long id) {
        String sql = "SELECT * FROM tags WHERE id = ?";
        try {
            Tag tag = jdbcTemplate.queryForObject(sql, tagRowMapper, id);
            return Optional.of(tag);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public Optional<Tag> findByName(String name) {
        String sql = "SELECT * FROM tags WHERE name = ?";
        try {
            Tag tag = jdbcTemplate.queryForObject(sql, tagRowMapper, name);
            return Optional.of(tag);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public List<Tag> findAll() {
        String sql = "SELECT * FROM tags";
        return jdbcTemplate.query(sql, tagRowMapper);
    }

    public List<Tag> findByPostId(Long postId) {
        String sql = "SELECT t.* FROM tags t JOIN post_tags pt ON t.id = pt.tag_id WHERE pt.post_id = ?";
        return jdbcTemplate.query(sql, tagRowMapper, postId);
    }

    public void deleteById(Long id) {
        String sql = "DELETE FROM tags WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }

    public void addTagToPost(Long postId, Long tagId) {
        String sql = "INSERT INTO post_tags (post_id, tag_id) VALUES (?, ?)";
        jdbcTemplate.update(sql, postId, tagId);
    }

    public void removeTagFromPost(Long postId, Long tagId) {
        String sql = "DELETE FROM post_tags WHERE post_id = ? AND tag_id = ?";
        jdbcTemplate.update(sql, postId, tagId);
    }

    public void removeAllTagsFromPost(Long postId) {
        String sql = "DELETE FROM post_tags WHERE post_id = ?";
        jdbcTemplate.update(sql, postId);
    }
    
    public List<Map<String, Object>> findHotTags(int limit) {
        String sql = "SELECT t.id, t.name, COUNT(pt.post_id) as post_count " +
                     "FROM tags t " +
                     "JOIN post_tags pt ON t.id = pt.tag_id " +
                     "GROUP BY t.id, t.name " +
                     "ORDER BY post_count DESC " +
                     "LIMIT ?";
        return jdbcTemplate.queryForList(sql, limit);
    }
}