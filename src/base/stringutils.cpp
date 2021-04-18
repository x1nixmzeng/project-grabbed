#include "stringutils.h"

#include <algorithm>

namespace grabbed::base::stringutils
{
    bool readLine(const string& source, size_t& offset, size_t& size)
    {
        offset += size;

        for (; offset < source.size(); )
        {
            auto pos = source.find("\r\n", offset);
            if (pos == source.npos) {
                if (offset < source.size()) {
                    size = source.size() - offset;
                    return true;
                }

                return false;
            }

            auto thisSize{ pos - offset };
            if (thisSize > 0) { // todo: check for all whitespace
                size = thisSize;
                return true;
            }

            offset = pos + 2;
        }

        return false;
    }

    bool canDowncast(wchar_t chr)
    {
        return chr >= L'\x0' && chr < L'\x7f';
    }

    char downcast(wchar_t chr)
    {
        return canDowncast(chr) ? static_cast<char>(chr & 0x7f) : '?';
    }

    string stringFromWide(const wstring& source)
    {
        string result;
        result.resize(source.size());
        std::transform(source.begin(), source.end(), result.begin(), downcast);

        return result;
    }

    void toUpper(string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), std::toupper);
    }

    void toUpper(wstring& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), std::toupper);
    }

    void toLower(string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), std::tolower);
    }

    void toLower(wstring& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), std::tolower);
    }
}
