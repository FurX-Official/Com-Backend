package com.furbbs.service;

import com.furbbs.model.MediaFile;
import org.springframework.web.multipart.MultipartFile;

import java.util.List;
import java.util.Optional;

public interface MediaService {
    MediaFile uploadFile(MultipartFile file, Long userId, Long postId);
    Optional<MediaFile> getMediaFileById(Long id);
    List<MediaFile> getMediaFilesByUserId(Long userId);
    List<MediaFile> getMediaFilesByPostId(Long postId);
    void deleteMediaFile(Long id);
}