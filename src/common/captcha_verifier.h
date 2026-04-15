#ifndef FURBBS_COMMON_CAPTCHA_VERIFIER_H
#define FURBBS_COMMON_CAPTCHA_VERIFIER_H

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace furbbs::common {

struct CaptchaResult {
    bool success = false;
    float score = 0.0f;
    std::vector<std::string> error_codes;
};

class CaptchaVerifier {
public:
    static CaptchaVerifier& Instance();

    CaptchaVerifier(const CaptchaVerifier&) = delete;
    CaptchaVerifier& operator=(const CaptchaVerifier&) = delete;

    std::optional<CaptchaResult> Verify(const std::string& provider, 
                                        const std::string& response_token,
                                        const std::string& remote_ip = "");

private:
    CaptchaVerifier() = default;
    ~CaptchaVerifier() = default;

    std::optional<CaptchaResult> VerifyReCaptcha(const std::string& secret, 
                                                 const std::string& response_token,
                                                 const std::string& remote_ip,
                                                 const std::string& verify_url);
    
    std::optional<CaptchaResult> VerifyHCaptcha(const std::string& secret, 
                                                const std::string& response_token,
                                                const std::string& remote_ip,
                                                const std::string& verify_url);
    
    std::optional<CaptchaResult> VerifyGeeTest(const std::string& secret, 
                                               const std::string& response_token,
                                               const std::string& remote_ip,
                                               const std::string& verify_url);

    std::string HttpPost(const std::string& url, const std::string& post_data);
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_CAPTCHA_VERIFIER_H
