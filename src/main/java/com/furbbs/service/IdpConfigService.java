package com.furbbs.service;

import com.furbbs.model.IdpConfig;

import java.util.List;
import java.util.Optional;

public interface IdpConfigService {
    List<IdpConfig> getAllIdpConfigs();
    Optional<IdpConfig> getIdpConfigById(Long id);
    Optional<IdpConfig> getIdpConfigByName(String name);
    List<IdpConfig> getIdpConfigsByType(String type);
    List<IdpConfig> getEnabledIdpConfigs();
    IdpConfig createIdpConfig(IdpConfig idpConfig);
    IdpConfig updateIdpConfig(Long id, IdpConfig idpConfig);
    void deleteIdpConfig(Long id);
}
