package com.furbbs.repository.impl;

import com.furbbs.model.Role;
import com.furbbs.repository.RoleRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;
import java.util.List;

@Repository
public class RoleRepositoryImpl implements RoleRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<Role> roleRowMapper = new RowMapper<Role>() {
        @Override
        public Role mapRow(ResultSet rs, int rowNum) throws SQLException {
            Role role = new Role();
            role.setId(rs.getLong("id"));
            role.setName(rs.getString("name"));
            role.setDescription(rs.getString("description"));
            role.setCreatedAt(rs.getTimestamp("created_at"));
            role.setUpdatedAt(rs.getTimestamp("updated_at"));
            return role;
        }
    };

    @Override
    public Role findById(Long id) {
        String sql = "SELECT * FROM roles WHERE id = ?";
        try {
            return jdbcTemplate.queryForObject(sql, roleRowMapper, id);
        } catch (Exception e) {
            return null;
        }
    }

    @Override
    public List<Role> findAll() {
        String sql = "SELECT * FROM roles";
        return jdbcTemplate.query(sql, roleRowMapper);
    }

    @Override
    public void save(Role role) {
        if (role.getId() == null) {
            // Insert new role
            String sql = "INSERT INTO roles (name, description, created_at, updated_at) VALUES (?, ?, ?, ?)";
            Date now = new Date();
            jdbcTemplate.update(sql, role.getName(), role.getDescription(), now, now);
            // Get the generated id
            Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
            role.setId(id);
            role.setCreatedAt(now);
            role.setUpdatedAt(now);
        } else {
            // Update existing role
            String sql = "UPDATE roles SET name = ?, description = ?, updated_at = ? WHERE id = ?";
            Date now = new Date();
            jdbcTemplate.update(sql, role.getName(), role.getDescription(), now, role.getId());
            role.setUpdatedAt(now);
        }
    }

    @Override
    public void update(Role role) {
        save(role);
    }

    @Override
    public void delete(Long id) {
        String sql = "DELETE FROM roles WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }

    @Override
    public Role findByName(String name) {
        String sql = "SELECT * FROM roles WHERE name = ?";
        try {
            return jdbcTemplate.queryForObject(sql, roleRowMapper, name);
        } catch (Exception e) {
            return null;
        }
    }

    @Override
    public List<Role> findByUserId(Long userId) {
        String sql = "SELECT r.* FROM roles r JOIN user_roles ur ON r.id = ur.role_id WHERE ur.user_id = ?";
        return jdbcTemplate.query(sql, roleRowMapper, userId);
    }
}