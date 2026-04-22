#ifndef FURBBS_COMMON_NETEASE_VERIFY_H
#define FURBBS_COMMON_NETEASE_VERIFY_H

#include <string>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>

namespace furbbs::common {

struct RealNameVerifyResult {
    bool success = false;
    std::string task_id;
    std::string status;
    std::string reason;
    int32_t code = 0;
    std::string transaction_id;
    bool is_verified = false;
    bool is_matched = false;
    int32_t confidence_level = 0;
};

struct FaceCompareResult {
    bool success = false;
    std::string task_id;
    std::string status;
    float similarity = 0.0f;
    bool is_same_person = false;
    std::string best_face_url;
    int32_t code = 0;
};

struct IdCardOcrResult {
    bool success = false;
    std::string task_id;
    std::string name;
    std::string id_card_number;
    std::string gender;
    std::string ethnicity;
    std::string birthday;
    std::string address;
    std::string authority;
    std::string valid_date;
    int32_t type = 0;
    int32_t code = 0;
};

class NeteaseVerify {
public:
    static NeteaseVerify& Instance();

    NeteaseVerify(const NeteaseVerify&) = delete;
    NeteaseVerify& operator=(const NeteaseVerify&) = delete;

    void Initialize(const std::string& secret_id, const std::string& secret_key,
                    const std::string& business_id = "");

    RealNameVerifyResult VerifyRealName(const std::string& name,
                                         const std::string& id_card_number,
                                         const std::string& user_id = "");

    FaceCompareResult CompareFace(const std::string& image_url1,
                                  const std::string& image_url2,
                                  const std::string& user_id = "");

    IdCardOcrResult OcrIdCard(const std::string& image_url,
                              const std::string& card_side = "front",
                              const std::string& user_id = "");

    std::optional<RealNameVerifyResult> GetVerifyResult(const std::string& task_id);

    bool IsInitialized() const { return initialized_; }

private:
    NeteaseVerify() = default;
    ~NeteaseVerify() = default;

    std::string GenerateSignature(const std::string& method,
                                   const std::string& endpoint,
                                   const std::map<std::string, std::string>& params);

    std::string HttpPost(const std::string& url,
                          const std::map<std::string, std::string>& params);

    std::string HttpPostJson(const std::string& url,
                              const nlohmann::json& data);

    std::string GenerateTimestamp();
    std::string GenerateNonce();
    std::string Md5(const std::string& input);
    std::string Sha1Hex(const std::string& key, const std::string& input);

    std::string secret_id_;
    std::string secret_key_;
    std::string business_id_;
    bool initialized_ = false;

    static const std::string API_HOST;
    static const std::string REAL_NAME_VERIFY_ENDPOINT;
    static const std::string FACE_COMPARE_ENDPOINT;
    static const std::string IDCARD_OCR_ENDPOINT;
    static const std::string GET_RESULT_ENDPOINT;
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_NETEASE_VERIFY_H
