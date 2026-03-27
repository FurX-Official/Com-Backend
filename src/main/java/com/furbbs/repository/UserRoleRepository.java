package com.furbbs.repository;

import com.furbbs.model.UserRole;
import java.util.List;

public interface UserRoleRepository {
    void save(UserRole userRole);
    void deleteByUserId(Long userId);
    void deleteByUserIdAndRoleId(Long userId, Long roleId);
    List<UserRole> findByUserId(Long userId);
    List<UserRole> findByRoleId(Long roleId);
}