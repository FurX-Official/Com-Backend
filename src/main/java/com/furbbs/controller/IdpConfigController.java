package com.furbbs.controller;

import com.furbbs.model.IdpConfig;
import com.furbbs.service.IdpConfigService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/idp-configs")
public class IdpConfigController {

    @Autowired
    private IdpConfigService idpConfigService;

    @GetMapping
    public ResponseEntity<List<IdpConfig>> getAllIdpConfigs() {
        List<IdpConfig> configs = idpConfigService.getAllIdpConfigs();
        return new ResponseEntity<>(configs, HttpStatus.OK);
    }

    @GetMapping("/{id}")
    public ResponseEntity<IdpConfig> getIdpConfigById(@PathVariable Long id) {
        return idpConfigService.getIdpConfigById(id)
            .map(config -> new ResponseEntity<>(config, HttpStatus.OK))
            .orElse(new ResponseEntity<>(HttpStatus.NOT_FOUND));
    }

    @GetMapping("/name/{name}")
    public ResponseEntity<IdpConfig> getIdpConfigByName(@PathVariable String name) {
        return idpConfigService.getIdpConfigByName(name)
            .map(config -> new ResponseEntity<>(config, HttpStatus.OK))
            .orElse(new ResponseEntity<>(HttpStatus.NOT_FOUND));
    }

    @GetMapping("/type/{type}")
    public ResponseEntity<List<IdpConfig>> getIdpConfigsByType(@PathVariable String type) {
        List<IdpConfig> configs = idpConfigService.getIdpConfigsByType(type);
        return new ResponseEntity<>(configs, HttpStatus.OK);
    }

    @GetMapping("/enabled")
    public ResponseEntity<List<IdpConfig>> getEnabledIdpConfigs() {
        List<IdpConfig> configs = idpConfigService.getEnabledIdpConfigs();
        return new ResponseEntity<>(configs, HttpStatus.OK);
    }

    @PostMapping
    public ResponseEntity<IdpConfig> createIdpConfig(@RequestBody IdpConfig idpConfig) {
        IdpConfig createdConfig = idpConfigService.createIdpConfig(idpConfig);
        return new ResponseEntity<>(createdConfig, HttpStatus.CREATED);
    }

    @PutMapping("/{id}")
    public ResponseEntity<IdpConfig> updateIdpConfig(@PathVariable Long id, @RequestBody IdpConfig idpConfig) {
        try {
            IdpConfig updatedConfig = idpConfigService.updateIdpConfig(id, idpConfig);
            return new ResponseEntity<>(updatedConfig, HttpStatus.OK);
        } catch (IllegalArgumentException e) {
            return new ResponseEntity<>(HttpStatus.NOT_FOUND);
        }
    }

    @DeleteMapping("/{id}")
    public ResponseEntity<?> deleteIdpConfig(@PathVariable Long id) {
        idpConfigService.deleteIdpConfig(id);
        return new ResponseEntity<>(HttpStatus.NO_CONTENT);
    }
}
