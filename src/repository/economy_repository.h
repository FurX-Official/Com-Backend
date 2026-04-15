#ifndef FURBBS_REPOSITORY_ECONOMY_REPOSITORY_H
#define FURBBS_REPOSITORY_ECONOMY_REPOSITORY_H

#include "base_repository.h"
#include "user_repository.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::repository {

struct PaidContentEntity {
    int64_t id = 0;
    int64_t post_id = 0;
    std::string author_id;
    int32_t price = 0;
    std::string preview_content;
    std::string full_content;
    int32_t purchase_count = 0;
    int32_t revenue = 0;
    int64_t created_at = 0;
    bool purchased = false;
};

struct GalleryEntity {
    int64_t id = 0;
    std::string user_id;
    std::string username;
    std::string avatar;
    std::string name;
    std::string description;
    std::string cover_image;
    bool is_public = true;
    bool is_nsfw = false;
    int32_t item_count = 0;
    int32_t view_count = 0;
    int32_t like_count = 0;
    bool is_favorited = false;
    int64_t created_at = 0;
};

struct GalleryItemEntity {
    int64_t id = 0;
    int64_t gallery_id = 0;
    std::string title;
    std::string description;
    std::string image_url;
    std::string thumbnail_url;
    int64_t fursona_id = 0;
    std::string artist_name;
    bool is_nsfw = false;
    int32_t view_count = 0;
    int64_t created_at = 0;
};

struct TransferEntity {
    int64_t id = 0;
    std::string from_user_id;
    std::string from_username;
    std::string from_avatar;
    std::string to_user_id;
    std::string to_username;
    std::string to_avatar;
    int32_t amount = 0;
    std::string message;
    int64_t created_at = 0;
};

struct RedEnvelopeEntity {
    int64_t id = 0;
    std::string sender_id;
    std::string sender_name;
    std::string sender_avatar;
    int32_t total_amount = 0;
    int32_t count = 0;
    int32_t remaining_amount = 0;
    int32_t remaining_count = 0;
    std::string message;
    bool is_random = true;
    int64_t expires_at = 0;
    int64_t created_at = 0;
    bool claimed = false;
    int32_t claimed_amount = 0;
};

struct PostRewardEntity {
    int64_t id = 0;
    int64_t post_id = 0;
    std::string sender_id;
    std::string sender_name;
    std::string sender_avatar;
    int32_t amount = 0;
    std::string message;
    bool anonymous = false;
    int64_t created_at = 0;
};

struct CollectionEntity {
    int64_t id = 0;
    std::string user_id;
    std::string name;
    std::string description;
    std::string cover_image;
    bool is_public = true;
    int32_t item_count = 0;
    int64_t created_at = 0;
};

class EconomyRepository : protected BaseRepository {
public:
    static EconomyRepository& Instance() {
        static EconomyRepository instance;
        return instance;
    }

    int64_t CreatePaidContent(const std::string& author_id, int64_t post_id,
                              int32_t price, const std::string& preview,
                              const std::string& full);

    std::vector<PaidContentEntity> GetUserPaidContents(
        const std::string& user_id, const std::string& viewer_id,
        int limit, int offset);

    int GetPaidContentCount(const std::string& user_id);

    std::optional<PaidContentEntity> GetPaidContent(
        int64_t content_id, const std::string& viewer_id);

    bool PurchaseContent(const std::string& buyer_id, int64_t content_id);

    int64_t CreateGallery(const std::string& user_id, const std::string& name,
                          const std::string& description, const std::string& cover,
                          bool is_public, bool is_nsfw);

    bool UpdateGallery(const std::string& user_id, int64_t gallery_id,
                       const GalleryEntity& data);

    bool DeleteGallery(const std::string& user_id, int64_t gallery_id);

    std::vector<GalleryEntity> GetGalleries(const std::string& user_id,
                                             const std::string& viewer_id,
                                             bool only_public, int limit, int offset);

    int GetGalleryCount(const std::string& user_id, bool only_public);

    std::optional<GalleryEntity> GetGallery(int64_t gallery_id,
                                             const std::string& viewer_id);

    bool SetGalleryFavorite(const std::string& user_id, int64_t gallery_id, bool favorite);

    int64_t AddGalleryItem(int64_t gallery_id, const std::string& user_id,
                           const GalleryItemEntity& item);

    bool RemoveGalleryItem(int64_t gallery_id, const std::string& user_id,
                           int64_t item_id);

    std::vector<GalleryItemEntity> GetGalleryItems(int64_t gallery_id,
                                                    const std::string& viewer_id,
                                                    int limit, int offset);

    int GetGalleryItemCount(int64_t gallery_id);

    bool TransferPoints(const std::string& from_id, const std::string& to_id,
                        int32_t amount, const std::string& message);

    std::vector<TransferEntity> GetTransferHistory(
        const std::string& user_id, int limit, int offset);

    int64_t CreateRedEnvelope(const std::string& sender_id, int32_t total_amount,
                              int32_t count, const std::string& message, bool is_random);

    int32_t ClaimRedEnvelope(int64_t envelope_id, const std::string& user_id);

    std::vector<RedEnvelopeEntity> GetActiveRedEnvelopes(int limit, int offset);

    bool RewardPost(const std::string& sender_id, int64_t post_id,
                    int32_t amount, const std::string& message, bool anonymous);

    std::vector<PostRewardEntity> GetPostRewards(int64_t post_id, int limit, int offset);

    int64_t CreateCollection(const std::string& user_id, const std::string& name,
                              const std::string& description, const std::string& cover,
                              bool is_public);

    bool DeleteCollection(const std::string& user_id, int64_t collection_id);

    bool AddToCollection(const std::string& user_id, int64_t collection_id,
                         int64_t post_id);

    bool RemoveFromCollection(const std::string& user_id, int64_t collection_id,
                              int64_t post_id);

    std::vector<CollectionEntity> GetUserCollections(
        const std::string& user_id, const std::string& viewer_id, int limit, int offset);

    std::vector<int64_t> GetCollectionPosts(int64_t collection_id,
                                              const std::string& viewer_id,
                                              int limit, int offset);

private:
    EconomyRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_ECONOMY_REPOSITORY_H
