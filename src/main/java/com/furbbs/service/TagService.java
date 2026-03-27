package com.furbbs.service;

import com.furbbs.model.Tag;
import java.util.List;
import java.util.Optional;

public interface TagService {
    Tag createTag(Tag tag);
    Tag updateTag(Tag tag);
    Optional<Tag> getTagById(Long id);
    Optional<Tag> getTagByName(String name);
    List<Tag> getAllTags();
    List<Tag> getTagsByPostId(Long postId);
    void deleteTag(Long id);
    void addTagToPost(Long postId, Long tagId);
    void removeTagFromPost(Long postId, Long tagId);
    void removeAllTagsFromPost(Long postId);
}