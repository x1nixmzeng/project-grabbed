#pragma once

#include "base/stream.h"

#include <map>
#include <functional>

namespace grabbed
{
    namespace hashutils
    {
        struct data
        {
            std::string source;
            std::string context;
        };

        using type = std::map<hash, data>;
        using delegate = std::function<hash(const std::string&)>;

        bool loadHashFile(base::stream& stream, type& output, const delegate& method);
    }
}
