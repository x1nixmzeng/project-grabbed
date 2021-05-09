#pragma once

#include "ghoulies/formats.h"

#include "base/types.h"
#include "base/stream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        struct Anim
        {
            float duration;
            u32 boneCount;
        };

        struct AnimDB
        {
            std::map<std::string, Anim> data;
        };

        class AnimReader : public BaseFormat
        {
        public:
            AnimReader(std::shared_ptr<AnimDB> data);

            virtual bool read(base::stream& stream, Context& context) override;
            virtual bool canAdd(const std::string& name) const override;

        private:
            std::shared_ptr<AnimDB> m_data;
        };
    }
}
