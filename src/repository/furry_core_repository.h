#ifndef FURBBS_REPOSITORY_FURRY_CORE_REPOSITORY_H
#define FURBBS_REPOSITORY_FURRY_CORE_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>
#include <map>

namespace furbbs::repository {

struct FursonaRelationEntity {
    int64_t id = 0;
    int64_t fursona_a_id = 0;
    std::string fursona_a_name;
    std::string fursona_a_avatar;
    std::string fursona_a_owner;
    int64_t fursona_b_id = 0;
    std::string fursona_b_name;
    std::string fursona_b_avatar;
    std::string fursona_b_owner;
    std::string relation_type;
    bool user_a_confirmed = false;
    bool user_b_confirmed = false;
    int64_t anniversary = 0;
    std::map<std::string, std::string> relation_data;
    int64_t created_at = 0;
};

struct WorldSettingEntity {
    int64_t id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    std::string name;
    std::string description;
    std::string cover_image;
    std::string setting_type;
    std::vector<std::string> tags;
    bool is_public = false;
    int32_t view_count = 0;
    int32_t like_count = 0;
    bool is_liked = false;
    int64_t created_at = 0;
};

struct WorldPageEntity {
    int64_t id = 0;
    int64_t world_id = 0;
    std::string title;
    std::string content;
    std::string page_type;
    int64_t parent_id = 0;
    int32_t sort_order = 0;
    int64_t created_at = 0;
};

class FurryCoreRepository : protected BaseRepository {
public:
    static FurryCoreRepository& Instance() {
        static FurryCoreRepository instance;
        return instance;
    }

    int64_t CreateRelation(const std::string& user_id, const FursonaRelationEntity& data);
    bool ConfirmRelation(const std::string& user_id, int64_t relation_id);
    std::vector<FursonaRelationEntity> GetFursonaRelations(int64_t fursona_id);
    bool DeleteRelation(const std::string& user_id, int64_t relation_id);

    int64_t CreateWorld(const std::string& user_id, const WorldSettingEntity& data);
    bool UpdateWorld(const std::string& user_id, int64_t world_id, const WorldSettingEntity& data);
    std::vector<WorldSettingEntity> GetWorlds(const std::string& user_id,
                                               const std::string& viewer_id,
                                               bool only_public, int limit, int offset);
    int GetWorldCount(const std::string& user_id, bool only_public);
    std::optional<WorldSettingEntity> GetWorld(int64_t world_id, const std::string& viewer_id);
    bool LikeWorld(const std::string& user_id, int64_t world_id, bool like);
    bool DeleteWorld(const std::string& user_id, int64_t world_id);

    int64_t AddWorldPage(int64_t world_id, const std::string& user_id, const WorldPageEntity& page);
    std::vector<WorldPageEntity> GetWorldPages(int64_t world_id);
    bool DeleteWorldPage(int64_t world_id, const std::string& user_id, int64_t page_id);

private:
    FurryCoreRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_FURRY_CORE_REPOSITORY_H
