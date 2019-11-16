#pragma once

#include "ghoulies/formats.h"

#include "base/stream.h"

namespace grabbed
{
    namespace ghoulies
    {
        class IBundleProvider
        {
        public:
            virtual ~IBundleProvider() = default;
            virtual bool readBundle(base::stream& stream, Context& context) = 0;
        };
    }
}

