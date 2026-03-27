package com.furbbs.controller;

import com.furbbs.model.Role;
import com.furbbs.model.Permission;
import com.furbbs.service.RoleService;
import com.furbbs.config.RequirePermission;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/roles")
public class RoleController {

    @Autowired
    private RoleService roleService;

    @RequirePermission("ROLE_MANAGE")
    @PostMapping
    public ResponseEntity<Role> createRole(@RequestBody Role role) {
        Role createdRole = roleService.createRole(role);
        return ResponseEntity.ok(createdRole);
    }

    @RequirePermission("ROLE_MANAGE")
    @PutMapping("/{id}")
    public ResponseEntity<Role> updateRole(@PathVariable Long id, @RequestBody Role role) {
        role.setId(id);
        Role updatedRole = roleService.updateRole(role);
        return ResponseEntity.ok(updatedRole);
    }

    @RequirePermission("ROLE_MANAGE")
    @DeleteMapping("/{id}")
    public ResponseEntity<Void> deleteRole(@PathVariable Long id) {
        roleService.deleteRole(id);
        return ResponseEntity.ok().build();
    }

    @GetMapping("/{id}")
    public ResponseEntity<Role> getRoleById(@PathVariable Long id) {
        Role role = roleService.getRoleById(id);
        return ResponseEntity.ok(role);
    }

    @GetMapping
    public ResponseEntity<List<Role>> getAllRoles() {
        List<Role> roles = roleService.getAllRoles();
        return ResponseEntity.ok(roles);
    }

    @RequirePermission("ROLE_MANAGE")
    @PostMapping("/{id}/permissions")
    public ResponseEntity<Void> assignPermissionsToRole(@PathVariable Long id, @RequestBody List<Long> permissionIds) {
        roleService.assignPermissionsToRole(id, permissionIds);
        return ResponseEntity.ok().build();
    }

    @GetMapping("/{id}/permissions")
    public ResponseEntity<List<Permission>> getPermissionsByRoleId(@PathVariable Long id) {
        List<Permission> permissions = roleService.getPermissionsByRoleId(id);
        return ResponseEntity.ok(permissions);
    }

    @RequirePermission("USER_MANAGE")
    @PostMapping("/assign-to-user")
    public ResponseEntity<Void> assignRoleToUser(@RequestParam Long userId, @RequestParam Long roleId) {
        roleService.assignRoleToUser(userId, roleId);
        return ResponseEntity.ok().build();
    }

    @RequirePermission("USER_MANAGE")
    @DeleteMapping("/remove-from-user")
    public ResponseEntity<Void> removeRoleFromUser(@RequestParam Long userId, @RequestParam Long roleId) {
        roleService.removeRoleFromUser(userId, roleId);
        return ResponseEntity.ok().build();
    }

    @GetMapping("/user/{userId}")
    public ResponseEntity<List<Role>> getRolesByUserId(@PathVariable Long userId) {
        List<Role> roles = roleService.getRolesByUserId(userId);
        return ResponseEntity.ok(roles);
    }
}