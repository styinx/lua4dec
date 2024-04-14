
#ifndef LUA4DEC_ERRORS_H
#define LUA4DEC_ERRORS_H

#include <string>
#include <unordered_map>

/*
 * Error handling
 */

enum class Error : unsigned
{
    NONE = 0x00,
    SIGNATURE_MISMATCH,
    ARCHITECTURE_MISMATCH,
    FUNCTION_PARAM_MISMATCH,
    EMPTY_STACK,
    BAD_VARIANT,
    UNDEFINED,
};

extern std::unordered_map<Error, std::string> ERROR_TO_STR;

void quit_on(const bool condition, const Error error, const char* message);

#endif  // LUA4DEC_ERRORS_H
