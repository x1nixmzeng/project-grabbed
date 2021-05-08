#pragma once

#include "base/types.h"

#include <map>

namespace grabbed
{
    namespace args
    {
        using type = std::vector<std::string>;

        struct splittype
        {
            std::map<std::string, std::string> mapped;
            std::vector<std::string> loose;

            bool read(const std::string& name, std::string& result) const;
            bool existsLoose(const std::string& name) const;
        };

        splittype splitArgs(const type& args);

        type createArgs(int argc, char** argv);
        type createArgs(std::string& argList);
        splittype createSplitArgs(int argc, char** argv);
    }
}
