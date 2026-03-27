package com.furbbs.config;

import com.furbbs.repository.UserRepository;
import com.furbbs.service.PermissionService;
import io.jsonwebtoken.Jwts;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.method.HandlerMethod;
import org.springframework.web.servlet.HandlerInterceptor;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.lang.reflect.Method;

@Component
public class PermissionAuthFilter implements HandlerInterceptor {

    private final String JWT_SECRET = "your-secret-key";

    @Autowired
    private PermissionService permissionService;

    @Autowired
    private UserRepository userRepository;

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler) throws Exception {
        // 检查是否是方法处理
        if (handler instanceof HandlerMethod) {
            HandlerMethod handlerMethod = (HandlerMethod) handler;
            Method method = handlerMethod.getMethod();

            // 检查方法是否有RequirePermission注解
            if (method.isAnnotationPresent(RequirePermission.class)) {
                RequirePermission annotation = method.getAnnotation(RequirePermission.class);
                String requiredPermission = annotation.value();

                // 从请求头获取token
                String token = extractToken(request);
                if (token == null || !validateToken(token)) {
                    response.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
                    return false;
                }

                // 从token获取用户信息
                String username = getUsernameFromToken(token);
                // 这里需要根据username获取userId，暂时假设我们有一个方法可以获取
                Long userId = getUserIdByUsername(username);

                // 检查用户是否有相应权限
                if (!permissionService.hasPermission(userId, requiredPermission)) {
                    response.setStatus(HttpServletResponse.SC_FORBIDDEN);
                    return false;
                }
            }
        }
        return true;
    }

    private String extractToken(HttpServletRequest request) {
        String bearerToken = request.getHeader("Authorization");
        if (bearerToken != null && bearerToken.startsWith("Bearer ")) {
            return bearerToken.substring(7);
        }
        return null;
    }

    private boolean validateToken(String token) {
        try {
            Jwts.parser().setSigningKey(JWT_SECRET).parseClaimsJws(token);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    private String getUsernameFromToken(String token) {
        return Jwts.parser().setSigningKey(JWT_SECRET).parseClaimsJws(token).getBody().getSubject();
    }

    private Long getUserIdByUsername(String username) {
        return userRepository.findByUsername(username)
                .map(user -> user.getId())
                .orElseThrow(() -> new RuntimeException("User not found"));
    }
}