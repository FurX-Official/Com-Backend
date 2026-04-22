#ifndef FURBBS_REPOSITORY_VERIFICATION_REPOSITORY_H
#define FURBBS_REPOSITORY_VERIFICATION_REPOSITORY_H

#include "base_repository.h"
#include <string>
#include <vector>
#include <optional>

namespace furbbs::repository {

struct RealNameVerificationEntity {
    int64_t id = 0;
    std::string user_id;
    std::string name;
    std::string id_card_number;
    bool is_verified = false;
    bool is_matched = false;
    int32_t confidence_level = 0;
    std::string task_id;
    std::string transaction_id;
    std::string status;
    std::string reason;
    bool face_verified = false;
    float face_similarity = 0.0f;
    std::string verify_provider;
    int64_t created_at = 0;
    int64_t verified_at = 0;
    int32_t retry_count = 0;
};

struct FaceVerificationRecordEntity {
    int64_t id = 0;
    std::string user_id;
    std::string task_id;
    std::string image_url;
    float similarity = 0.0f;
    bool is_passed = false;
    std::string best_face_url;
    int64_t created_at = 0;
};

class VerificationRepository : protected BaseRepository {
public:
    static VerificationRepository& Instance() {
        static VerificationRepository instance;
        return instance;
    }

    int64_t CreateVerification(const RealNameVerificationEntity& verification);

    bool UpdateVerificationResult(int64_t verification_id,
                                   bool is_verified,
                                   bool is_matched,
                                   int32_t confidence_level,
                                   const std::string& status,
                                   const std::string& reason,
                                   const std::string& task_id,
                                   const std::string& transaction_id);

    std::optional<RealNameVerificationEntity> GetLatestVerification(const std::string& user_id);

    std::optional<RealNameVerificationEntity> GetVerificationByTaskId(const std::string& task_id);

    std::vector<RealNameVerificationEntity> GetUserVerifications(
        const std::string& user_id, int limit, int offset);

    bool IsUserVerified(const std::string& user_id);

    int64_t RecordFaceVerify(const FaceVerificationRecordEntity& record);

    std::vector<FaceVerificationRecordEntity> GetUserFaceRecords(
        const std::string& user_id, int limit, int offset);

    int GetVerifyRetryCount(const std::string& user_id, int64_t time_window);

    bool IncrementRetryCount(const std::string& user_id);

private:
    VerificationRepository() = default;
};

} // namespace furbbs::repository

#endif // FURBBS_REPOSITORY_VERIFICATION_REPOSITORY_H
