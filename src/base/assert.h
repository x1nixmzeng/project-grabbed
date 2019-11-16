#pragma once

#include "base/types.h"

namespace grabbed
{
    bool confirm(const char* title, const char* function, unsigned int line, const char* context);
    bool hasDebugger();

#define assert_true(expression, ...)                                        \
    if((expression) != true)                                                \
    {                                                                       \
        if (confirm("assert_true",                                          \
                    __FUNCTION__, __LINE__, #expression)) {                 \
            __debugbreak();                                                 \
        }                                                                   \
    }

#define assert_true_once(expression, ...)                                   \
    if((expression) != true)                                                \
    {                                                                       \
        static bool has_fired = false;                                      \
        if (!has_fired) {                                                   \
            if (confirm("assert_true_once",                                 \
                        __FUNCTION__, __LINE__, #expression)) {             \
                __debugbreak();                                             \
            }                                                               \
            has_fired = true;                                               \
        }                                                                   \
    }

#define assert_false(expression, ...)                                       \
    if((expression) != false)                                               \
    {                                                                       \
        if (confirm("assert_false",                                         \
                    __FUNCTION__, __LINE__, #expression)) {                 \
            __debugbreak();                                                 \
        }                                                                   \
    }

#define assert_false_once(expression, ...)                                  \
    if((expression) != false)                                               \
    {                                                                       \
        static bool has_fired = false;                                      \
        if (!has_fired) {                                                   \
            if (confirm("assert_false_once",                                \
                        __FUNCTION__, __LINE__, #expression)) {             \
                __debugbreak();                                             \
            }                                                               \
            has_fired = true;                                               \
        }                                                                   \
    }

#define assert_always(message, ...)                                         \
    if (confirm("assert_always",                                            \
                __FUNCTION__, __LINE__, message))                           \
    {                                                                       \
        __debugbreak();                                                     \
    }

#define assert_always_once(message, ...)                                    \
    {                                                                       \
        static bool has_fired = false;                                      \
        if (!has_fired) {                                                   \
            if (confirm("assert_always_once",                               \
                    __FUNCTION__, __LINE__, message))                       \
            {                                                               \
                __debugbreak();                                             \
            }                                                               \
            has_fired = true;                                               \
        }                                                                   \
    }

#define assert_for_debugger()                                               \
    if (hasDebugger()) {                                                    \
        __debugbreak();                                                     \
    }

}
