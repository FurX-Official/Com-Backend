package com.furbbs.service.impl;

import com.furbbs.model.IdpConfig;
import com.furbbs.repository.IdpConfigRepository;
import com.furbbs.service.IdpConfigService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;

@Service
public class IdpConfigServiceImpl implements IdpConfigService {

    @Autowired
    private IdpConfigRepository idpConfigRepository;

    @Override
    public List<IdpConfig> getAllIdpConfigs() {
        return idpConfigRepository.findAll();
    }

    @Override
    public Optional<IdpConfig> getIdpConfigById(Long id) {
        return idpConfigRepository.findById(id);
    }

    @Override
    public Optional<IdpConfig> getIdpConfigByName(String name) {
        return idpConfigRepository.findByName(name);
    }

    @Override
    public List<IdpConfig> getIdpConfigsByType(String type) {
        return idpConfigRepository.findByType(type);
    }

    @Override
    public List<IdpConfig> getEnabledIdpConfigs() {
        return idpConfigRepository.findByEnabled(true);
    }

    @Override
    public IdpConfig createIdpConfig(IdpConfig idpConfig) {
        return idpConfigRepository.save(idpConfig);
    }

    @Override
    public IdpConfig updateIdpConfig(Long id, IdpConfig idpConfig) {
        Optional<IdpConfig> existingConfig = idpConfigRepository.findById(id);
        if (existingConfig.isPresent()) {
            idpConfig.setId(id);
            return idpConfigRepository.save(idpConfig);
        } else {
            throw new IllegalArgumentException("IDP config not found");
        }
    }

    @Override
    public void deleteIdpConfig(Long id) {
        idpConfigRepository.deleteById(id);
    }
}
