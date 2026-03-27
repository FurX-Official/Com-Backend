package com.furbbs.repository.impl;

import com.furbbs.model.IdpConfig;
import com.furbbs.repository.IdpConfigRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;

@Repository
public class IdpConfigRepositoryImpl implements IdpConfigRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private RowMapper<IdpConfig> rowMapper = new RowMapper<IdpConfig>() {
        @Override
        public IdpConfig mapRow(ResultSet rs, int rowNum) throws SQLException {
            IdpConfig idpConfig = new IdpConfig();
            idpConfig.setId(rs.getLong("id"));
            idpConfig.setName(rs.getString("name"));
            idpConfig.setType(rs.getString("type"));
            idpConfig.setClientId(rs.getString("client_id"));
            idpConfig.setClientSecret(rs.getString("client_secret"));
            idpConfig.setAuthorizationUri(rs.getString("authorization_uri"));
            idpConfig.setTokenUri(rs.getString("token_uri"));
            idpConfig.setUserInfoUri(rs.getString("user_info_uri"));
            idpConfig.setRedirectUri(rs.getString("redirect_uri"));
            idpConfig.setScope(rs.getString("scope"));
            idpConfig.setEntityId(rs.getString("entity_id"));
            idpConfig.setSsoUrl(rs.getString("sso_url"));
            idpConfig.setMetadataUrl(rs.getString("metadata_url"));
            idpConfig.setEnabled(rs.getBoolean("enabled"));
            idpConfig.setCreatedAt(rs.getObject("created_at", LocalDateTime.class));
            idpConfig.setUpdatedAt(rs.getObject("updated_at", LocalDateTime.class));
            return idpConfig;
        }
    };

    @Override
    public List<IdpConfig> findAll() {
        String sql = "SELECT * FROM idp_configs";
        return jdbcTemplate.query(sql, rowMapper);
    }

    @Override
    public Optional<IdpConfig> findById(Long id) {
        String sql = "SELECT * FROM idp_configs WHERE id = ?";
        List<IdpConfig> result = jdbcTemplate.query(sql, rowMapper, id);
        return result.isEmpty() ? Optional.empty() : Optional.of(result.get(0));
    }

    @Override
    public Optional<IdpConfig> findByName(String name) {
        String sql = "SELECT * FROM idp_configs WHERE name = ?";
        List<IdpConfig> result = jdbcTemplate.query(sql, rowMapper, name);
        return result.isEmpty() ? Optional.empty() : Optional.of(result.get(0));
    }

    @Override
    public List<IdpConfig> findByType(String type) {
        String sql = "SELECT * FROM idp_configs WHERE type = ?";
        return jdbcTemplate.query(sql, rowMapper, type);
    }

    @Override
    public List<IdpConfig> findByEnabled(boolean enabled) {
        String sql = "SELECT * FROM idp_configs WHERE enabled = ?";
        return jdbcTemplate.query(sql, rowMapper, enabled);
    }

    @Override
    public IdpConfig save(IdpConfig idpConfig) {
        if (idpConfig.getId() == null) {
            // Insert new record
            String sql = "INSERT INTO idp_configs (name, type, client_id, client_secret, authorization_uri, token_uri, user_info_uri, redirect_uri, scope, entity_id, sso_url, metadata_url, enabled, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";
            jdbcTemplate.update(sql, 
                idpConfig.getName(),
                idpConfig.getType(),
                idpConfig.getClientId(),
                idpConfig.getClientSecret(),
                idpConfig.getAuthorizationUri(),
                idpConfig.getTokenUri(),
                idpConfig.getUserInfoUri(),
                idpConfig.getRedirectUri(),
                idpConfig.getScope(),
                idpConfig.getEntityId(),
                idpConfig.getSsoUrl(),
                idpConfig.getMetadataUrl(),
                idpConfig.isEnabled()
            );
        } else {
            // Update existing record
            String sql = "UPDATE idp_configs SET name = ?, type = ?, client_id = ?, client_secret = ?, authorization_uri = ?, token_uri = ?, user_info_uri = ?, redirect_uri = ?, scope = ?, entity_id = ?, sso_url = ?, metadata_url = ?, enabled = ?, updated_at = NOW() WHERE id = ?";
            jdbcTemplate.update(sql, 
                idpConfig.getName(),
                idpConfig.getType(),
                idpConfig.getClientId(),
                idpConfig.getClientSecret(),
                idpConfig.getAuthorizationUri(),
                idpConfig.getTokenUri(),
                idpConfig.getUserInfoUri(),
                idpConfig.getRedirectUri(),
                idpConfig.getScope(),
                idpConfig.getEntityId(),
                idpConfig.getSsoUrl(),
                idpConfig.getMetadataUrl(),
                idpConfig.isEnabled(),
                idpConfig.getId()
            );
        }
        return idpConfig;
    }

    @Override
    public void deleteById(Long id) {
        String sql = "DELETE FROM idp_configs WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }
}
