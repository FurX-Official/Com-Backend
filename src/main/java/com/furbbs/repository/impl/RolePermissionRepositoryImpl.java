package com.furbbs.repository.impl;

import com.furbbs.model.RolePermission;
import com.furbbs.repository.RolePermissionRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

@Repository
public class RolePermissionRepositoryImpl implements RolePermissionRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<RolePermission> rolePermissionRowMapper = new RowMapper<RolePermission>() {
        @Override
        public RolePermission mapRow(ResultSet rs, int rowNum) throws SQLException {
            RolePermission rolePermission = new RolePermission();
            rolePermission.setRoleId(rs.getLong("role_id"));
            rolePermission.setPermissionId(rs.getLong("permission_id"));
            return rolePermission;
        }
    };

    @Override
    public void save(RolePermission rolePermission) {
        String sql = "INSERT INTO role_permissions (role_id, permission_id) VALUES (?, ?)";
        jdbcTemplate.update(sql, rolePermission.getRoleId(), rolePermission.getPermissionId());
    }

    @Override
    public void deleteByRoleId(Long roleId) {
        String sql = "DELETE FROM role_permissions WHERE role_id = ?";
        jdbcTemplate.update(sql, roleId);
    }

    @Override
    public void deleteByRoleIdAndPermissionId(Long roleId, Long permissionId) {
        String sql = "DELETE FROM role_permissions WHERE role_id = ? AND permission_id = ?";
        jdbcTemplate.update(sql, roleId, permissionId);
    }

    @Override
    public List<RolePermission> findByRoleId(Long roleId) {
        String sql = "SELECT * FROM role_permissions WHERE role_id = ?";
        return jdbcTemplate.query(sql, rolePermissionRowMapper, roleId);
    }

    @Override
    public List<RolePermission> findByPermissionId(Long permissionId) {
        String sql = "SELECT * FROM role_permissions WHERE permission_id = ?";
        return jdbcTemplate.query(sql, rolePermissionRowMapper, permissionId);
    }
}