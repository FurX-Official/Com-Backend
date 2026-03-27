package com.furbbs.service.impl;

import com.furbbs.model.MediaFile;
import com.furbbs.repository.MediaFileRepository;
import com.furbbs.service.MediaService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

@Service
public class MediaServiceImpl implements MediaService {

    @Autowired
    private MediaFileRepository mediaFileRepository;

    private final String UPLOAD_DIR = "uploads/";

    public MediaServiceImpl() {
        // Create upload directory if it doesn't exist
        File uploadDir = new File(UPLOAD_DIR);
        if (!uploadDir.exists()) {
            uploadDir.mkdirs();
        }
    }

    @Override
    public MediaFile uploadFile(MultipartFile file, Long userId, Long postId) {
        try {
            // Generate unique filename
            String originalFilename = file.getOriginalFilename();
            String extension = originalFilename.substring(originalFilename.lastIndexOf("."));
            String uniqueFilename = UUID.randomUUID().toString() + extension;
            
            // Create file path
            String filepath = UPLOAD_DIR + uniqueFilename;
            
            // Save file to disk
            file.transferTo(new File(filepath));
            
            // Create media file record
            MediaFile mediaFile = new MediaFile();
            mediaFile.setFilename(originalFilename);
            mediaFile.setFilepath(filepath);
            mediaFile.setFiletype(file.getContentType());
            mediaFile.setFilesize(file.getSize());
            mediaFile.setUserId(userId);
            mediaFile.setPostId(postId);
            
            // Save to database
            return mediaFileRepository.save(mediaFile);
        } catch (IOException e) {
            throw new RuntimeException("Failed to upload file", e);
        }
    }

    @Override
    public Optional<MediaFile> getMediaFileById(Long id) {
        return mediaFileRepository.findById(id);
    }

    @Override
    public List<MediaFile> getMediaFilesByUserId(Long userId) {
        return mediaFileRepository.findByUserId(userId);
    }

    @Override
    public List<MediaFile> getMediaFilesByPostId(Long postId) {
        return mediaFileRepository.findByPostId(postId);
    }

    @Override
    public void deleteMediaFile(Long id) {
        Optional<MediaFile> mediaFileOptional = mediaFileRepository.findById(id);
        if (mediaFileOptional.isPresent()) {
            MediaFile mediaFile = mediaFileOptional.get();
            // Delete file from disk
            File file = new File(mediaFile.getFilepath());
            if (file.exists()) {
                file.delete();
            }
            // Delete from database
            mediaFileRepository.deleteById(id);
        }
    }
}