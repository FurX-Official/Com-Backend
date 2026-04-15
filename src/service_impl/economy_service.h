#ifndef FURBBS_SERVICE_IMPL_ECONOMY_SERVICE_H
#define FURBBS_SERVICE_IMPL_ECONOMY_SERVICE_H

#include "../repository/economy_repository.h"
#include "../auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class EconomyService {
public:
    static EconomyService& Instance() {
        static EconomyService instance;
        return instance;
    }

    int64_t CreatePaidContent(const std::string& token, int64_t post_id,
                              int32_t price, const std::string& preview,
                              const std::string& full) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return 0;
        return repository::EconomyRepository::Instance().CreatePaidContent(
            user->user_id, post_id, price, preview, full);
    }

    std::vector<repository::PaidContentEntity> GetUserPaidContents(
        const std::string& token, const std::string& user_id,
        int page, int page_size, int& out_total) {
        std::string viewer_id;
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user.has_value()) viewer_id = user->user_id;

        out_total = repository::EconomyRepository::Instance().GetPaidContentCount(user_id);
        return repository::EconomyRepository::Instance().GetUserPaidContents(
            user_id, viewer_id, page_size, (page - 1) * page_size);
    }

    bool PurchaseContent(const std::string& token, int64_t content_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().PurchaseContent(
            user->user_id, content_id);
    }

    int64_t CreateGallery(const std::string& token, const std::string& name,
                          const std::string& description, const std::string& cover,
                          bool is_public, bool is_nsfw) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return 0;
        return repository::EconomyRepository::Instance().CreateGallery(
            user->user_id, name, description, cover, is_public, is_nsfw);
    }

    bool DeleteGallery(const std::string& token, int64_t gallery_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().DeleteGallery(
            user->user_id, gallery_id);
    }

    std::vector<repository::GalleryEntity> GetGalleries(
        const std::string& token, const std::string& user_id,
        bool only_public, int page, int page_size, int& out_total) {
        std::string viewer_id;
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user.has_value()) viewer_id = user->user_id;

        bool pub = only_public || (viewer_id != user_id);
        out_total = repository::EconomyRepository::Instance().GetGalleryCount(user_id, pub);
        return repository::EconomyRepository::Instance().GetGalleries(
            user_id, viewer_id, pub, page_size, (page - 1) * page_size);
    }

    bool SetGalleryFavorite(const std::string& token, int64_t gallery_id, bool favorite) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().SetGalleryFavorite(
            user->user_id, gallery_id, favorite);
    }

    int64_t AddGalleryItem(const std::string& token, int64_t gallery_id,
                           const repository::GalleryItemEntity& item) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return 0;
        return repository::EconomyRepository::Instance().AddGalleryItem(
            gallery_id, user->user_id, item);
    }

    bool RemoveGalleryItem(const std::string& token, int64_t gallery_id, int64_t item_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().RemoveGalleryItem(
            gallery_id, user->user_id, item_id);
    }

    std::vector<repository::GalleryItemEntity> GetGalleryItems(
        const std::string& token, int64_t gallery_id,
        int page, int page_size, int& out_total) {
        std::string viewer_id;
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user.has_value()) viewer_id = user->user_id;

        out_total = repository::EconomyRepository::Instance().GetGalleryItemCount(gallery_id);
        return repository::EconomyRepository::Instance().GetGalleryItems(
            gallery_id, viewer_id, page_size, (page - 1) * page_size);
    }

    bool TransferPoints(const std::string& token, const std::string& to_id,
                        int32_t amount, const std::string& message) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().TransferPoints(
            user->user_id, to_id, amount, message);
    }

    std::vector<repository::TransferEntity> GetTransferHistory(
        const std::string& token, int page, int page_size) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return {};
        return repository::EconomyRepository::Instance().GetTransferHistory(
            user->user_id, page_size, (page - 1) * page_size);
    }

    int64_t CreateRedEnvelope(const std::string& token, int32_t total_amount,
                              int32_t count, const std::string& message, bool is_random) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return 0;
        return repository::EconomyRepository::Instance().CreateRedEnvelope(
            user->user_id, total_amount, count, message, is_random);
    }

    int32_t ClaimRedEnvelope(const std::string& token, int64_t envelope_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return 0;
        return repository::EconomyRepository::Instance().ClaimRedEnvelope(
            envelope_id, user->user_id);
    }

    std::vector<repository::RedEnvelopeEntity> GetActiveRedEnvelopes(
        int page, int page_size) {
        return repository::EconomyRepository::Instance().GetActiveRedEnvelopes(
            page_size, (page - 1) * page_size);
    }

    bool RewardPost(const std::string& token, int64_t post_id,
                    int32_t amount, const std::string& message, bool anonymous) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().RewardPost(
            user->user_id, post_id, amount, message, anonymous);
    }

    std::vector<repository::PostRewardEntity> GetPostRewards(
        int64_t post_id, int page, int page_size) {
        return repository::EconomyRepository::Instance().GetPostRewards(
            post_id, page_size, (page - 1) * page_size);
    }

    int64_t CreateCollection(const std::string& token, const std::string& name,
                              const std::string& description, const std::string& cover,
                              bool is_public) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return 0;
        return repository::EconomyRepository::Instance().CreateCollection(
            user->user_id, name, description, cover, is_public);
    }

    bool DeleteCollection(const std::string& token, int64_t collection_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().DeleteCollection(
            user->user_id, collection_id);
    }

    bool AddToCollection(const std::string& token, int64_t collection_id,
                         int64_t post_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().AddToCollection(
            user->user_id, collection_id, post_id);
    }

    bool RemoveFromCollection(const std::string& token, int64_t collection_id,
                              int64_t post_id) {
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user.has_value()) return false;
        return repository::EconomyRepository::Instance().RemoveFromCollection(
            user->user_id, collection_id, post_id);
    }

    std::vector<repository::CollectionEntity> GetUserCollections(
        const std::string& token, const std::string& user_id,
        int page, int page_size) {
        std::string viewer_id;
        auto user = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user.has_value()) viewer_id = user->user_id;
        return repository::EconomyRepository::Instance().GetUserCollections(
            user_id, viewer_id, page_size, (page - 1) * page_size);
    }

private:
    EconomyService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_ECONOMY_SERVICE_H
