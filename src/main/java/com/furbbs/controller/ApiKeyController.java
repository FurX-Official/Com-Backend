package com.furbbs.controller;

import com.furbbs.model.ApiKey;
import com.furbbs.service.ApiKeyService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.time.LocalDateTime;
import java.util.List;

@RestController
@RequestMapping("/api/api-keys")
public class ApiKeyController {

    @Autowired
    private ApiKeyService apiKeyService;

    @PostMapping("/generate")
    public ResponseEntity<ApiKey> generateApiKey(Authentication authentication, @RequestParam String name, @RequestParam(required = false) String expiresAt) {
        Long userId = Long.parseLong(authentication.getName());
        LocalDateTime expiration = null;
        if (expiresAt != null) {
            expiration = LocalDateTime.parse(expiresAt);
        }
        ApiKey apiKey = apiKeyService.generateApiKey(userId, name, expiration);
        return ResponseEntity.status(HttpStatus.CREATED).body(apiKey);
    }

    @GetMapping
    public ResponseEntity<List<ApiKey>> getApiKeys(Authentication authentication) {
        Long userId = Long.parseLong(authentication.getName());
        List<ApiKey> apiKeys = apiKeyService.getApiKeysByUserId(userId);
        return ResponseEntity.ok(apiKeys);
    }

    @PutMapping("/{id}/revoke")
    public ResponseEntity<Void> revokeApiKey(Authentication authentication, @PathVariable Long id) {
        Long userId = Long.parseLong(authentication.getName());
        ApiKey apiKey = apiKeyService.getApiKeyById(id);
        if (!apiKey.getUserId().equals(userId)) {
            return ResponseEntity.status(HttpStatus.FORBIDDEN).build();
        }
        apiKeyService.revokeApiKey(id);
        return ResponseEntity.ok().build();
    }

    @PutMapping("/{id}/status")
    public ResponseEntity<Void> updateApiKeyStatus(Authentication authentication, @PathVariable Long id, @RequestParam boolean active) {
        Long userId = Long.parseLong(authentication.getName());
        ApiKey apiKey = apiKeyService.getApiKeyById(id);
        if (!apiKey.getUserId().equals(userId)) {
            return ResponseEntity.status(HttpStatus.FORBIDDEN).build();
        }
        apiKeyService.updateApiKeyStatus(id, active);
        return ResponseEntity.ok().build();
    }
}