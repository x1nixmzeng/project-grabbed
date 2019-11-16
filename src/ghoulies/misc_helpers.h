#pragma once

#include "ghoulies/formats.h"

#include "base/stream.h"

namespace grabbed
{
    namespace ghoulies
    {
        class MiscIntroData
        {
        public:
            void read(base::stream& stream, Context& context);
        };

        class MiscLightVolumeTable
        {
        public:
            void read(base::stream& stream, Context& context);
        };

        class MiscPartTable
        {
        public:
            void read(base::stream& stream, Context& context);
        };
    }
}
