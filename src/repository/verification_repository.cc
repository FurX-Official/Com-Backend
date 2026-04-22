#include "verification_repository.h"
#include "../db/database.h"
#include <spdlog/spdlog.h>

namespace furbbs::repository {

int64_t VerificationRepository::CreateVerification(const RealNameVerificationEntity& verification) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "INSERT INTO real_name_verifications "
            "(user_id, name, id_card_number, is_verified, is_matched, "
            "confidence_level, task_id, transaction_id, status, reason, "
            "face_verified, face_similarity, verify_provider, created_at, retry_count) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15) "
            "RETURNING id",
            verification.user_id,
            verification.name,
            verification.id_card_number,
            verification.is_verified,
            verification.is_matched,
            verification.confidence_level,
            verification.task_id,
            verification.transaction_id,
            verification.status,
            verification.reason,
            verification.face_verified,
            verification.face_similarity,
            verification.verify_provider,
            static_cast<int64_t>(std::time(nullptr)),
            verification.retry_count
        );

        txn.commit();

        if (!result.empty()) {
            return result[0][0].as<int64_t>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Create verification error: {}", e.what());
    }
    return 0;
}

bool VerificationRepository::UpdateVerificationResult(int64_t verification_id,
                                                        bool is_verified,
                                                        bool is_matched,
                                                        int32_t confidence_level,
                                                        const std::string& status,
                                                        const std::string& reason,
                                                        const std::string& task_id,
                                                        const std::string& transaction_id) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        txn.exec_params(
            "UPDATE real_name_verifications SET "
            "is_verified = $1, is_matched = $2, confidence_level = $3, "
            "status = $4, reason = $5, task_id = $6, transaction_id = $7, "
            "verified_at = $8 "
            "WHERE id = $9",
            is_verified,
            is_matched,
            confidence_level,
            status,
            reason,
            task_id,
            transaction_id,
            static_cast<int64_t>(std::time(nullptr)),
            verification_id
        );

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Update verification error: {}", e.what());
        return false;
    }
}

std::optional<RealNameVerificationEntity> VerificationRepository::GetLatestVerification(
    const std::string& user_id) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT id, user_id, name, id_card_number, is_verified, "
            "is_matched, confidence_level, task_id, transaction_id, status, "
            "reason, face_verified, face_similarity, verify_provider, "
            "created_at, verified_at, retry_count "
            "FROM real_name_verifications "
            "WHERE user_id = $1 "
            "ORDER BY created_at DESC LIMIT 1",
            user_id
        );

        if (!result.empty()) {
            RealNameVerificationEntity entity;
            auto row = result[0];
            entity.id = row["id"].as<int64_t>();
            entity.user_id = row["user_id"].as<std::string>();
            entity.name = row["name"].as<std::string>();
            entity.id_card_number = row["id_card_number"].as<std::string>();
            entity.is_verified = row["is_verified"].as<bool>();
            entity.is_matched = row["is_matched"].as<bool>();
            entity.confidence_level = row["confidence_level"].as<int32_t>();
            entity.task_id = row["task_id"].as<std::string>();
            entity.transaction_id = row["transaction_id"].as<std::string>();
            entity.status = row["status"].as<std::string>();
            entity.reason = row["reason"].as<std::string>();
            entity.face_verified = row["face_verified"].as<bool>();
            entity.face_similarity = row["face_similarity"].as<float>();
            entity.verify_provider = row["verify_provider"].as<std::string>();
            entity.created_at = row["created_at"].as<int64_t>();
            entity.verified_at = row["verified_at"].as<int64_t>(0);
            entity.retry_count = row["retry_count"].as<int32_t>();
            return entity;
        }
    } catch (const std::exception& e) {
        spdlog::error("Get latest verification error: {}", e.what());
    }
    return std::nullopt;
}

std::optional<RealNameVerificationEntity> VerificationRepository::GetVerificationByTaskId(
    const std::string& task_id) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT id, user_id, name, id_card_number, is_verified, "
            "is_matched, confidence_level, task_id, transaction_id, status, "
            "reason, face_verified, face_similarity, verify_provider, "
            "created_at, verified_at, retry_count "
            "FROM real_name_verifications "
            "WHERE task_id = $1",
            task_id
        );

        if (!result.empty()) {
            RealNameVerificationEntity entity;
            auto row = result[0];
            entity.id = row["id"].as<int64_t>();
            entity.user_id = row["user_id"].as<std::string>();
            entity.name = row["name"].as<std::string>();
            entity.id_card_number = row["id_card_number"].as<std::string>();
            entity.is_verified = row["is_verified"].as<bool>();
            entity.is_matched = row["is_matched"].as<bool>();
            entity.confidence_level = row["confidence_level"].as<int32_t>();
            entity.task_id = row["task_id"].as<std::string>();
            entity.transaction_id = row["transaction_id"].as<std::string>();
            entity.status = row["status"].as<std::string>();
            entity.reason = row["reason"].as<std::string>();
            entity.face_verified = row["face_verified"].as<bool>();
            entity.face_similarity = row["face_similarity"].as<float>();
            entity.verify_provider = row["verify_provider"].as<std::string>();
            entity.created_at = row["created_at"].as<int64_t>();
            entity.verified_at = row["verified_at"].as<int64_t>(0);
            entity.retry_count = row["retry_count"].as<int32_t>();
            return entity;
        }
    } catch (const std::exception& e) {
        spdlog::error("Get verification by task_id error: {}", e.what());
    }
    return std::nullopt;
}

std::vector<RealNameVerificationEntity> VerificationRepository::GetUserVerifications(
    const std::string& user_id, int limit, int offset) {
    std::vector<RealNameVerificationEntity> result_list;
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT id, user_id, name, id_card_number, is_verified, "
            "is_matched, confidence_level, task_id, transaction_id, status, "
            "reason, face_verified, face_similarity, verify_provider, "
            "created_at, verified_at, retry_count "
            "FROM real_name_verifications "
            "WHERE user_id = $1 "
            "ORDER BY created_at DESC LIMIT $2 OFFSET $3",
            user_id, limit, offset
        );

        for (const auto& row : result) {
            RealNameVerificationEntity entity;
            entity.id = row["id"].as<int64_t>();
            entity.user_id = row["user_id"].as<std::string>();
            entity.name = row["name"].as<std::string>();
            entity.id_card_number = row["id_card_number"].as<std::string>();
            entity.is_verified = row["is_verified"].as<bool>();
            entity.is_matched = row["is_matched"].as<bool>();
            entity.confidence_level = row["confidence_level"].as<int32_t>();
            entity.task_id = row["task_id"].as<std::string>();
            entity.transaction_id = row["transaction_id"].as<std::string>();
            entity.status = row["status"].as<std::string>();
            entity.reason = row["reason"].as<std::string>();
            entity.face_verified = row["face_verified"].as<bool>();
            entity.face_similarity = row["face_similarity"].as<float>();
            entity.verify_provider = row["verify_provider"].as<std::string>();
            entity.created_at = row["created_at"].as<int64_t>();
            entity.verified_at = row["verified_at"].as<int64_t>(0);
            entity.retry_count = row["retry_count"].as<int32_t>();
            result_list.push_back(entity);
        }
    } catch (const std::exception& e) {
        spdlog::error("Get user verifications error: {}", e.what());
    }
    return result_list;
}

bool VerificationRepository::IsUserVerified(const std::string& user_id) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT 1 FROM real_name_verifications "
            "WHERE user_id = $1 AND is_verified = true "
            "ORDER BY created_at DESC LIMIT 1",
            user_id
        );

        return !result.empty();
    } catch (const std::exception& e) {
        spdlog::error("Check user verified error: {}", e.what());
        return false;
    }
}

int64_t VerificationRepository::RecordFaceVerify(const FaceVerificationRecordEntity& record) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "INSERT INTO face_verification_records "
            "(user_id, task_id, image_url, similarity, is_passed, best_face_url, created_at) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7) "
            "RETURNING id",
            record.user_id,
            record.task_id,
            record.image_url,
            record.similarity,
            record.is_passed,
            record.best_face_url,
            static_cast<int64_t>(std::time(nullptr))
        );

        txn.commit();

        if (!result.empty()) {
            return result[0][0].as<int64_t>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Record face verify error: {}", e.what());
    }
    return 0;
}

std::vector<FaceVerificationRecordEntity> VerificationRepository::GetUserFaceRecords(
    const std::string& user_id, int limit, int offset) {
    std::vector<FaceVerificationRecordEntity> result_list;
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT id, user_id, task_id, image_url, similarity, "
            "is_passed, best_face_url, created_at "
            "FROM face_verification_records "
            "WHERE user_id = $1 "
            "ORDER BY created_at DESC LIMIT $2 OFFSET $3",
            user_id, limit, offset
        );

        for (const auto& row : result) {
            FaceVerificationRecordEntity entity;
            entity.id = row["id"].as<int64_t>();
            entity.user_id = row["user_id"].as<std::string>();
            entity.task_id = row["task_id"].as<std::string>();
            entity.image_url = row["image_url"].as<std::string>();
            entity.similarity = row["similarity"].as<float>();
            entity.is_passed = row["is_passed"].as<bool>();
            entity.best_face_url = row["best_face_url"].as<std::string>();
            entity.created_at = row["created_at"].as<int64_t>();
            result_list.push_back(entity);
        }
    } catch (const std::exception& e) {
        spdlog::error("Get user face records error: {}", e.what());
    }
    return result_list;
}

int VerificationRepository::GetVerifyRetryCount(const std::string& user_id, int64_t time_window) {
    try {
        int64_t since = static_cast<int64_t>(std::time(nullptr)) - time_window;
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT COUNT(*) FROM real_name_verifications "
            "WHERE user_id = $1 AND created_at >= $2 AND is_verified = false",
            user_id, since
        );

        if (!result.empty()) {
            return result[0][0].as<int>();
        }
    } catch (const std::exception& e) {
        spdlog::error("Get retry count error: {}", e.what());
    }
    return 0;
}

bool VerificationRepository::IncrementRetryCount(const std::string& user_id) {
    try {
        auto conn = Database::Instance().GetConnection();
        pqxx::work txn(*conn);

        txn.exec_params(
            "UPDATE real_name_verifications SET retry_count = retry_count + 1 "
            "WHERE user_id = $1 "
            "ORDER BY created_at DESC LIMIT 1",
            user_id
        );

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Increment retry count error: {}", e.what());
        return false;
    }
}

} // namespace furbbs::repository
