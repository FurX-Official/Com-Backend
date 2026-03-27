package com.furbbs.repository;

import com.furbbs.model.IdpConfig;

import java.util.List;
import java.util.Optional;

public interface IdpConfigRepository {
    List<IdpConfig> findAll();
    Optional<IdpConfig> findById(Long id);
    Optional<IdpConfig> findByName(String name);
    List<IdpConfig> findByType(String type);
    List<IdpConfig> findByEnabled(boolean enabled);
    IdpConfig save(IdpConfig idpConfig);
    void deleteById(Long id);
}
