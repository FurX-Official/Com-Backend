package com.furbbs.service;

import com.furbbs.model.Permission;
import java.util.List;

public interface PermissionService {
    Permission createPermission(Permission permission);
    Permission updatePermission(Permission permission);
    void deletePermission(Long permissionId);
    Permission getPermissionById(Long permissionId);
    Permission getPermissionByCode(String code);
    List<Permission> getAllPermissions();
    List<Permission> getPermissionsByUserId(Long userId);
    boolean hasPermission(Long userId, String permissionCode);
}