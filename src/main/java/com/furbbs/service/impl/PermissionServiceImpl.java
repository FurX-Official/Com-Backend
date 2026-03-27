package com.furbbs.service.impl;

import com.furbbs.model.Permission;
import com.furbbs.repository.PermissionRepository;
import com.furbbs.service.PermissionService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class PermissionServiceImpl implements PermissionService {

    @Autowired
    private PermissionRepository permissionRepository;

    @Override
    public Permission createPermission(Permission permission) {
        permissionRepository.save(permission);
        return permission;
    }

    @Override
    public Permission updatePermission(Permission permission) {
        permissionRepository.update(permission);
        return permission;
    }

    @Override
    public void deletePermission(Long permissionId) {
        permissionRepository.delete(permissionId);
    }

    @Override
    public Permission getPermissionById(Long permissionId) {
        return permissionRepository.findById(permissionId);
    }

    @Override
    public Permission getPermissionByCode(String code) {
        return permissionRepository.findByCode(code);
    }

    @Override
    public List<Permission> getAllPermissions() {
        return permissionRepository.findAll();
    }

    @Override
    public List<Permission> getPermissionsByUserId(Long userId) {
        return permissionRepository.findByUserId(userId);
    }

    @Override
    public boolean hasPermission(Long userId, String permissionCode) {
        List<Permission> permissions = permissionRepository.findByUserId(userId);
        for (Permission permission : permissions) {
            if (permission.getCode().equals(permissionCode)) {
                return true;
            }
        }
        return false;
    }
}