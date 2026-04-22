#ifndef FURBBS_SERVICE_VERIFICATION_SERVICE_H
#define FURBBS_SERVICE_VERIFICATION_SERVICE_H

#include "../repository/verification_repository.h"
#include <string>
#include <vector>
#include <optional>

namespace furbbs::service {

struct VerifyResult {
    bool success = false;
    std::string message;
    bool is_verified = false;
    bool is_matched = false;
    std::string task_id;
    int32_t confidence_level = 0;
};

struct FaceVerifyResult {
    bool success = false;
    std::string message;
    bool is_passed = false;
    float similarity = 0.0f;
    std::string task_id;
};

struct VerifyStatus {
    bool is_verified = false;
    bool face_verified = false;
    int64_t verified_at = 0;
    std::string verify_provider;
    int32_t retry_count = 0;
};

class VerificationService {
public:
    static VerificationService& Instance();

    VerificationService(const VerificationService&) = delete;
    VerificationService& operator=(const VerificationService&) = delete;

    void Initialize();

    VerifyResult VerifyRealName(const std::string& token,
                                 const std::string& name,
                                 const std::string& id_card_number);

    FaceVerifyResult VerifyFace(const std::string& token,
                                 const std::string& image_url);

    FaceVerifyResult CompareFace(const std::string& token,
                                  const std::string& face_image_url,
                                  const std::string& id_card_image_url);

    VerifyStatus GetVerifyStatus(const std::string& token, const std::string& user_id = "");

    std::optional<VerifyResult> QueryVerifyResult(const std::string& token,
                                                   const std::string& task_id);

    bool CanVerify(const std::string& user_id, std::string& out_reason);

private:
    VerificationService() = default;
    ~VerificationService() = default;

    std::string MaskIdCardNumber(const std::string& id_card_number);
    std::string MaskName(const std::string& name);

    bool initialized_ = false;
    int64_t retry_time_window_ = 86400;
    int max_retry_count_ = 5;
};

} // namespace furbbs::service

#endif // FURBBS_SERVICE_VERIFICATION_SERVICE_H
