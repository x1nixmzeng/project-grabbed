#pragma once

#include "base/types.h"

namespace grabbed
{
    namespace base
    {
        namespace stringutils
        {
            bool readLine(const string& source, size_t& offset, size_t& size);

            string stringFromWide(const wstring& source);
        }
    }
}
