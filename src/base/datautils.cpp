#include "datautils.h"

namespace grabbed::base::datautils
{
    void sanityCheck(std::string& str)
    {
        for (auto& c : str) {
            if (std::isprint(c) == 0) {
                assert_always("String failed a sanity check (there are unprintable characters)");
            }
        }
    }
}
