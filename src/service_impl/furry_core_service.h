#ifndef FURBBS_SERVICE_IMPL_FURRY_CORE_SERVICE_H
#define FURBBS_SERVICE_IMPL_FURRY_CORE_SERVICE_H

#include "../repository/furry_core_repository.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::service {

class FurryCoreService {
public:
    static FurryCoreService& Instance() {
        static FurryCoreService instance;
        return instance;
    }

    int64_t CreateRelation(const std::string& user_id, const repository::FursonaRelationEntity& data) {
        auto relations = repository::FurryCoreRepository::Instance().GetFursonaRelations(data.fursona_a_id);
        for (auto& r : relations) {
            if ((r.fursona_a_id == data.fursona_a_id && r.fursona_b_id == data.fursona_b_id) ||
                (r.fursona_a_id == data.fursona_b_id && r.fursona_b_id == data.fursona_a_id)) {
                return 0;
            }
        }
        return repository::FurryCoreRepository::Instance().CreateRelation(user_id, data);
    }

    bool ConfirmRelation(const std::string& user_id, int64_t relation_id) {
        return repository::FurryCoreRepository::Instance().ConfirmRelation(user_id, relation_id);
    }

    std::vector<repository::FursonaRelationEntity> GetFursonaRelations(int64_t fursona_id) {
        return repository::FurryCoreRepository::Instance().GetFursonaRelations(fursona_id);
    }

    bool DeleteRelation(const std::string& user_id, int64_t relation_id) {
        return repository::FurryCoreRepository::Instance().DeleteRelation(user_id, relation_id);
    }

    int64_t CreateWorld(const std::string& user_id, const repository::WorldSettingEntity& data) {
        return repository::FurryCoreRepository::Instance().CreateWorld(user_id, data);
    }

    bool UpdateWorld(const std::string& user_id, int64_t world_id, const repository::WorldSettingEntity& data) {
        return repository::FurryCoreRepository::Instance().UpdateWorld(user_id, world_id, data);
    }

    std::vector<repository::WorldSettingEntity> GetWorlds(
            const std::string& user_id, const std::string& viewer_id,
            bool only_public, int page, int page_size) {
        return repository::FurryCoreRepository::Instance().GetWorlds(
            user_id, viewer_id, only_public, page_size, (page - 1) * page_size);
    }

    std::optional<repository::WorldSettingEntity> GetWorld(int64_t world_id, const std::string& viewer_id) {
        return repository::FurryCoreRepository::Instance().GetWorld(world_id, viewer_id);
    }

    bool LikeWorld(const std::string& user_id, int64_t world_id, bool like) {
        return repository::FurryCoreRepository::Instance().LikeWorld(user_id, world_id, like);
    }

    bool DeleteWorld(const std::string& user_id, int64_t world_id) {
        return repository::FurryCoreRepository::Instance().DeleteWorld(user_id, world_id);
    }

    int64_t AddWorldPage(int64_t world_id, const std::string& user_id, const repository::WorldPageEntity& page) {
        return repository::FurryCoreRepository::Instance().AddWorldPage(world_id, user_id, page);
    }

    std::vector<repository::WorldPageEntity> GetWorldPages(int64_t world_id) {
        return repository::FurryCoreRepository::Instance().GetWorldPages(world_id);
    }

    bool DeleteWorldPage(int64_t world_id, const std::string& user_id, int64_t page_id) {
        return repository::FurryCoreRepository::Instance().DeleteWorldPage(world_id, user_id, page_id);
    }

private:
    FurryCoreService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_FURRY_CORE_SERVICE_H
