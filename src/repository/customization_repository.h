#ifndef FURBBS_REPOSITORY_CUSTOMIZATION_REPOSITORY_H
#define FURBBS_REPOSITORY_CUSTOMIZATION_REPOSITORY_H

#include "base_repository.h"
#include "db/sql_queries.h"
#include <vector>

namespace furbbs::repository {

struct TitleEntity {
    int32_t id = 0;
    std::string name;
    std::string color;
    std::string bg_color;
    std::string icon;
    int32_t rarity = 1;
    bool is_animated = false;
};

struct AvatarFrameEntity {
    int32_t id = 0;
    std::string name;
    std::string image_url;
    int32_t rarity = 1;
    bool is_animated = false;
    int32_t price = 0;
};

struct NameplateEntity {
    int32_t id = 0;
    std::string name;
    std::string card_bg;
    std::string text_color;
    std::string effect;
    int32_t rarity = 1;
};

struct ThemeEntity {
    int32_t id = 0;
    std::string name;
    std::string bg_image;
    std::string primary_color;
    std::string secondary_color;
    bool is_premium = false;
};

class CustomizationRepository : protected BaseRepository {
public:
    static CustomizationRepository& Instance() {
        static CustomizationRepository instance;
        return instance;
    }

    std::vector<TitleEntity> GetAllTitles();
    void SetActiveTitle(const std::string& user_id, int32_t title_id, int64_t timestamp);

    std::vector<AvatarFrameEntity> GetAllAvatarFrames();
    void SetActiveFrame(const std::string& user_id, int32_t frame_id, int64_t timestamp);

    std::vector<NameplateEntity> GetAllNameplates();
    std::vector<ThemeEntity> GetAllThemes();
    void SetCustomization(const std::string& user_id, int32_t nameplate_id,
                          int32_t theme_id, int64_t timestamp);

private:
    CustomizationRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_CUSTOMIZATION_REPOSITORY_H
