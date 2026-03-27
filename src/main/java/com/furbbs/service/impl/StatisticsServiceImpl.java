package com.furbbs.service.impl;

import com.furbbs.service.StatisticsService;
import com.furbbs.repository.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.stream.Collectors;

@Service
public class StatisticsServiceImpl implements StatisticsService {
    
    @Autowired
    private UserRepository userRepository;
    
    @Autowired
    private PostRepository postRepository;
    
    @Autowired
    private CommentRepository commentRepository;
    
    @Autowired
    private PostLikeRepository postLikeRepository;
    
    @Autowired
    private PostFavoriteRepository postFavoriteRepository;
    
    @Autowired
    private MessageRepository messageRepository;
    
    @Autowired
    private NotificationRepository notificationRepository;
    
    @Autowired
    private MediaFileRepository mediaFileRepository;
    
    @Autowired
    private CategoryRepository categoryRepository;
    
    @Autowired
    private TagRepository tagRepository;
    
    @Override
    public Map<String, Object> getUserActivityStatistics() {
        Map<String, Object> statistics = new HashMap<>();
        
        // 总用户数
        long totalUsers = userRepository.count();
        statistics.put("totalUsers", totalUsers);
        
        // 今日注册用户数
        LocalDateTime today = LocalDateTime.now().withHour(0).withMinute(0).withSecond(0).withNano(0);
        long todayRegisteredUsers = userRepository.countByCreatedAtAfter(today);
        statistics.put("todayRegisteredUsers", todayRegisteredUsers);
        
        // 活跃用户数（最近7天有活动）
        LocalDateTime sevenDaysAgo = today.minusDays(7);
        long activeUsers = userRepository.countActiveUsers(sevenDaysAgo);
        statistics.put("activeUsers", activeUsers);
        
        // 用户活跃度分布
        List<Map<String, Object>> activityDistribution = userRepository.getUserActivityDistribution();
        statistics.put("activityDistribution", activityDistribution);
        
        return statistics;
    }
    
    @Override
    public Map<String, Object> getContentHotnessStatistics() {
        Map<String, Object> statistics = new HashMap<>();
        
        // 总帖子数
        long totalPosts = postRepository.count();
        statistics.put("totalPosts", totalPosts);
        
        // 今日发布帖子数
        LocalDateTime today = LocalDateTime.now().withHour(0).withMinute(0).withSecond(0).withNano(0);
        long todayPosts = postRepository.countByCreatedAtAfter(today);
        statistics.put("todayPosts", todayPosts);
        
        // 总评论数
        long totalComments = commentRepository.count();
        statistics.put("totalComments", totalComments);
        
        // 今日评论数
        long todayComments = commentRepository.countByCreatedAtAfter(today);
        statistics.put("todayComments", todayComments);
        
        // 热门帖子（按点赞数和评论数排序）
        List<Map<String, Object>> hotPosts = postRepository.findHotPosts(10);
        statistics.put("hotPosts", hotPosts);
        
        // 热门分类
        List<Map<String, Object>> hotCategories = categoryRepository.findHotCategories(5);
        statistics.put("hotCategories", hotCategories);
        
        // 热门标签
        List<Map<String, Object>> hotTags = tagRepository.findHotTags(10);
        statistics.put("hotTags", hotTags);
        
        return statistics;
    }
    
    @Override
    public Map<String, Object> getSystemStatusStatistics() {
        Map<String, Object> statistics = new HashMap<>();
        
        // 系统运行时间
        statistics.put("systemUptime", System.currentTimeMillis());
        
        // 数据库连接数（模拟）
        statistics.put("databaseConnections", 10);
        
        // 内存使用情况（模拟）
        Runtime runtime = Runtime.getRuntime();
        long totalMemory = runtime.totalMemory();
        long freeMemory = runtime.freeMemory();
        long usedMemory = totalMemory - freeMemory;
        statistics.put("memoryUsage", Map.of(
            "total", totalMemory,
            "used", usedMemory,
            "free", freeMemory
        ));
        
        // 请求统计（模拟）
        statistics.put("requestStatistics", Map.of(
            "totalRequests", 1000,
            "todayRequests", 100,
            "averageResponseTime", 150
        ));
        
        // 存储使用情况（模拟）
        statistics.put("storageUsage", Map.of(
            "total", 1024 * 1024 * 1024 * 10L, // 10GB
            "used", 1024 * 1024 * 1024 * 3L,  // 3GB
            "free", 1024 * 1024 * 1024 * 7L   // 7GB
        ));
        
        return statistics;
    }
    
    @Override
    public Map<String, Object> generateDataReport(String reportType, String startDate, String endDate) {
        Map<String, Object> report = new HashMap<>();
        
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd");
        LocalDateTime start = LocalDateTime.parse(startDate, formatter).withHour(0).withMinute(0).withSecond(0).withNano(0);
        LocalDateTime end = LocalDateTime.parse(endDate, formatter).withHour(23).withMinute(59).withSecond(59).withNano(999999999);
        
        switch (reportType) {
            case "user_activity":
                report.put("reportType", "user_activity");
                report.put("period", Map.of("start", startDate, "end", endDate));
                report.put("userRegistration", userRepository.countByCreatedAtBetween(start, end));
                report.put("activeUsers", userRepository.countActiveUsersInPeriod(start, end));
                report.put("userActivityTrend", userRepository.getUserActivityTrend(start, end));
                break;
                
            case "content_performance":
                report.put("reportType", "content_performance");
                report.put("period", Map.of("start", startDate, "end", endDate));
                report.put("postsCreated", postRepository.countByCreatedAtBetween(start, end));
                report.put("commentsCreated", commentRepository.countByCreatedAtBetween(start, end));
                report.put("likesGiven", postLikeRepository.countByCreatedAtBetween(start, end));
                report.put("favoritesAdded", postFavoriteRepository.countByCreatedAtBetween(start, end));
                report.put("contentTrend", postRepository.getPostCreationTrend(start, end));
                break;
                
            case "system_health":
                report.put("reportType", "system_health");
                report.put("period", Map.of("start", startDate, "end", endDate));
                report.put("systemStatus", getSystemStatusStatistics());
                report.put("errorRate", 0.01); // 模拟错误率
                report.put("performanceTrend", List.of(
                    Map.of("date", startDate, "responseTime", 150),
                    Map.of("date", endDate, "responseTime", 145)
                ));
                break;
                
            default:
                report.put("error", "Invalid report type");
        }
        
        return report;
    }
}