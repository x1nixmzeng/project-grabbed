#pragma once

#include "ghoulies/formats.h"

#include "base/types.h"
#include "base/stream.h"

#include <vector>

namespace grabbed
{
    namespace ghoulies
    {
        struct AnimEventSfx
        {
            string name;
        };

        struct AnimEventFrame
        {
            float time;
        };

        struct AnimEventsDB
        {
            std::vector<AnimEventFrame> data;
        };

        class AnimEventsReader : public BaseFormat
        {
        public:
            AnimEventsReader();

            virtual bool read(base::stream& stream, Context& context) override;

        private:
            bool onBlock(base::stream& stream, size_t size, f32 time);

            std::shared_ptr<AnimEventsDB> m_data;
        };
    }
}
