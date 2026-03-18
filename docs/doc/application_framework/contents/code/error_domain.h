/*!********************************************************************************************************************
 *  COPYRIGHT
 *  -------------------------------------------------------------------------------------------------------------------
 *
 *  Copyright (c) 2025 by Vector Informatik GmbH. All rights reserved.
 *
 *                This software is copyright protected and proprietary to Vector Informatik GmbH.
 *                Vector Informatik GmbH grants to you only those rights as set out in the license conditions.
 *                All other rights remain with Vector Informatik GmbH.
 *
 *  -------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -----------------------------------------------------------------------------------------------------------------*/
/*!        file  error_domain.h
 *         brief
 *
 *********************************************************************************************************************/
#ifndef VAF_ERROR_DOMAIN_H_
#define VAF_ERROR_DOMAIN_H_

#include <string>

namespace vaf {

    enum class ErrorCode {
        kDefaultErrorCode = 1,
        kServiceNotFound,
        kServiceModelMissMatch,
        kServiceLost,
        kNoSampleAvailable,
        kServiceNotRunning,
        kNoOperationHandlerRegistered
    };

    class Error {
    public:
        Error(ErrorCode error_code, std::string message) : error_code_{error_code}, message_{message} {}

        const std::string Message() const noexcept {
            return std::string{std::to_string(static_cast<int>(error_code_)) + ": " + message_};
        }
        
        const std::string UserMessage() const noexcept {
            return message_;
        }

        Error(const Error &) = default;
        Error(Error &&) = default;

        Error &operator=(const Error &) = default;
        Error &operator=(Error &&) = default;

    private:
        ErrorCode error_code_;
        std::string message_;

    };


}  // namespace vaf

#endif  // VAF_ERROR_DOMAIN_H_
