package com.furbbs.repository;

import com.furbbs.model.PostFavorite;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;

@Repository
public interface PostFavoriteRepository extends JpaRepository<PostFavorite, Long> {
    Optional<PostFavorite> findByUserIdAndPostId(Long userId, Long postId);
    List<PostFavorite> findByUserId(Long userId);
    boolean existsByUserIdAndPostId(Long userId, Long postId);
    
    long countByCreatedAtBetween(LocalDateTime start, LocalDateTime end);
}
