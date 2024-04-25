#pragma once

#include <exception>
#include <string>


namespace STIP::errors {

    class STIPException : public std::exception {
    private:
        std::string message;

    public:
        explicit STIPException(const char *msg) : message(msg) {}

        [[nodiscard]] const char *what() const noexcept override {
            return message.c_str();
        }
    };

    class STIPTimeoutException : public STIPException {
    public:
        STIPTimeoutException() : STIPException("Timeout exception") {}

        explicit STIPTimeoutException(const char *msg) : STIPException(msg) {}

    };

}


