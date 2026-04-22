#include "verification_service.h"
#include "../common/netease_verify.h"
#include "../config/config.h"
#include "../auth/auth.h"
#include <spdlog/spdlog.h>

namespace furbbs::service {

VerificationService& VerificationService::Instance() {
    static VerificationService instance;
    return instance;
}

void VerificationService::Initialize() {
    auto& config = config::Config::Instance().GetNeteaseVerifyConfig();
    if (config.enabled) {
        common::NeteaseVerify::Instance().Initialize(
            config.secret_id,
            config.secret_key,
            config.business_id
        );
        initialized_ = true;
        spdlog::info("Netease Verify initialized successfully");
    } else {
        spdlog::info("Netease Verify is disabled in config");
    }
}

bool VerificationService::CanVerify(const std::string& user_id, std::string& out_reason) {
    auto& config = config::Config::Instance().GetNeteaseVerifyConfig();
    if (!config.enabled) {
        out_reason = "实名认证功能未启用";
        return false;
    }

    auto existing = repository::VerificationRepository::Instance().GetLatestVerification(user_id);
    if (existing && existing->is_verified) {
        out_reason = "用户已完成实名认证";
        return false;
    }

    int retry_count = repository::VerificationRepository::Instance().GetVerifyRetryCount(
        user_id, retry_time_window_);
    if (retry_count >= max_retry_count_) {
        out_reason = "验证次数过多，请24小时后再试";
        return false;
    }

    return true;
}

std::string VerificationService::MaskName(const std::string& name) {
    if (name.empty()) {
        return name;
    }
    if (name.size() == 1) {
        return name;
    }
    if (name.size() == 2) {
        return std::string(1, name[0]) + "*";
    }
    return std::string(1, name[0]) + std::string(name.size() - 2, '*') + name.back();
}

std::string VerificationService::MaskIdCardNumber(const std::string& id_card_number) {
    if (id_card_number.size() < 10) {
        return id_card_number;
    }
    return id_card_number.substr(0, 6) + "********" + id_card_number.substr(14);
}

VerifyResult VerificationService::VerifyRealName(const std::string& token,
                                                   const std::string& name,
                                                   const std::string& id_card_number) {
    VerifyResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "无效的访问令牌";
        return result;
    }

    std::string reason;
    if (!CanVerify(user_opt->id, reason)) {
        result.success = false;
        result.message = reason;
        return result;
    }

    repository::RealNameVerificationEntity entity;
    entity.user_id = user_opt->id;
    entity.name = name;
    entity.id_card_number = id_card_number;
    entity.verify_provider = "netease";
    entity.status = "pending";
    entity.retry_count = 0;

    int64_t verification_id = repository::VerificationRepository::Instance().CreateVerification(entity);
    if (verification_id == 0) {
        result.success = false;
        result.message = "创建验证记录失败";
        return result;
    }

    auto verify_result = common::NeteaseVerify::Instance().VerifyRealName(
        name, id_card_number, user_opt->id);

    result.success = verify_result.success;
    result.message = verify_result.reason;
    result.is_verified = verify_result.is_verified;
    result.is_matched = verify_result.is_matched;
    result.task_id = verify_result.task_id;
    result.confidence_level = verify_result.confidence_level;

    repository::VerificationRepository::Instance().UpdateVerificationResult(
        verification_id,
        verify_result.is_verified,
        verify_result.is_matched,
        verify_result.confidence_level,
        verify_result.status,
        verify_result.reason,
        verify_result.task_id,
        verify_result.transaction_id
    );

    if (result.is_verified) {
        result.message = "实名认证成功";
    } else if (!result.success) {
        repository::VerificationRepository::Instance().IncrementRetryCount(user_opt->id);
    }

    return result;
}

FaceVerifyResult VerificationService::VerifyFace(const std::string& token,
                                                   const std::string& image_url) {
    FaceVerifyResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "无效的访问令牌";
        return result;
    }

    auto& config = config::Config::Instance().GetNeteaseVerifyConfig();
    if (!config.enabled) {
        result.success = false;
        result.message = "人脸识别功能未启用";
        return result;
    }

    auto latest_verify = repository::VerificationRepository::Instance().GetLatestVerification(user_opt->id);
    if (!latest_verify || !latest_verify->is_verified) {
        result.success = false;
        result.message = "请先完成实名认证";
        return result;
    }

    repository::FaceVerificationRecordEntity record;
    record.user_id = user_opt->id;
    record.image_url = image_url;

    auto compare_result = common::NeteaseVerify::Instance().CompareFace(
        image_url, image_url, user_opt->id);

    result.success = compare_result.success;
    result.is_passed = compare_result.is_same_person;
    result.similarity = compare_result.similarity;
    result.task_id = compare_result.task_id;
    result.message = compare_result.is_same_person ? "人脸验证通过" : "人脸验证不通过";

    record.task_id = compare_result.task_id;
    record.similarity = compare_result.similarity;
    record.is_passed = compare_result.is_same_person;
    record.best_face_url = compare_result.best_face_url;

    repository::VerificationRepository::Instance().RecordFaceVerify(record);

    return result;
}

FaceVerifyResult VerificationService::CompareFace(const std::string& token,
                                                    const std::string& face_image_url,
                                                    const std::string& id_card_image_url) {
    FaceVerifyResult result;
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        result.success = false;
        result.message = "无效的访问令牌";
        return result;
    }

    auto& config = config::Config::Instance().GetNeteaseVerifyConfig();
    if (!config.enabled) {
        result.success = false;
        result.message = "人脸识别功能未启用";
        return result;
    }

    repository::FaceVerificationRecordEntity record;
    record.user_id = user_opt->id;
    record.image_url = face_image_url;

    auto compare_result = common::NeteaseVerify::Instance().CompareFace(
        face_image_url, id_card_image_url, user_opt->id);

    result.success = compare_result.success;
    result.is_passed = compare_result.is_same_person;
    result.similarity = compare_result.similarity;
    result.task_id = compare_result.task_id;
    result.message = compare_result.is_same_person ? "人证比对通过" : "人证比对不通过";

    record.task_id = compare_result.task_id;
    record.similarity = compare_result.similarity;
    record.is_passed = compare_result.is_same_person;
    record.best_face_url = compare_result.best_face_url;

    repository::VerificationRepository::Instance().RecordFaceVerify(record);

    return result;
}

VerifyStatus VerificationService::GetVerifyStatus(const std::string& token,
                                                    const std::string& user_id) {
    VerifyStatus status;

    std::string target_user_id = user_id;
    if (target_user_id.empty()) {
        auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
        if (!user_opt) {
            return status;
        }
        target_user_id = user_opt->id;
    }

    auto latest_verify = repository::VerificationRepository::Instance().GetLatestVerification(target_user_id);
    if (latest_verify) {
        status.is_verified = latest_verify->is_verified;
        status.face_verified = latest_verify->face_verified;
        status.verified_at = latest_verify->verified_at;
        status.verify_provider = latest_verify->verify_provider;
        status.retry_count = latest_verify->retry_count;
    }

    return status;
}

std::optional<VerifyResult> VerificationService::QueryVerifyResult(const std::string& token,
                                                                     const std::string& task_id) {
    auto user_opt = auth::CasdoorAuth::Instance().VerifyToken(token);
    if (!user_opt) {
        return std::nullopt;
    }

    auto& config = config::Config::Instance().GetNeteaseVerifyConfig();
    if (!config.enabled) {
        return std::nullopt;
    }

    auto netease_result = common::NeteaseVerify::Instance().GetVerifyResult(task_id);
    if (!netease_result) {
        return std::nullopt;
    }

    VerifyResult result;
    result.success = netease_result->success;
    result.message = netease_result->reason;
    result.is_verified = netease_result->is_verified;
    result.is_matched = netease_result->is_matched;
    result.task_id = netease_result->task_id;
    result.confidence_level = netease_result->confidence_level;

    auto verification = repository::VerificationRepository::Instance().GetVerificationByTaskId(task_id);
    if (verification) {
        repository::VerificationRepository::Instance().UpdateVerificationResult(
            verification->id,
            netease_result->is_verified,
            netease_result->is_matched,
            netease_result->confidence_level,
            netease_result->status,
            netease_result->reason,
            task_id,
            netease_result->transaction_id
        );
    }

    return result;
}

} // namespace furbbs::service
