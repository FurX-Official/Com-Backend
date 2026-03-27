package com.furbbs.repository.impl;

import com.furbbs.model.Permission;
import com.furbbs.repository.PermissionRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;
import java.util.List;

@Repository
public class PermissionRepositoryImpl implements PermissionRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<Permission> permissionRowMapper = new RowMapper<Permission>() {
        @Override
        public Permission mapRow(ResultSet rs, int rowNum) throws SQLException {
            Permission permission = new Permission();
            permission.setId(rs.getLong("id"));
            permission.setName(rs.getString("name"));
            permission.setCode(rs.getString("code"));
            permission.setDescription(rs.getString("description"));
            permission.setCreatedAt(rs.getTimestamp("created_at"));
            permission.setUpdatedAt(rs.getTimestamp("updated_at"));
            return permission;
        }
    };

    @Override
    public Permission findById(Long id) {
        String sql = "SELECT * FROM permissions WHERE id = ?";
        try {
            return jdbcTemplate.queryForObject(sql, permissionRowMapper, id);
        } catch (Exception e) {
            return null;
        }
    }

    @Override
    public List<Permission> findAll() {
        String sql = "SELECT * FROM permissions";
        return jdbcTemplate.query(sql, permissionRowMapper);
    }

    @Override
    public void save(Permission permission) {
        if (permission.getId() == null) {
            // Insert new permission
            String sql = "INSERT INTO permissions (name, code, description, created_at, updated_at) VALUES (?, ?, ?, ?, ?)";
            Date now = new Date();
            jdbcTemplate.update(sql, permission.getName(), permission.getCode(), permission.getDescription(), now, now);
            // Get the generated id
            Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
            permission.setId(id);
            permission.setCreatedAt(now);
            permission.setUpdatedAt(now);
        } else {
            // Update existing permission
            String sql = "UPDATE permissions SET name = ?, code = ?, description = ?, updated_at = ? WHERE id = ?";
            Date now = new Date();
            jdbcTemplate.update(sql, permission.getName(), permission.getCode(), permission.getDescription(), now, permission.getId());
            permission.setUpdatedAt(now);
        }
    }

    @Override
    public void update(Permission permission) {
        save(permission);
    }

    @Override
    public void delete(Long id) {
        String sql = "DELETE FROM permissions WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }

    @Override
    public Permission findByCode(String code) {
        String sql = "SELECT * FROM permissions WHERE code = ?";
        try {
            return jdbcTemplate.queryForObject(sql, permissionRowMapper, code);
        } catch (Exception e) {
            return null;
        }
    }

    @Override
    public List<Permission> findByRoleId(Long roleId) {
        String sql = "SELECT p.* FROM permissions p JOIN role_permissions rp ON p.id = rp.permission_id WHERE rp.role_id = ?";
        return jdbcTemplate.query(sql, permissionRowMapper, roleId);
    }

    @Override
    public List<Permission> findByUserId(Long userId) {
        String sql = "SELECT p.* FROM permissions p JOIN role_permissions rp ON p.id = rp.permission_id JOIN user_roles ur ON rp.role_id = ur.role_id WHERE ur.user_id = ?";
        return jdbcTemplate.query(sql, permissionRowMapper, userId);
    }
}