#pragma once

#include "base/types.h"

namespace grabbed::base::stringutils
{
    bool readLine(const string& source, size_t& offset, size_t& size);

    string stringFromWide(const wstring& source);

    void toUpper(string& str);
    void toUpper(wstring& str);
    void toLower(string& str);
    void toLower(wstring& str);
}
