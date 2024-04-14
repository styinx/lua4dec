#include "errors.hpp"

// clang-format off
std::unordered_map<Error, std::string> ERROR_TO_STR = {
    {Error::NONE,                    "NONE"},
    {Error::SIGNATURE_MISMATCH,      "SIGNATURE_MISMATCH"},
    {Error::FUNCTION_PARAM_MISMATCH, "FUNCTION_PARAM_MISMATCH"},
    {Error::EMPTY_STACK,             "EMPTY_STACK"},
    {Error::BAD_VARIANT,             "BAD_VARIANT"},
    {Error::UNDEFINED,               "UNDEFINED"},
};
// clang-format on
