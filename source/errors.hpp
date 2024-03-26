
#ifndef LUA4DEC_ERRORS_H
#define LUA4DEC_ERRORS_H

/*
 * Error handling
 */

enum class Error : int
{
    NONE = 0x00,
    SIGNATURE_MISMATCH,
    ARCHITECTURE_MISMATCH,
    FUNCTION_PARAM_MISMATCH,
    EMPTY_STACK,
    BAD_VARIANT,
    UNDEFINED,
};

void quit_on(const bool condition, const Error error, const char* message);

#endif  // LUA4DEC_ERRORS_H
