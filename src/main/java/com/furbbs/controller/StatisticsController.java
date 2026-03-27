package com.furbbs.controller;

import com.furbbs.service.StatisticsService;
import com.furbbs.config.RequirePermission;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;

@RestController
@RequestMapping("/api/statistics")
public class StatisticsController {
    
    @Autowired
    private StatisticsService statisticsService;
    
    @RequirePermission("STATISTICS_VIEW")
    @GetMapping("/user-activity")
    public ResponseEntity<Map<String, Object>> getUserActivityStatistics() {
        Map<String, Object> statistics = statisticsService.getUserActivityStatistics();
        return ResponseEntity.ok(statistics);
    }
    
    @RequirePermission("STATISTICS_VIEW")
    @GetMapping("/content-hotness")
    public ResponseEntity<Map<String, Object>> getContentHotnessStatistics() {
        Map<String, Object> statistics = statisticsService.getContentHotnessStatistics();
        return ResponseEntity.ok(statistics);
    }
    
    @RequirePermission("STATISTICS_VIEW")
    @GetMapping("/system-status")
    public ResponseEntity<Map<String, Object>> getSystemStatusStatistics() {
        Map<String, Object> statistics = statisticsService.getSystemStatusStatistics();
        return ResponseEntity.ok(statistics);
    }
    
    @RequirePermission("STATISTICS_VIEW")
    @GetMapping("/generate-report")
    public ResponseEntity<Map<String, Object>> generateDataReport(
            @RequestParam String reportType,
            @RequestParam String startDate,
            @RequestParam String endDate) {
        Map<String, Object> report = statisticsService.generateDataReport(reportType, startDate, endDate);
        return ResponseEntity.ok(report);
    }
}