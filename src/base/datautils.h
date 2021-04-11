#pragma once

#include "base/types.h"
#include "base/assert.h"

namespace grabbed::base::datautils
{
    // Ensure this whole string is printable
    void sanityCheck(string& str);

    // Ensure this whole c-string is printable (sized-sized array)
    template <size_t N>
    void sanityCheck(char(&str)[N])
    {
        size_t i{ 0 };

        while (i < N) {
            if (str[i] == 0) break;
            if (std::isprint(str[i]) == 0) {
                assert_always("String failed a sanity check (there are unprintable characters)");
            }
            ++i;
        }
    }
}
