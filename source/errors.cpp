#include "errors.hpp"

// clang-format off
std::unordered_map<Status, std::string> STATUS_TO_STR = {
    {Status::OK,                    "NONE"},
    {Status::SIGNATURE_MISMATCH,      "SIGNATURE_MISMATCH"},
    {Status::FUNCTION_PARAM_MISMATCH, "FUNCTION_PARAM_MISMATCH"},
    {Status::EMPTY_STACK,             "EMPTY_STACK"},
    {Status::BAD_VARIANT,             "BAD_VARIANT"},
    {Status::UNDEFINED,               "UNDEFINED"},
};
// clang-format on
