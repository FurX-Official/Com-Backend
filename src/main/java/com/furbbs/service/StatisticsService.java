package com.furbbs.service;

import java.util.Map;
import java.util.List;

public interface StatisticsService {
    // 用户活跃度统计
    Map<String, Object> getUserActivityStatistics();
    
    // 内容热度统计
    Map<String, Object> getContentHotnessStatistics();
    
    // 系统运行状态统计
    Map<String, Object> getSystemStatusStatistics();
    
    // 生成数据报表
    Map<String, Object> generateDataReport(String reportType, String startDate, String endDate);
}