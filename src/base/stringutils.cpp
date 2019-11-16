#include "stringutils.h"

namespace grabbed
{
    namespace base
    {
        namespace stringutils
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

            string stringFromWide(const wstring& source)
            {
                string result;
                result.reserve(source.size());

                for (const wchar_t& chr : source) {
                    if (::iswascii(chr) != 0) {
                        result.append(1, static_cast<char>(chr & 0xFF));
                    }
                    else {
                        result.append(1, '?');
                    }
                }

                return result;
            }
        }
    }
}