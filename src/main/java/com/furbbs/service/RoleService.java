package com.furbbs.service;

import com.furbbs.model.Role;
import com.furbbs.model.Permission;
import java.util.List;

public interface RoleService {
    Role createRole(Role role);
    Role updateRole(Role role);
    void deleteRole(Long roleId);
    Role getRoleById(Long roleId);
    Role getRoleByName(String name);
    List<Role> getAllRoles();
    void assignPermissionsToRole(Long roleId, List<Long> permissionIds);
    List<Permission> getPermissionsByRoleId(Long roleId);
    void assignRoleToUser(Long userId, Long roleId);
    void removeRoleFromUser(Long userId, Long roleId);
    List<Role> getRolesByUserId(Long userId);
}