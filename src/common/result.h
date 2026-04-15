#ifndef FURBBS_COMMON_RESULT_H
#define FURBBS_COMMON_RESULT_H

#include <string>
#include <optional>
#include <system_error>

namespace furbbs::common {

enum class ErrorCode {
    SUCCESS = 0,
    INVALID_PARAMETER = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    RATE_LIMITED = 429,
    INTERNAL_ERROR = 500,
    DATABASE_ERROR = 501,
    VALIDATION_ERROR = 502,
};

template<typename T>
class Result {
public:
    static Result<T> Ok(T&& value) {
        return Result(std::forward<T>(value));
    }

    static Result<T> Ok(const T& value) {
        return Result(value);
    }

    static Result<T> Err(ErrorCode code, const std::string& message) {
        return Result(code, message);
    }

    bool IsOk() const { return !error_; }
    bool IsErr() const { return error_.has_value(); }

    T& Unwrap() {
        if (error_) {
            throw std::system_error(static_cast<int>(error_->first), std::generic_category(), error_->second);
        }
        return *value_;
    }

    const T& Unwrap() const {
        if (error_) {
            throw std::system_error(static_cast<int>(error_->first), std::generic_category(), error_->second);
        }
        return *value_;
    }

    ErrorCode Code() const {
        return error_ ? error_->first : ErrorCode::SUCCESS;
    }

    const std::string& Message() const {
        static const std::string empty;
        return error_ ? error_->second : empty;
    }

private:
    explicit Result(const T& value) : value_(value) {}
    explicit Result(T&& value) : value_(std::move(value)) {}
    Result(ErrorCode code, const std::string& message) : error_(std::make_pair(code, message)) {}

    std::optional<T> value_;
    std::optional<std::pair<ErrorCode, std::string>> error_;
};

template<>
class Result<void> {
public:
    static Result<void> Ok() {
        return Result();
    }

    static Result<void> Err(ErrorCode code, const std::string& message) {
        return Result(code, message);
    }

    bool IsOk() const { return !error_; }
    bool IsErr() const { return error_.has_value(); }

    void Unwrap() const {
        if (error_) {
            throw std::system_error(static_cast<int>(error_->first), std::generic_category(), error_->second);
        }
    }

    ErrorCode Code() const {
        return error_ ? error_->first : ErrorCode::SUCCESS;
    }

    const std::string& Message() const {
        static const std::string empty;
        return error_ ? error_->second : empty;
    }

private:
    Result() = default;
    Result(ErrorCode code, const std::string& message) : error_(std::make_pair(code, message)) {}

    std::optional<std::pair<ErrorCode, std::string>> error_;
};

class ErrorHelper {
public:
    static std::string ErrorCodeToString(ErrorCode code);
    static int ToHttpStatus(ErrorCode code);
    static int ToProtoStatus(ErrorCode code);
};

} // namespace furbbs::common

#endif // FURBBS_COMMON_RESULT_H
