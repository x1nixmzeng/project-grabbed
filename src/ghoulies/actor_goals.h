#pragma once

#include "ghoulies/formats.h"

#include "base/types.h"
#include "base/stream.h"

namespace grabbed
{
    namespace ghoulies
    {
        class ActorGoalsReader : public BaseFormat
        {
        public:
            ActorGoalsReader();

            virtual bool read(base::stream& stream, Context& context) override;

        };
    }
}
