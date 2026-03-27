package com.furbbs.controller;

import com.furbbs.model.MediaFile;
import com.furbbs.service.MediaService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;

import java.util.List;

@RestController
@RequestMapping("/api/media")
public class MediaController {

    @Autowired
    private MediaService mediaService;

    @PostMapping("/upload")
    public ResponseEntity<MediaFile> uploadFile(
            @RequestParam("file") MultipartFile file,
            @RequestParam("userId") Long userId,
            @RequestParam(value = "postId", required = false) Long postId) {
        MediaFile uploadedFile = mediaService.uploadFile(file, userId, postId);
        return new ResponseEntity<>(uploadedFile, HttpStatus.CREATED);
    }

    @GetMapping("/{id}")
    public ResponseEntity<MediaFile> getMediaFileById(@PathVariable Long id) {
        return mediaService.getMediaFileById(id)
                .map(mediaFile -> new ResponseEntity<>(mediaFile, HttpStatus.OK))
                .orElse(new ResponseEntity<>(HttpStatus.NOT_FOUND));
    }

    @GetMapping("/user/{userId}")
    public ResponseEntity<List<MediaFile>> getMediaFilesByUserId(@PathVariable Long userId) {
        List<MediaFile> mediaFiles = mediaService.getMediaFilesByUserId(userId);
        return new ResponseEntity<>(mediaFiles, HttpStatus.OK);
    }

    @GetMapping("/post/{postId}")
    public ResponseEntity<List<MediaFile>> getMediaFilesByPostId(@PathVariable Long postId) {
        List<MediaFile> mediaFiles = mediaService.getMediaFilesByPostId(postId);
        return new ResponseEntity<>(mediaFiles, HttpStatus.OK);
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deleteMediaFile(@PathVariable Long id) {
        mediaService.deleteMediaFile(id);
        return new ResponseEntity<>(HttpStatus.NO_CONTENT);
    }
}