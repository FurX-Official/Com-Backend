#include "../service_impl/verification_service.h"
#include "../common/result.h"

namespace furbbs::controller {

using ::trpc::ServerContextPtr;
using namespace furbbs::repository;

::trpc::Status RealNameVerify(ServerContextPtr context,
                                const ::furbbs::RealNameVerifyRequest* request,
                                ::furbbs::RealNameVerifyResponse* response) {
    try {
        auto result = service::VerificationService::Instance().VerifyRealName(
            request->access_token(), request->name(), request->id_card_number());
        response->set_success(result.success);
        response->set_message(result.message);
        response->set_is_verified(result.is_verified);
        response->set_is_matched(result.is_matched);
        response->set_task_id(result.task_id);
        response->set_confidence_level(result.confidence_level);
        response->set_code(result.success ? RESULT_OK : RESULT_ERROR);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("验证失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status FaceVerify(ServerContextPtr context,
                           const ::furbbs::FaceVerifyRequest* request,
                           ::furbbs::FaceVerifyResponse* response) {
    try {
        auto result = service::VerificationService::Instance().VerifyFace(
            request->access_token(), request->image_url());
        response->set_success(result.success);
        response->set_message(result.message);
        response->set_is_passed(result.is_passed);
        response->set_similarity(result.similarity);
        response->set_task_id(result.task_id);
        response->set_code(result.success ? RESULT_OK : RESULT_ERROR);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("人脸验证失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status FaceCompare(ServerContextPtr context,
                            const ::furbbs::FaceCompareRequest* request,
                            ::furbbs::FaceCompareResponse* response) {
    try {
        auto result = service::VerificationService::Instance().CompareFace(
            request->access_token(), request->face_image_url(), request->id_card_image_url());
        response->set_success(result.success);
        response->set_message(result.message);
        response->set_is_passed(result.is_passed);
        response->set_similarity(result.similarity);
        response->set_task_id(result.task_id);
        response->set_best_face_url("");
        response->set_code(result.success ? RESULT_OK : RESULT_ERROR);
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("人证比对失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status GetVerifyStatus(ServerContextPtr context,
                                const ::furbbs::GetVerifyStatusRequest* request,
                                ::furbbs::GetVerifyStatusResponse* response) {
    try {
        auto status = service::VerificationService::Instance().GetVerifyStatus(
            request->access_token(), request->user_id());
        response->set_is_verified(status.is_verified);
        response->set_face_verified(status.face_verified);
        response->set_verified_at(status.verified_at);
        response->set_verify_provider(status.verify_provider);
        response->set_retry_count(status.retry_count);
        response->set_code(RESULT_OK);
        response->set_message("获取成功");
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("获取状态失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

::trpc::Status QueryVerifyResult(ServerContextPtr context,
                                  const ::furbbs::QueryVerifyResultRequest* request,
                                  ::furbbs::QueryVerifyResultResponse* response) {
    try {
        auto result = service::VerificationService::Instance().QueryVerifyResult(
            request->access_token(), request->task_id());
        if (result) {
            response->set_success(result->success);
            response->set_message(result->message);
            response->set_is_verified(result->is_verified);
            response->set_is_matched(result->is_matched);
            response->set_confidence_level(result->confidence_level);
            response->set_code(result->success ? RESULT_OK : RESULT_ERROR);
        } else {
            response->set_code(RESULT_ERROR);
            response->set_message("查询失败或任务不存在");
        }
        return ::trpc::kSuccStatus;
    } catch (const std::exception& e) {
        response->set_code(RESULT_ERROR);
        response->set_message(std::string("查询失败: ") + e.what());
        return ::trpc::kSuccStatus;
    }
}

} // namespace furbbs::controller
