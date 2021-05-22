#pragma once

#include "base/types.h"

#include <string_view>

namespace grabbed::base::stringutils
{
    bool readLine(const std::string& source, size_t& offset, size_t& size);

    std::string stringFromWide(const std::wstring_view& source);

    std::string stringFromPtr(wchar_t* pString);
    std::string stringFromPtrBe(wchar_t* pString);

    void toUpper(std::string& str);
    void toUpper(std::wstring& str);
    void toLower(std::string& str);
    void toLower(std::wstring& str);

    // Helper to create a tight string view around a known-length char array
    template <size_t N>
    std::string_view makeStringView(char(&str)[N])
    {
        auto view{ std::string_view(str, N) };

        auto firstNull = view.find('\0');
        if (firstNull != view.npos) {
            // Trim to first null
            return view.substr(0, firstNull);
        }

        return view;
    }
}
