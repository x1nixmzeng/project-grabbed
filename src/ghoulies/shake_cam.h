#pragma once

#include "ghoulies/formats.h"

#include "base/types.h"
#include "base/stream.h"

namespace grabbed
{
    namespace ghoulies
    {
        class ShakeCamReader : public BaseFormat
        {
        public:
            ShakeCamReader();

            virtual bool read(base::stream& stream, Context& context) override;

        };
    }
}
