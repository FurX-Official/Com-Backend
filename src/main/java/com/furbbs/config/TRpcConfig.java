package com.furbbs.config;

import com.furbbs.service.UserService;
import com.furbbs.service.impl.UserServiceImpl;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
public class TRpcConfig {
    @Bean
    public UserService userService() {
        return new UserServiceImpl();
    }
}