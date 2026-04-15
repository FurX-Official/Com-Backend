#include "social_service.h"
#include "repository/user_repository.h"

namespace furbbs::service {

bool SocialService::FollowUser(const std::string& token, 
                                 const std::string& target_id, bool follow) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || user_opt->id == target_id) {
        return false;
    }
    return repository::SocialRepository::Instance().SetFollow(
        user_opt->id, target_id, follow);
}

std::vector<repository::FollowInfoEntity> SocialService::GetFollowing(
    const std::string& user_id, int page, int page_size, int& out_total) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::SocialRepository::Instance().GetFollowingCount(user_id);
    return repository::SocialRepository::Instance().GetFollowing(
        user_id, page_size, offset);
}

std::vector<repository::FollowInfoEntity> SocialService::GetFollowers(
    const std::string& user_id, int page, int page_size, int& out_total) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::SocialRepository::Instance().GetFollowerCount(user_id);
    return repository::SocialRepository::Instance().GetFollowers(
        user_id, page_size, offset);
}

std::vector<PostEntity> SocialService::GetFriendCircle(
    const std::string& token, int page, int page_size, int& out_total) {

    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        out_total = 0;
        return {};
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    auto post_ids = repository::SocialRepository::Instance().GetFriendCirclePosts(
        user_opt->id, page_size, offset);

    out_total = static_cast<int>(post_ids.size());

    std::vector<PostEntity> posts;
    for (int64_t id : post_ids) {
        auto post = repository::PostRepository::Instance().GetPost(id);
        if (post) posts.push_back(*post);
    }
    return posts;
}

bool SocialService::FavoriteFursona(const std::string& token, 
                                      int64_t fursona_id, bool favorite) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return false;
    }
    return repository::SocialRepository::Instance().SetFursonaFavorite(
        user_opt->id, fursona_id, favorite);
}

std::vector<FursonaEntity> SocialService::GetFavoriteFursonas(
    const std::string& user_id, int page, int page_size,
    int& out_total, int& out_favorite_count) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    auto fursona_ids = repository::SocialRepository::Instance().GetFavoriteFursonas(
        user_id, page_size, offset);

    out_total = repository::SocialRepository::Instance().GetFavoriteFursonaCount(user_id);
    out_favorite_count = out_total;

    std::vector<FursonaEntity> fursonas;
    for (int64_t id : fursona_ids) {
        auto f = repository::FursonaRepository::Instance().GetFursona(id);
        if (f) fursonas.push_back(*f);
    }
    return fursonas;
}

std::vector<repository::GiftEntity> SocialService::GetGiftList() {
    return repository::SocialRepository::Instance().GetAllGifts();
}

bool SocialService::SendGift(const std::string& token, const std::string& target_id,
                             int32_t gift_id, int32_t quantity, const std::string& message,
                             bool is_anonymous, int32_t& out_cost, std::string& out_gift_name) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || user_opt->id == target_id) {
        return false;
    }

    auto gifts = repository::SocialRepository::Instance().GetAllGifts();
    repository::GiftEntity selected;
    bool found = false;
    for (const auto& g : gifts) {
        if (g.id == gift_id) {
            selected = g;
            found = true;
            break;
        }
    }
    if (!found) return false;

    quantity = std::max(1, quantity);
    out_cost = selected.price * quantity;
    out_gift_name = selected.name;

    auto stats = repository::UserRepository::Instance().GetUserStats(user_opt->id);
    if (!stats || stats->points < out_cost) {
        return false;
    }

    repository::UserRepository::Instance().AddPoints(user_opt->id, -out_cost);
    repository::SocialRepository::Instance().SendGift(
        user_opt->id, target_id, gift_id, quantity, message, is_anonymous, out_cost);
    return true;
}

std::vector<repository::GiftRecordEntity> SocialService::GetUserGifts(
    const std::string& user_id, int page, int page_size,
    int& out_total, int& out_total_value) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::SocialRepository::Instance().GetUserGiftCount(user_id);
    out_total_value = repository::SocialRepository::Instance().GetUserGiftTotalValue(user_id);

    return repository::SocialRepository::Instance().GetUserGifts(
        user_id, page_size, offset);
}

double SocialService::RateArtist(const std::string& token, const std::string& artist_id,
                                 int64_t commission_id, int32_t rating,
                                 const std::string& comment, const std::vector<std::string>& tags) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || user_opt->id == artist_id) {
        return -1;
    }
    rating = std::min(5, std::max(1, rating));

    repository::SocialRepository::Instance().AddArtistReview(
        user_opt->id, artist_id, commission_id, rating, comment, tags);

    return repository::SocialRepository::Instance().GetArtistAverageRating(artist_id);
}

std::vector<repository::ArtistReviewEntity> SocialService::GetArtistReviews(
    const std::string& artist_id, int page, int page_size,
    int& out_total, double& out_avg_rating) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::SocialRepository::Instance().GetArtistReviewCount(artist_id);
    out_avg_rating = repository::SocialRepository::Instance().GetArtistAverageRating(artist_id);

    return repository::SocialRepository::Instance().GetArtistReviews(
        artist_id, page_size, offset);
}

std::vector<repository::BlacklistEntryEntity> SocialService::ManageArtistBlacklist(
    const std::string& token, const std::string& artist_id,
    bool add, const std::string& reason) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return {};
    }

    repository::SocialRepository::Instance().ManageArtistBlacklist(
        user_opt->id, artist_id, add, reason);

    return repository::SocialRepository::Instance().GetArtistBlacklist(user_opt->id);
}

int64_t SocialService::CreateQuestionBox(const std::string& token, bool is_public,
                                          bool allow_anonymous, 
                                          const std::string& description) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return 0;
    }
    return repository::SocialRepository::Instance().CreateQuestionBox(
        user_opt->id, is_public, allow_anonymous, description);
}

std::vector<repository::QuestionBoxEntity> SocialService::GetQuestionBoxes(
    const std::string& user_id, int page, int page_size, int& out_total) {

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::SocialRepository::Instance().GetQuestionBoxCount(user_id);
    return repository::SocialRepository::Instance().GetQuestionBoxes(
        user_id, page_size, offset);
}

int64_t SocialService::AskQuestion(const std::string& token, int64_t box_id,
                                    const std::string& content, bool is_anonymous) {
    std::string asker_id;
    if (!token.empty() && !is_anonymous) {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user_opt) asker_id = user_opt->id;
    }

    if (content.empty() || content.length() > 2000) {
        return 0;
    }

    return repository::SocialRepository::Instance().AskQuestion(
        box_id, asker_id, content, is_anonymous);
}

bool SocialService::AnswerQuestion(const std::string& token, int64_t question_id,
                                    const std::string& answer, bool is_public) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt || answer.empty() || answer.length() > 5000) {
        return false;
    }

    auto box_id = 0;
    auto owner = repository::SocialRepository::Instance().GetQuestionBoxOwner(box_id);
    if (!owner || *owner != user_opt->id) {
        return false;
    }

    repository::SocialRepository::Instance().AnswerQuestion(
        question_id, answer, is_public);
    return true;
}

std::vector<repository::QuestionEntity> SocialService::GetQuestions(
    const std::string& token, int64_t box_id, bool only_unanswered,
    int page, int page_size, int& out_total) {

    std::string viewer_id;
    if (!token.empty()) {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (user_opt) viewer_id = user_opt->id;
    }

    page = std::max(1, page);
    page_size = std::min(100, std::max(1, page_size > 0 ? page_size : 20));
    int offset = (page - 1) * page_size;

    out_total = repository::SocialRepository::Instance().GetQuestionCount(
        box_id, only_unanswered);

    return repository::SocialRepository::Instance().GetQuestions(
        box_id, viewer_id, only_unanswered, page_size, offset);
}

} // namespace furbbs::service
