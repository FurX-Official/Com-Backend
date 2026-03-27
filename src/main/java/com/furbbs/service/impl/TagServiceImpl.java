package com.furbbs.service.impl;

import com.furbbs.model.Tag;
import com.furbbs.repository.TagRepository;
import com.furbbs.service.TagService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;

@Service
public class TagServiceImpl implements TagService {

    @Autowired
    private TagRepository tagRepository;

    @Override
    public Tag createTag(Tag tag) {
        return tagRepository.save(tag);
    }

    @Override
    public Tag updateTag(Tag tag) {
        return tagRepository.save(tag);
    }

    @Override
    public Optional<Tag> getTagById(Long id) {
        return tagRepository.findById(id);
    }

    @Override
    public Optional<Tag> getTagByName(String name) {
        return tagRepository.findByName(name);
    }

    @Override
    public List<Tag> getAllTags() {
        return tagRepository.findAll();
    }

    @Override
    public List<Tag> getTagsByPostId(Long postId) {
        return tagRepository.findByPostId(postId);
    }

    @Override
    public void deleteTag(Long id) {
        tagRepository.deleteById(id);
    }

    @Override
    public void addTagToPost(Long postId, Long tagId) {
        tagRepository.addTagToPost(postId, tagId);
    }

    @Override
    public void removeTagFromPost(Long postId, Long tagId) {
        tagRepository.removeTagFromPost(postId, tagId);
    }

    @Override
    public void removeAllTagsFromPost(Long postId) {
        tagRepository.removeAllTagsFromPost(postId);
    }
}