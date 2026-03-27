package com.furbbs.controller;

import com.furbbs.model.Tag;
import com.furbbs.service.TagService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/tags")
public class TagController {

    @Autowired
    private TagService tagService;

    @PostMapping
    public ResponseEntity<Tag> createTag(@RequestBody Tag tag) {
        Tag createdTag = tagService.createTag(tag);
        return new ResponseEntity<>(createdTag, HttpStatus.CREATED);
    }

    @PutMapping("/{id}")
    public ResponseEntity<Tag> updateTag(@PathVariable Long id, @RequestBody Tag tag) {
        tag.setId(id);
        Tag updatedTag = tagService.updateTag(tag);
        return new ResponseEntity<>(updatedTag, HttpStatus.OK);
    }

    @GetMapping("/{id}")
    public ResponseEntity<Tag> getTagById(@PathVariable Long id) {
        return tagService.getTagById(id)
                .map(tag -> new ResponseEntity<>(tag, HttpStatus.OK))
                .orElse(new ResponseEntity<>(HttpStatus.NOT_FOUND));
    }

    @GetMapping
    public ResponseEntity<List<Tag>> getAllTags() {
        List<Tag> tags = tagService.getAllTags();
        return new ResponseEntity<>(tags, HttpStatus.OK);
    }

    @GetMapping("/post/{postId}")
    public ResponseEntity<List<Tag>> getTagsByPostId(@PathVariable Long postId) {
        List<Tag> tags = tagService.getTagsByPostId(postId);
        return new ResponseEntity<>(tags, HttpStatus.OK);
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deleteTag(@PathVariable Long id) {
        tagService.deleteTag(id);
        return new ResponseEntity<>(HttpStatus.NO_CONTENT);
    }

    @PostMapping("/post/{postId}/add/{tagId}")
    public ResponseEntity<Void> addTagToPost(@PathVariable Long postId, @PathVariable Long tagId) {
        tagService.addTagToPost(postId, tagId);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @DeleteMapping("/post/{postId}/remove/{tagId}")
    public ResponseEntity<Void> removeTagFromPost(@PathVariable Long postId, @PathVariable Long tagId) {
        tagService.removeTagFromPost(postId, tagId);
        return new ResponseEntity<>(HttpStatus.OK);
    }

    @DeleteMapping("/post/{postId}/clear")
    public ResponseEntity<Void> removeAllTagsFromPost(@PathVariable Long postId) {
        tagService.removeAllTagsFromPost(postId);
        return new ResponseEntity<>(HttpStatus.OK);
    }
}