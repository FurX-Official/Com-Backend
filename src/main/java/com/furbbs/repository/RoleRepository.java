package com.furbbs.repository;

import com.furbbs.model.Role;
import java.util.List;

public interface RoleRepository {
    Role findById(Long id);
    List<Role> findAll();
    void save(Role role);
    void update(Role role);
    void delete(Long id);
    Role findByName(String name);
    List<Role> findByUserId(Long userId);
}