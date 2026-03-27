package com.furbbs.service;

import com.furbbs.model.ApiKey;

import java.util.List;

public interface ApiKeyService {
    ApiKey generateApiKey(Long userId, String name, java.time.LocalDateTime expiresAt);
    List<ApiKey> getApiKeysByUserId(Long userId);
    void revokeApiKey(Long id);
    void updateApiKeyStatus(Long id, boolean active);
    ApiKey getApiKeyById(Long id);
    ApiKey validateApiKey(String keyValue);
}