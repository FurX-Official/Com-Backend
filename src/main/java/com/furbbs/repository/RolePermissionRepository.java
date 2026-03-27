package com.furbbs.repository;

import com.furbbs.model.RolePermission;
import java.util.List;

public interface RolePermissionRepository {
    void save(RolePermission rolePermission);
    void deleteByRoleId(Long roleId);
    void deleteByRoleIdAndPermissionId(Long roleId, Long permissionId);
    List<RolePermission> findByRoleId(Long roleId);
    List<RolePermission> findByPermissionId(Long permissionId);
}