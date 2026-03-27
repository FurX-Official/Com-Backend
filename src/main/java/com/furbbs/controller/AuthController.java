package com.furbbs.controller;

import com.furbbs.model.User;
import com.furbbs.service.AuthService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.security.oauth2.client.OAuth2AuthorizedClient;
import org.springframework.security.oauth2.client.annotation.RegisteredOAuth2AuthorizedClient;
import org.springframework.security.oauth2.core.user.OAuth2User;
import org.springframework.web.bind.annotation.*;

import java.util.Map;

@RestController
@RequestMapping("/api/auth")
public class AuthController {

    @Autowired
    private AuthService authService;

    @PostMapping("/register")
    public ResponseEntity<?> register(@RequestBody Map<String, String> request) {
        try {
            String username = request.get("username");
            String email = request.get("email");
            String password = request.get("password");

            User user = authService.register(username, email, password);
            return new ResponseEntity<>(user, HttpStatus.CREATED);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.BAD_REQUEST);
        }
    }

    @PostMapping("/login")
    public ResponseEntity<?> login(@RequestBody Map<String, String> request) {
        try {
            String username = request.get("username");
            String password = request.get("password");

            Map<String, Object> response = authService.login(username, password);
            return new ResponseEntity<>(response, HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.UNAUTHORIZED);
        }
    }

    @PostMapping("/logout")
    public ResponseEntity<?> logout(@RequestHeader("Authorization") String token) {
        try {
            // Remove "Bearer " prefix from token
            if (token.startsWith("Bearer ")) {
                token = token.substring(7);
            }

            authService.logout(token);
            return new ResponseEntity<>("Logged out successfully", HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.BAD_REQUEST);
        }
    }

    @PutMapping("/profile")
    public ResponseEntity<?> updateProfile(@RequestHeader("Authorization") String token, @RequestBody Map<String, String> request) {
        try {
            // Remove "Bearer " prefix from token
            if (token.startsWith("Bearer ")) {
                token = token.substring(7);
            }

            User currentUser = authService.getCurrentUser(token);
            String nickname = request.get("nickname");
            String avatar = request.get("avatar");
            String bio = request.get("bio");

            User updatedUser = authService.updateProfile(currentUser.getId(), nickname, avatar, bio);
            return new ResponseEntity<>(updatedUser, HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.BAD_REQUEST);
        }
    }

    @PostMapping("/reset-password/request")
    public ResponseEntity<?> resetPasswordRequest(@RequestBody Map<String, String> request) {
        try {
            String email = request.get("email");
            authService.resetPasswordRequest(email);
            return new ResponseEntity<>("Reset password email sent", HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.BAD_REQUEST);
        }
    }

    @PostMapping("/reset-password")
    public ResponseEntity<?> resetPassword(@RequestBody Map<String, String> request) {
        try {
            String resetToken = request.get("resetToken");
            String newPassword = request.get("newPassword");
            authService.resetPassword(resetToken, newPassword);
            return new ResponseEntity<>("Password reset successfully", HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.BAD_REQUEST);
        }
    }

    @GetMapping("/me")
    public ResponseEntity<?> getCurrentUser(@RequestHeader("Authorization") String token) {
        try {
            // Remove "Bearer " prefix from token
            if (token.startsWith("Bearer ")) {
                token = token.substring(7);
            }

            User user = authService.getCurrentUser(token);
            return new ResponseEntity<>(user, HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.UNAUTHORIZED);
        }
    }

    // OAuth2 endpoints
    @GetMapping("/oauth2/login/{provider}")
    public ResponseEntity<?> oauth2Login(@PathVariable String provider) {
        // Redirect to OAuth2 provider
        String redirectUrl = "https://furbbs.com/api/auth/oauth2/callback/" + provider;
        return new ResponseEntity<>(Map.of("redirectUrl", redirectUrl), HttpStatus.OK);
    }

    @GetMapping("/oauth2/callback/{provider}")
    public ResponseEntity<?> oauth2Callback(@PathVariable String provider, Authentication authentication) {
        try {
            OAuth2User oauth2User = (OAuth2User) authentication.getPrincipal();
            Map<String, Object> attributes = oauth2User.getAttributes();

            // Process OAuth2 user information
            String email = (String) attributes.get("email");
            String name = (String) attributes.get("name");
            String avatar = (String) attributes.get("picture");

            // Create or update user and generate JWT
            Map<String, Object> response = authService.oauth2Login(provider, email, name, avatar);
            return new ResponseEntity<>(response, HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.UNAUTHORIZED);
        }
    }

    // SAML endpoints
    @GetMapping("/saml/login")
    public ResponseEntity<?> samlLogin() {
        // Redirect to SAML IDP
        String redirectUrl = "https://furbbs.com/api/auth/saml/callback";
        return new ResponseEntity<>(Map.of("redirectUrl", redirectUrl), HttpStatus.OK);
    }

    @PostMapping("/saml/callback")
    public ResponseEntity<?> samlCallback(Authentication authentication) {
        try {
            // Process SAML authentication
            Map<String, Object> attributes = authentication.getPrincipal().toString().equals("anonymousUser") 
                ? Map.of() 
                : (Map<String, Object>) authentication.getPrincipal();

            // Create or update user and generate JWT
            Map<String, Object> response = authService.samlLogin(attributes);
            return new ResponseEntity<>(response, HttpStatus.OK);
        } catch (Exception e) {
            return new ResponseEntity<>(e.getMessage(), HttpStatus.UNAUTHORIZED);
        }
    }
}