package com.furbbs.repository;

import com.furbbs.model.Category;
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
public class CategoryRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<Category> categoryRowMapper = new RowMapper<Category>() {
        @Override
        public Category mapRow(ResultSet rs, int rowNum) throws SQLException {
            Category category = new Category();
            category.setId(rs.getLong("id"));
            category.setName(rs.getString("name"));
            category.setDescription(rs.getString("description"));
            category.setCreatedAt(rs.getTimestamp("created_at"));
            category.setUpdatedAt(rs.getTimestamp("updated_at"));
            return category;
        }
    };

    public Category save(Category category) {
        if (category.getId() == null) {
            // Insert new category
            String sql = "INSERT INTO categories (name, description, created_at, updated_at) VALUES (?, ?, ?, ?)";
            Date now = new Date();
            jdbcTemplate.update(sql, category.getName(), category.getDescription(), now, now);
            // Get the generated id
            Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
            category.setId(id);
            category.setCreatedAt(now);
            category.setUpdatedAt(now);
        } else {
            // Update existing category
            String sql = "UPDATE categories SET name = ?, description = ?, updated_at = ? WHERE id = ?";
            Date now = new Date();
            jdbcTemplate.update(sql, category.getName(), category.getDescription(), now, category.getId());
            category.setUpdatedAt(now);
        }
        return category;
    }

    public Optional<Category> findById(Long id) {
        String sql = "SELECT * FROM categories WHERE id = ?";
        try {
            Category category = jdbcTemplate.queryForObject(sql, categoryRowMapper, id);
            return Optional.of(category);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public Optional<Category> findByName(String name) {
        String sql = "SELECT * FROM categories WHERE name = ?";
        try {
            Category category = jdbcTemplate.queryForObject(sql, categoryRowMapper, name);
            return Optional.of(category);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public List<Category> findAll() {
        String sql = "SELECT * FROM categories";
        return jdbcTemplate.query(sql, categoryRowMapper);
    }

    public void deleteById(Long id) {
        String sql = "DELETE FROM categories WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }
    
    public List<Map<String, Object>> findHotCategories(int limit) {
        String sql = "SELECT c.id, c.name, COUNT(p.id) as post_count " +
                     "FROM categories c " +
                     "LEFT JOIN posts p ON c.id = p.category_id " +
                     "GROUP BY c.id, c.name " +
                     "ORDER BY post_count DESC " +
                     "LIMIT ?";
        return jdbcTemplate.queryForList(sql, limit);
    }
}