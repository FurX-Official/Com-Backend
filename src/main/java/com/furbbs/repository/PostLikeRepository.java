package com.furbbs.repository;

import com.furbbs.model.PostLike;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.Optional;

@Repository
public interface PostLikeRepository extends JpaRepository<PostLike, Long> {
    Optional<PostLike> findByUserIdAndPostId(Long userId, Long postId);
    long countByPostId(Long postId);
    boolean existsByUserIdAndPostId(Long userId, Long postId);
    
    long countByCreatedAtBetween(LocalDateTime start, LocalDateTime end);
}
