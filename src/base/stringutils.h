#pragma once

#include "base/types.h"

namespace grabbed::base::stringutils
{
    bool readLine(const std::string& source, size_t& offset, size_t& size);

    std::string stringFromWide(const std::wstring& source);

    void toUpper(std::string& str);
    void toUpper(std::wstring& str);
    void toLower(std::string& str);
    void toLower(std::wstring& str);
}
