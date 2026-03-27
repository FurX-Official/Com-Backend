package com.furbbs.service;

import com.furbbs.model.User;
import com.furbbs.repository.UserRepository;
import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.SignatureAlgorithm;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

@Service
public class AuthService {

    @Autowired
    private UserRepository userRepository;

    private final BCryptPasswordEncoder passwordEncoder = new BCryptPasswordEncoder();
    private final String JWT_SECRET = "your-secret-key";
    private final long JWT_EXPIRATION = 86400000; // 24 hours

    public User register(String username, String email, String password) {
        // Check if username already exists
        if (userRepository.findByUsername(username).isPresent()) {
            throw new RuntimeException("Username already exists");
        }

        // Check if email already exists
        if (userRepository.findByEmail(email).isPresent()) {
            throw new RuntimeException("Email already exists");
        }

        // Create new user
        User user = new User();
        user.setUsername(username);
        user.setEmail(email);
        user.setPassword(passwordEncoder.encode(password));
        user.setNickname(username); // Default nickname is username

        return userRepository.save(user);
    }

    public Map<String, Object> login(String username, String password) {
        // Find user by username
        Optional<User> optionalUser = userRepository.findByUsername(username);
        if (!optionalUser.isPresent()) {
            throw new RuntimeException("Invalid username or password");
        }

        User user = optionalUser.get();

        // Check password
        if (!passwordEncoder.matches(password, user.getPassword())) {
            throw new RuntimeException("Invalid username or password");
        }

        // Generate JWT token
        String token = generateToken(user);

        // Return user info and token
        Map<String, Object> response = new HashMap<>();
        response.put("user", user);
        response.put("token", token);

        return response;
    }

    public void logout(String token) {
        // In a production environment, you might want to add the token to a blacklist
        // For simplicity, we'll just return success
    }

    public User updateProfile(Long userId, String nickname, String avatar, String bio) {
        Optional<User> optionalUser = userRepository.findById(userId);
        if (!optionalUser.isPresent()) {
            throw new RuntimeException("User not found");
        }

        User user = optionalUser.get();
        user.setNickname(nickname);
        user.setAvatar(avatar);
        user.setBio(bio);

        return userRepository.save(user);
    }

    public void resetPasswordRequest(String email) {
        Optional<User> optionalUser = userRepository.findByEmail(email);
        if (!optionalUser.isPresent()) {
            throw new RuntimeException("Email not found");
        }

        // Generate reset token
        String resetToken = UUID.randomUUID().toString();
        Date expiry = new Date(System.currentTimeMillis() + 3600000); // 1 hour expiry

        // Update user with reset token
        userRepository.updateResetToken(email, resetToken, expiry);

        // In a production environment, you would send an email with the reset token
        // For simplicity, we'll just return success
    }

    public void resetPassword(String resetToken, String newPassword) {
        Optional<User> optionalUser = userRepository.findByResetToken(resetToken);
        if (!optionalUser.isPresent()) {
            throw new RuntimeException("Invalid or expired reset token");
        }

        User user = optionalUser.get();
        user.setPassword(passwordEncoder.encode(newPassword));
        user.setResetToken(null);
        user.setResetTokenExpiry(null);

        userRepository.save(user);
    }

    public User getCurrentUser(String token) {
        // Parse token and get user id
        String username = parseToken(token);
        Optional<User> optionalUser = userRepository.findByUsername(username);
        if (!optionalUser.isPresent()) {
            throw new RuntimeException("User not found");
        }

        return optionalUser.get();
    }

    public Map<String, Object> oauth2Login(String provider, String email, String name, String avatar) {
        // Find user by email
        Optional<User> optionalUser = userRepository.findByEmail(email);
        User user;

        if (optionalUser.isPresent()) {
            // Update existing user
            user = optionalUser.get();
            user.setNickname(name);
            user.setAvatar(avatar);
        } else {
            // Create new user
            user = new User();
            user.setUsername(email.split("@")[0]);
            user.setEmail(email);
            user.setPassword(passwordEncoder.encode(UUID.randomUUID().toString())); // Generate random password
            user.setNickname(name);
            user.setAvatar(avatar);
        }

        userRepository.save(user);

        // Generate JWT token
        String token = generateToken(user);

        // Return user info and token
        Map<String, Object> response = new HashMap<>();
        response.put("user", user);
        response.put("token", token);

        return response;
    }

    public Map<String, Object> samlLogin(Map<String, Object> attributes) {
        // Extract user information from SAML attributes
        String email = (String) attributes.get("email");
        String name = (String) attributes.get("name");
        String avatar = (String) attributes.get("avatar");

        // Find user by email
        Optional<User> optionalUser = userRepository.findByEmail(email);
        User user;

        if (optionalUser.isPresent()) {
            // Update existing user
            user = optionalUser.get();
            user.setNickname(name);
            user.setAvatar(avatar);
        } else {
            // Create new user
            user = new User();
            user.setUsername(email.split("@")[0]);
            user.setEmail(email);
            user.setPassword(passwordEncoder.encode(UUID.randomUUID().toString())); // Generate random password
            user.setNickname(name);
            user.setAvatar(avatar);
        }

        userRepository.save(user);

        // Generate JWT token
        String token = generateToken(user);

        // Return user info and token
        Map<String, Object> response = new HashMap<>();
        response.put("user", user);
        response.put("token", token);

        return response;
    }

    private String generateToken(User user) {
        Date now = new Date();
        Date expiryDate = new Date(now.getTime() + JWT_EXPIRATION);

        return Jwts.builder()
                .setSubject(user.getUsername())
                .setIssuedAt(now)
                .setExpiration(expiryDate)
                .signWith(SignatureAlgorithm.HS512, JWT_SECRET)
                .compact();
    }

    private String parseToken(String token) {
        return Jwts.parser()
                .setSigningKey(JWT_SECRET)
                .parseClaimsJws(token)
                .getBody()
                .getSubject();
    }
}