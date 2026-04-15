#ifndef FURBBS_SERVICE_IMPL_SOCIAL_SERVICE_H
#define FURBBS_SERVICE_IMPL_SOCIAL_SERVICE_H

#include "repository/social_repository.h"
#include "repository/post_repository.h"
#include "repository/fursona_repository.h"
#include "auth/casdoor_auth.h"
#include <vector>
#include <string>

namespace furbbs::service {

class SocialService {
public:
    static SocialService& Instance() {
        static SocialService instance;
        return instance;
    }

    bool FollowUser(const std::string& token, const std::string& target_id, bool follow);

    std::vector<repository::FollowInfoEntity> GetFollowing(
        const std::string& user_id, int page, int page_size, int& out_total);

    std::vector<repository::FollowInfoEntity> GetFollowers(
        const std::string& user_id, int page, int page_size, int& out_total);

    std::vector<PostEntity> GetFriendCircle(
        const std::string& token, int page, int page_size, int& out_total);

    bool FavoriteFursona(const std::string& token, int64_t fursona_id, bool favorite);

    std::vector<FursonaEntity> GetFavoriteFursonas(
        const std::string& user_id, int page, int page_size,
        int& out_total, int& out_favorite_count);

    std::vector<repository::GiftEntity> GetGiftList();

    bool SendGift(const std::string& token, const std::string& target_id,
                  int32_t gift_id, int32_t quantity, const std::string& message,
                  bool is_anonymous, int32_t& out_cost, std::string& out_gift_name);

    std::vector<repository::GiftRecordEntity> GetUserGifts(
        const std::string& user_id, int page, int page_size,
        int& out_total, int& out_total_value);

    double RateArtist(const std::string& token, const std::string& artist_id,
                      int64_t commission_id, int32_t rating,
                      const std::string& comment, const std::vector<std::string>& tags);

    std::vector<repository::ArtistReviewEntity> GetArtistReviews(
        const std::string& artist_id, int page, int page_size,
        int& out_total, double& out_avg_rating);

    std::vector<repository::BlacklistEntryEntity> ManageArtistBlacklist(
        const std::string& token, const std::string& artist_id,
        bool add, const std::string& reason);

    int64_t CreateQuestionBox(const std::string& token, bool is_public,
                              bool allow_anonymous, const std::string& description);

    std::vector<repository::QuestionBoxEntity> GetQuestionBoxes(
        const std::string& user_id, int page, int page_size, int& out_total);

    int64_t AskQuestion(const std::string& token, int64_t box_id,
                        const std::string& content, bool is_anonymous);

    bool AnswerQuestion(const std::string& token, int64_t question_id,
                        const std::string& answer, bool is_public);

    std::vector<repository::QuestionEntity> GetQuestions(
        const std::string& token, int64_t box_id, bool only_unanswered,
        int page, int page_size, int& out_total);

private:
    SocialService() = default;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_IMPL_SOCIAL_SERVICE_H
