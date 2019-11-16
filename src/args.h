#pragma once

#include "base/types.h"

#include <map>

namespace grabbed
{
    namespace args
    {
        using type = std::vector<string>;

        struct splittype
        {
            std::map<string, string> mapped;
            std::vector<string> loose;

            bool read(const string& name, string& result) const;
            bool existsLoose(const string& name) const;
        };

        splittype splitArgs(const type& args);

        type createArgs(int argc, char** argv);
        type createArgs(string& argList);
        splittype createSplitArgs(int argc, char** argv);
    }
}
