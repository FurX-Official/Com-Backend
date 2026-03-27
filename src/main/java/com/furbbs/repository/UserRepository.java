package com.furbbs.repository;

import com.furbbs.model.User;
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
public class UserRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<User> userRowMapper = new RowMapper<User>() {
        @Override
        public User mapRow(ResultSet rs, int rowNum) throws SQLException {
            User user = new User();
            user.setId(rs.getLong("id"));
            user.setUsername(rs.getString("username"));
            user.setEmail(rs.getString("email"));
            user.setPassword(rs.getString("password"));
            user.setNickname(rs.getString("nickname"));
            user.setAvatar(rs.getString("avatar"));
            user.setBio(rs.getString("bio"));
            user.setResetToken(rs.getString("reset_token"));
            user.setResetTokenExpiry(rs.getTimestamp("reset_token_expiry"));
            user.setCreatedAt(rs.getTimestamp("created_at"));
            user.setUpdatedAt(rs.getTimestamp("updated_at"));
            return user;
        }
    };

    public User save(User user) {
        if (user.getId() == null) {
            // Insert new user
            String sql = "INSERT INTO users (username, email, password, nickname, avatar, bio, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
            Date now = new Date();
            jdbcTemplate.update(sql, user.getUsername(), user.getEmail(), user.getPassword(), 
                user.getNickname(), user.getAvatar(), user.getBio(), now, now);
            // Get the generated id
            Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
            user.setId(id);
            user.setCreatedAt(now);
            user.setUpdatedAt(now);
        } else {
            // Update existing user
            String sql = "UPDATE users SET username = ?, email = ?, password = ?, nickname = ?, avatar = ?, bio = ?, updated_at = ? WHERE id = ?";
            Date now = new Date();
            jdbcTemplate.update(sql, user.getUsername(), user.getEmail(), user.getPassword(), 
                user.getNickname(), user.getAvatar(), user.getBio(), now, user.getId());
            user.setUpdatedAt(now);
        }
        return user;
    }

    public Optional<User> findById(Long id) {
        String sql = "SELECT * FROM users WHERE id = ?";
        try {
            User user = jdbcTemplate.queryForObject(sql, userRowMapper, id);
            return Optional.of(user);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public Optional<User> findByUsername(String username) {
        String sql = "SELECT * FROM users WHERE username = ?";
        try {
            User user = jdbcTemplate.queryForObject(sql, userRowMapper, username);
            return Optional.of(user);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public Optional<User> findByEmail(String email) {
        String sql = "SELECT * FROM users WHERE email = ?";
        try {
            User user = jdbcTemplate.queryForObject(sql, userRowMapper, email);
            return Optional.of(user);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public Optional<User> findByResetToken(String resetToken) {
        String sql = "SELECT * FROM users WHERE reset_token = ? AND reset_token_expiry > ?";
        try {
            User user = jdbcTemplate.queryForObject(sql, userRowMapper, resetToken, new Date());
            return Optional.of(user);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public void updateResetToken(String email, String resetToken, Date expiry) {
        String sql = "UPDATE users SET reset_token = ?, reset_token_expiry = ? WHERE email = ?";
        jdbcTemplate.update(sql, resetToken, expiry, email);
    }

    public void clearResetToken(Long userId) {
        String sql = "UPDATE users SET reset_token = NULL, reset_token_expiry = NULL WHERE id = ?";
        jdbcTemplate.update(sql, userId);
    }

    public List<User> findAll() {
        String sql = "SELECT * FROM users";
        return jdbcTemplate.query(sql, userRowMapper);
    }

    public void deleteById(Long id) {
        String sql = "DELETE FROM users WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }
    
    public long count() {
        String sql = "SELECT COUNT(*) FROM users";
        return jdbcTemplate.queryForObject(sql, Long.class);
    }
    
    public long countByCreatedAtAfter(LocalDateTime date) {
        String sql = "SELECT COUNT(*) FROM users WHERE created_at > ?";
        return jdbcTemplate.queryForObject(sql, Long.class, date);
    }
    
    public long countActiveUsers(LocalDateTime since) {
        String sql = "SELECT COUNT(DISTINCT u.id) FROM users u " +
                     "LEFT JOIN posts p ON u.id = p.user_id " +
                     "LEFT JOIN comments c ON u.id = c.user_id " +
                     "LEFT JOIN post_likes pl ON u.id = pl.user_id " +
                     "LEFT JOIN post_favorites pf ON u.id = pf.user_id " +
                     "LEFT JOIN messages m ON u.id = m.sender_id OR u.id = m.receiver_id " +
                     "WHERE p.created_at > ? OR c.created_at > ? OR pl.created_at > ? OR pf.created_at > ? OR m.created_at > ?";
        return jdbcTemplate.queryForObject(sql, Long.class, since, since, since, since, since);
    }
    
    public List<Map<String, Object>> getUserActivityDistribution() {
        String sql = "SELECT "+ 
                     "CASE " +
                     "WHEN last_activity IS NULL THEN 'Inactive' " +
                     "WHEN last_activity > NOW() - INTERVAL '1 day' THEN 'Very Active' " +
                     "WHEN last_activity > NOW() - INTERVAL '7 days' THEN 'Active' " +
                     "WHEN last_activity > NOW() - INTERVAL '30 days' THEN 'Occasional' " +
                     "ELSE 'Inactive' " +
                     "END as activity_level, " +
                     "COUNT(*) as user_count " +
                     "FROM ( " +
                     "SELECT u.id, MAX(GREATEST( " +
                     "COALESCE(p.created_at, '1970-01-01'), " +
                     "COALESCE(c.created_at, '1970-01-01'), " +
                     "COALESCE(pl.created_at, '1970-01-01'), " +
                     "COALESCE(pf.created_at, '1970-01-01'), " +
                     "COALESCE(m.created_at, '1970-01-01') " +
                     ")) as last_activity " +
                     "FROM users u " +
                     "LEFT JOIN posts p ON u.id = p.user_id " +
                     "LEFT JOIN comments c ON u.id = c.user_id " +
                     "LEFT JOIN post_likes pl ON u.id = pl.user_id " +
                     "LEFT JOIN post_favorites pf ON u.id = pf.user_id " +
                     "LEFT JOIN messages m ON u.id = m.sender_id OR u.id = m.receiver_id " +
                     "GROUP BY u.id " +
                     ") as user_activity " +
                     "GROUP BY activity_level " +
                     "ORDER BY user_count DESC";
        return jdbcTemplate.queryForList(sql);
    }
    
    public long countByCreatedAtBetween(LocalDateTime start, LocalDateTime end) {
        String sql = "SELECT COUNT(*) FROM users WHERE created_at BETWEEN ? AND ?";
        return jdbcTemplate.queryForObject(sql, Long.class, start, end);
    }
    
    public long countActiveUsersInPeriod(LocalDateTime start, LocalDateTime end) {
        String sql = "SELECT COUNT(DISTINCT u.id) FROM users u " +
                     "LEFT JOIN posts p ON u.id = p.user_id " +
                     "LEFT JOIN comments c ON u.id = c.user_id " +
                     "LEFT JOIN post_likes pl ON u.id = pl.user_id " +
                     "LEFT JOIN post_favorites pf ON u.id = pf.user_id " +
                     "LEFT JOIN messages m ON u.id = m.sender_id OR u.id = m.receiver_id " +
                     "WHERE (p.created_at BETWEEN ? AND ?) OR (c.created_at BETWEEN ? AND ?) OR " +
                     "(pl.created_at BETWEEN ? AND ?) OR (pf.created_at BETWEEN ? AND ?) OR (m.created_at BETWEEN ? AND ?)";
        return jdbcTemplate.queryForObject(sql, Long.class, start, end, start, end, start, end, start, end, start, end);
    }
    
    public List<Map<String, Object>> getUserActivityTrend(LocalDateTime start, LocalDateTime end) {
        String sql = "SELECT " +
                     "DATE_TRUNC('day', activity_date) as date, " +
                     "COUNT(DISTINCT user_id) as active_users " +
                     "FROM ( " +
                     "SELECT user_id, created_at as activity_date FROM posts " +
                     "UNION ALL " +
                     "SELECT user_id, created_at as activity_date FROM comments " +
                     "UNION ALL " +
                     "SELECT user_id, created_at as activity_date FROM post_likes " +
                     "UNION ALL " +
                     "SELECT user_id, created_at as activity_date FROM post_favorites " +
                     "UNION ALL " +
                     "SELECT sender_id as user_id, created_at as activity_date FROM messages " +
                     "UNION ALL " +
                     "SELECT receiver_id as user_id, created_at as activity_date FROM messages " +
                     ") as all_activities " +
                     "WHERE activity_date BETWEEN ? AND ? " +
                     "GROUP BY DATE_TRUNC('day', activity_date) " +
                     "ORDER BY date";
        return jdbcTemplate.queryForList(sql, start, end);
    }
}