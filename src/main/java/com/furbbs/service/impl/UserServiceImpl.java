package com.furbbs.service.impl;

import com.furbbs.service.UserService;

public class UserServiceImpl implements UserService {
    @Override
    public String getUserInfo(String userId) {
        return "User info for ID: " + userId;
    }

    @Override
    public String createUser(String username, String email) {
        return "Created user: " + username + " with email: " + email;
    }
}