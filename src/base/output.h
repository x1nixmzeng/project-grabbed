#pragma once

#include "base/types.h"

#include <array>
#include <string>

namespace grabbed
{
    enum class OutputType
    {
        Info,
        Log,
        Warning,
        Error,
        Failure,
    };

    template<typename... Args>
    void output(OutputType type, const char* format, Args... args)
    {
        switch (type)
        {
        case OutputType::Info:
        case OutputType::Log:
            fprintf(stdout, format, args...);
            break;
        case OutputType::Warning:
        case OutputType::Error:
        case OutputType::Failure:
            fprintf(stderr, format, args...);
            break;
        }
    }

    template<typename... Args>
    void output(const char* format, Args... args)
    {
        output(OutputType::Info, format, args...);
    }

    template<typename... Args>
    std::string output_string(const char* format, Args... args)
    {
        std::array<char, 512> stringBuffer;
        sprintf_s(stringBuffer.data(), stringBuffer.max_size(), format, args...);
        
        return string(stringBuffer.begin(), stringBuffer.end());
    }

    template<typename... Args>
    void output_log(const char* format, Args... args)
    {
        output(OutputType::Log, format, args...);
    }

    template<typename... Args>
    void output_error(const char* format, Args... args)
    {
        output(OutputType::Error, format, args...);
    }

    template<typename... Args>
    void output_warn(const char* format, Args... args)
    {
        output(OutputType::Warning, format, args...);
    }

    template<typename... Args>
    void output_fail(const char* format, Args... args)
    {
        output(OutputType::Failure, format, args...);
    }
}
