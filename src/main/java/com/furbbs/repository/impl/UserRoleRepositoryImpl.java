package com.furbbs.repository.impl;

import com.furbbs.model.UserRole;
import com.furbbs.repository.UserRoleRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

@Repository
public class UserRoleRepositoryImpl implements UserRoleRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<UserRole> userRoleRowMapper = new RowMapper<UserRole>() {
        @Override
        public UserRole mapRow(ResultSet rs, int rowNum) throws SQLException {
            UserRole userRole = new UserRole();
            userRole.setUserId(rs.getLong("user_id"));
            userRole.setRoleId(rs.getLong("role_id"));
            return userRole;
        }
    };

    @Override
    public void save(UserRole userRole) {
        String sql = "INSERT INTO user_roles (user_id, role_id) VALUES (?, ?)";
        jdbcTemplate.update(sql, userRole.getUserId(), userRole.getRoleId());
    }

    @Override
    public void deleteByUserId(Long userId) {
        String sql = "DELETE FROM user_roles WHERE user_id = ?";
        jdbcTemplate.update(sql, userId);
    }

    @Override
    public void deleteByUserIdAndRoleId(Long userId, Long roleId) {
        String sql = "DELETE FROM user_roles WHERE user_id = ? AND role_id = ?";
        jdbcTemplate.update(sql, userId, roleId);
    }

    @Override
    public List<UserRole> findByUserId(Long userId) {
        String sql = "SELECT * FROM user_roles WHERE user_id = ?";
        return jdbcTemplate.query(sql, userRoleRowMapper, userId);
    }

    @Override
    public List<UserRole> findByRoleId(Long roleId) {
        String sql = "SELECT * FROM user_roles WHERE role_id = ?";
        return jdbcTemplate.query(sql, userRoleRowMapper, roleId);
    }
}