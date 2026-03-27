package com.furbbs.repository;

import com.furbbs.model.Permission;
import java.util.List;

public interface PermissionRepository {
    Permission findById(Long id);
    List<Permission> findAll();
    void save(Permission permission);
    void update(Permission permission);
    void delete(Long id);
    Permission findByCode(String code);
    List<Permission> findByRoleId(Long roleId);
    List<Permission> findByUserId(Long userId);
}