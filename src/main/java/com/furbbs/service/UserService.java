package com.furbbs.service;

public interface UserService {
    String getUserInfo(String userId);
    String createUser(String username, String email);
}