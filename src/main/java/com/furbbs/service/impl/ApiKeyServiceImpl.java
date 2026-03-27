package com.furbbs.service.impl;

import com.furbbs.model.ApiKey;
import com.furbbs.repository.ApiKeyRepository;
import com.furbbs.service.ApiKeyService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.time.LocalDateTime;
import java.util.List;
import java.util.UUID;

@Service
public class ApiKeyServiceImpl implements ApiKeyService {

    @Autowired
    private ApiKeyRepository apiKeyRepository;

    @Override
    public ApiKey generateApiKey(Long userId, String name, LocalDateTime expiresAt) {
        String rawKey = UUID.randomUUID().toString() + "-" + System.currentTimeMillis();
        String hashedKey = hashApiKey(rawKey);

        ApiKey apiKey = new ApiKey();
        apiKey.setKeyValue(hashedKey);
        apiKey.setUserId(userId);
        apiKey.setName(name);
        apiKey.setCreatedAt(LocalDateTime.now());
        apiKey.setExpiresAt(expiresAt);
        apiKey.setActive(true);

        return apiKeyRepository.save(apiKey);
    }

    @Override
    public List<ApiKey> getApiKeysByUserId(Long userId) {
        return apiKeyRepository.findByUserId(userId);
    }

    @Override
    public void revokeApiKey(Long id) {
        ApiKey apiKey = apiKeyRepository.findById(id).orElseThrow(() -> new RuntimeException("API Key not found"));
        apiKey.setActive(false);
        apiKeyRepository.save(apiKey);
    }

    @Override
    public void updateApiKeyStatus(Long id, boolean active) {
        ApiKey apiKey = apiKeyRepository.findById(id).orElseThrow(() -> new RuntimeException("API Key not found"));
        apiKey.setActive(active);
        apiKeyRepository.save(apiKey);
    }

    @Override
    public ApiKey getApiKeyById(Long id) {
        return apiKeyRepository.findById(id).orElseThrow(() -> new RuntimeException("API Key not found"));
    }

    @Override
    public ApiKey validateApiKey(String keyValue) {
        String hashedKey = hashApiKey(keyValue);
        ApiKey apiKey = apiKeyRepository.findByKeyValue(hashedKey).orElse(null);

        if (apiKey == null || !apiKey.isActive()) {
            return null;
        }

        if (apiKey.getExpiresAt() != null && apiKey.getExpiresAt().isBefore(LocalDateTime.now())) {
            return null;
        }

        return apiKey;
    }

    private String hashApiKey(String rawKey) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(rawKey.getBytes(StandardCharsets.UTF_8));
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing API key", e);
        }
    }
}