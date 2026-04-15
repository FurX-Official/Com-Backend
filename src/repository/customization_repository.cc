#include "customization_repository.h"

namespace furbbs::repository {

std::vector<TitleEntity> CustomizationRepository::GetAllTitles() {
    return Execute<std::vector<TitleEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::USER_TITLE_LIST);
        std::vector<TitleEntity> titles;
        for (const auto& row : result) {
            TitleEntity t;
            t.id = row[0].as<int32_t>();
            t.name = row[1].as<std::string>();
            t.color = row[2].as<std::string>();
            if (!row[3].is_null()) t.bg_color = row[3].as<std::string>();
            if (!row[4].is_null()) t.icon = row[4].as<std::string>();
            t.rarity = row[5].as<int32_t>();
            t.is_animated = row[6].as<bool>();
            titles.push_back(t);
        }
        return titles;
    });
}

void CustomizationRepository::SetActiveTitle(const std::string& user_id, int32_t title_id, int64_t timestamp) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::USER_ACTIVE_TITLE_SET, user_id, title_id, timestamp);
    });
}

std::vector<AvatarFrameEntity> CustomizationRepository::GetAllAvatarFrames() {
    return Execute<std::vector<AvatarFrameEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::AVATAR_FRAME_LIST);
        std::vector<AvatarFrameEntity> frames;
        for (const auto& row : result) {
            AvatarFrameEntity f;
            f.id = row[0].as<int32_t>();
            f.name = row[1].as<std::string>();
            f.image_url = row[2].as<std::string>();
            f.rarity = row[3].as<int32_t>();
            f.is_animated = row[4].as<bool>();
            f.price = row[5].as<int32_t>();
            frames.push_back(f);
        }
        return frames;
    });
}

void CustomizationRepository::SetActiveFrame(const std::string& user_id, int32_t frame_id, int64_t timestamp) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::USER_ACTIVE_FRAME_SET, user_id, frame_id, timestamp);
    });
}

std::vector<NameplateEntity> CustomizationRepository::GetAllNameplates() {
    return Execute<std::vector<NameplateEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::NAMEPLATE_STYLE_LIST);
        std::vector<NameplateEntity> nameplates;
        for (const auto& row : result) {
            NameplateEntity n;
            n.id = row[0].as<int32_t>();
            n.name = row[1].as<std::string>();
            if (!row[2].is_null()) n.card_bg = row[2].as<std::string>();
            if (!row[3].is_null()) n.text_color = row[3].as<std::string>();
            if (!row[4].is_null()) n.effect = row[4].as<std::string>();
            n.rarity = row[5].as<int32_t>();
            nameplates.push_back(n);
        }
        return nameplates;
    });
}

std::vector<ThemeEntity> CustomizationRepository::GetAllThemes() {
    return Execute<std::vector<ThemeEntity>>([&](pqxx::work& txn) {
        auto result = txn.exec(sql::PROFILE_THEME_LIST);
        std::vector<ThemeEntity> themes;
        for (const auto& row : result) {
            ThemeEntity t;
            t.id = row[0].as<int32_t>();
            t.name = row[1].as<std::string>();
            if (!row[2].is_null()) t.bg_image = row[2].as<std::string>();
            t.primary_color = row[3].as<std::string>();
            t.secondary_color = row[4].as<std::string>();
            t.is_premium = row[5].as<bool>();
            themes.push_back(t);
        }
        return themes;
    });
}

void CustomizationRepository::SetCustomization(const std::string& user_id, int32_t nameplate_id,
                                                int32_t theme_id, int64_t timestamp) {
    Execute([&](pqxx::work& txn) {
        txn.exec_params(sql::USER_CUSTOMIZATION_SET, user_id,
            nameplate_id > 0 ? nameplate_id : nullptr,
            theme_id > 0 ? theme_id : nullptr,
            timestamp);
    });
}

} // namespace furbbs::repository
