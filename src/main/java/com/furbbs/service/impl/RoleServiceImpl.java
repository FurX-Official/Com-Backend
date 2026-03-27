package com.furbbs.service.impl;

import com.furbbs.model.Role;
import com.furbbs.model.Permission;
import com.furbbs.model.UserRole;
import com.furbbs.model.RolePermission;
import com.furbbs.repository.RoleRepository;
import com.furbbs.repository.PermissionRepository;
import com.furbbs.repository.UserRoleRepository;
import com.furbbs.repository.RolePermissionRepository;
import com.furbbs.service.RoleService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class RoleServiceImpl implements RoleService {

    @Autowired
    private RoleRepository roleRepository;

    @Autowired
    private PermissionRepository permissionRepository;

    @Autowired
    private UserRoleRepository userRoleRepository;

    @Autowired
    private RolePermissionRepository rolePermissionRepository;

    @Override
    public Role createRole(Role role) {
        roleRepository.save(role);
        return role;
    }

    @Override
    public Role updateRole(Role role) {
        roleRepository.update(role);
        return role;
    }

    @Override
    public void deleteRole(Long roleId) {
        // 先删除角色-权限关联
        rolePermissionRepository.deleteByRoleId(roleId);
        // 再删除用户-角色关联
        List<UserRole> userRoles = userRoleRepository.findByRoleId(roleId);
        for (UserRole userRole : userRoles) {
            userRoleRepository.deleteByUserIdAndRoleId(userRole.getUserId(), roleId);
        }
        // 最后删除角色
        roleRepository.delete(roleId);
    }

    @Override
    public Role getRoleById(Long roleId) {
        return roleRepository.findById(roleId);
    }

    @Override
    public Role getRoleByName(String name) {
        return roleRepository.findByName(name);
    }

    @Override
    public List<Role> getAllRoles() {
        return roleRepository.findAll();
    }

    @Override
    public void assignPermissionsToRole(Long roleId, List<Long> permissionIds) {
        // 先删除原有权限关联
        rolePermissionRepository.deleteByRoleId(roleId);
        // 添加新的权限关联
        for (Long permissionId : permissionIds) {
            RolePermission rolePermission = new RolePermission();
            rolePermission.setRoleId(roleId);
            rolePermission.setPermissionId(permissionId);
            rolePermissionRepository.save(rolePermission);
        }
    }

    @Override
    public List<Permission> getPermissionsByRoleId(Long roleId) {
        return permissionRepository.findByRoleId(roleId);
    }

    @Override
    public void assignRoleToUser(Long userId, Long roleId) {
        UserRole userRole = new UserRole();
        userRole.setUserId(userId);
        userRole.setRoleId(roleId);
        userRoleRepository.save(userRole);
    }

    @Override
    public void removeRoleFromUser(Long userId, Long roleId) {
        userRoleRepository.deleteByUserIdAndRoleId(userId, roleId);
    }

    @Override
    public List<Role> getRolesByUserId(Long userId) {
        return roleRepository.findByUserId(userId);
    }
}