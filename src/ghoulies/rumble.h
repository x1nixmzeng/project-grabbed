#pragma once

#include "ghoulies/formats.h"

#include "base/types.h"
#include "base/stream.h"

namespace grabbed
{
    namespace ghoulies
    {
        class RumbleReader : public BaseFormat
        {
        public:
            RumbleReader();

            virtual bool read(base::stream& stream, Context& context) override;
        };
    }
}
