#ifndef FURBBS_REPOSITORY_SOCIAL_REPOSITORY_H
#define FURBBS_REPOSITORY_SOCIAL_REPOSITORY_H

#include "base_repository.h"
#include <vector>
#include <string>
#include <optional>

namespace furbbs::repository {

struct FollowInfoEntity {
    std::string user_id;
    std::string username;
    std::string avatar;
    std::string bio;
    bool is_mutual = false;
    int64_t followed_at = 0;
};

struct GiftEntity {
    int32_t id = 0;
    std::string name;
    std::string icon;
    int32_t price = 0;
    std::string animation;
    bool is_animated = false;
    std::string rarity;
};

struct GiftRecordEntity {
    std::string from_user_id;
    std::string from_username;
    std::string from_avatar;
    int32_t gift_id = 0;
    std::string gift_name;
    int32_t quantity = 0;
    int32_t total_value = 0;
    std::string message;
    bool is_anonymous = false;
    int64_t sent_at = 0;
};

struct ArtistReviewEntity {
    std::string reviewer_id;
    std::string reviewer_name;
    std::string reviewer_avatar;
    int32_t rating = 0;
    std::string comment;
    std::vector<std::string> tags;
    int64_t created_at = 0;
};

struct BlacklistEntryEntity {
    std::string artist_id;
    std::string artist_name;
    std::string reason;
    int64_t added_at = 0;
};

struct QuestionBoxEntity {
    int64_t id = 0;
    std::string owner_id;
    std::string owner_name;
    std::string owner_avatar;
    bool is_public = true;
    bool allow_anonymous = true;
    std::string description;
    int32_t question_count = 0;
    int64_t created_at = 0;
};

struct QuestionEntity {
    int64_t id = 0;
    std::string asker_id;
    std::string asker_name;
    std::string asker_avatar;
    bool is_anonymous = false;
    std::string content;
    std::string answer;
    bool is_answered = false;
    bool is_public = true;
    int64_t asked_at = 0;
    int64_t answered_at = 0;
};

class SocialRepository : protected BaseRepository {
public:
    static SocialRepository& Instance() {
        static SocialRepository instance;
        return instance;
    }

    bool SetFollow(const std::string& follower_id, const std::string& following_id, bool follow);

    std::vector<FollowInfoEntity> GetFollowing(
        const std::string& user_id, int limit, int offset);

    std::vector<FollowInfoEntity> GetFollowers(
        const std::string& user_id, int limit, int offset);

    int GetFollowingCount(const std::string& user_id);
    int GetFollowerCount(const std::string& user_id);

    std::vector<int64_t> GetFriendCirclePosts(
        const std::string& user_id, int limit, int offset);

    bool SetFursonaFavorite(const std::string& user_id, int64_t fursona_id, bool favorite);

    std::vector<int64_t> GetFavoriteFursonas(
        const std::string& user_id, int limit, int offset);

    int GetFavoriteFursonaCount(const std::string& user_id);

    std::vector<GiftEntity> GetAllGifts();

    void SendGift(const std::string& from_id, const std::string& to_id,
                  int32_t gift_id, int32_t quantity, const std::string& message,
                  bool is_anonymous, int32_t total_value);

    std::vector<GiftRecordEntity> GetUserGifts(
        const std::string& user_id, int limit, int offset);

    int GetUserGiftCount(const std::string& user_id);
    int GetUserGiftTotalValue(const std::string& user_id);

    void AddArtistReview(const std::string& reviewer_id, const std::string& artist_id,
                         int64_t commission_id, int32_t rating,
                         const std::string& comment, const std::vector<std::string>& tags);

    std::vector<ArtistReviewEntity> GetArtistReviews(
        const std::string& artist_id, int limit, int offset);

    int GetArtistReviewCount(const std::string& artist_id);
    double GetArtistAverageRating(const std::string& artist_id);

    void UpdateArtistRating(const std::string& artist_id);

    void ManageArtistBlacklist(const std::string& user_id, const std::string& artist_id,
                               bool add, const std::string& reason);

    std::vector<BlacklistEntryEntity> GetArtistBlacklist(const std::string& user_id);

    int64_t CreateQuestionBox(const std::string& owner_id, bool is_public,
                              bool allow_anonymous, const std::string& description);

    std::vector<QuestionBoxEntity> GetQuestionBoxes(
        const std::string& user_id, int limit, int offset);

    int GetQuestionBoxCount(const std::string& user_id);

    int64_t AskQuestion(int64_t box_id, const std::string& asker_id,
                        const std::string& content, bool is_anonymous);

    void AnswerQuestion(int64_t question_id, const std::string& answer, bool is_public);

    std::vector<QuestionEntity> GetQuestions(
        int64_t box_id, const std::string& viewer_id, bool only_unanswered,
        int limit, int offset);

    int GetQuestionCount(int64_t box_id, bool only_unanswered);

    std::optional<std::string> GetQuestionBoxOwner(int64_t box_id);

private:
    SocialRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_SOCIAL_REPOSITORY_H
