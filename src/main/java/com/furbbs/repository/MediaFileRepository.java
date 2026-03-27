package com.furbbs.repository;

import com.furbbs.model.MediaFile;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;
import java.util.List;
import java.util.Optional;

@Repository
public class MediaFileRepository {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    private final RowMapper<MediaFile> mediaFileRowMapper = new RowMapper<MediaFile>() {
        @Override
        public MediaFile mapRow(ResultSet rs, int rowNum) throws SQLException {
            MediaFile mediaFile = new MediaFile();
            mediaFile.setId(rs.getLong("id"));
            mediaFile.setFilename(rs.getString("filename"));
            mediaFile.setFilepath(rs.getString("filepath"));
            mediaFile.setFiletype(rs.getString("filetype"));
            mediaFile.setFilesize(rs.getLong("filesize"));
            mediaFile.setUserId(rs.getLong("user_id"));
            mediaFile.setPostId(rs.getLong("post_id"));
            mediaFile.setCreatedAt(rs.getTimestamp("created_at"));
            return mediaFile;
        }
    };

    public MediaFile save(MediaFile mediaFile) {
        String sql = "INSERT INTO media_files (filename, filepath, filetype, filesize, user_id, post_id, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)";
        Date now = new Date();
        jdbcTemplate.update(sql, mediaFile.getFilename(), mediaFile.getFilepath(), mediaFile.getFiletype(),
            mediaFile.getFilesize(), mediaFile.getUserId(), mediaFile.getPostId(), now);
        // Get the generated id
        Long id = jdbcTemplate.queryForObject("SELECT lastval()", Long.class);
        mediaFile.setId(id);
        mediaFile.setCreatedAt(now);
        return mediaFile;
    }

    public Optional<MediaFile> findById(Long id) {
        String sql = "SELECT * FROM media_files WHERE id = ?";
        try {
            MediaFile mediaFile = jdbcTemplate.queryForObject(sql, mediaFileRowMapper, id);
            return Optional.of(mediaFile);
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    public List<MediaFile> findByUserId(Long userId) {
        String sql = "SELECT * FROM media_files WHERE user_id = ? ORDER BY created_at DESC";
        return jdbcTemplate.query(sql, mediaFileRowMapper, userId);
    }

    public List<MediaFile> findByPostId(Long postId) {
        String sql = "SELECT * FROM media_files WHERE post_id = ? ORDER BY created_at DESC";
        return jdbcTemplate.query(sql, mediaFileRowMapper, postId);
    }

    public void deleteById(Long id) {
        String sql = "DELETE FROM media_files WHERE id = ?";
        jdbcTemplate.update(sql, id);
    }

    public void deleteByPostId(Long postId) {
        String sql = "DELETE FROM media_files WHERE post_id = ?";
        jdbcTemplate.update(sql, postId);
    }
}