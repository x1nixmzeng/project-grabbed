#pragma once

#include "base/stream.h"

#include "ghoulies/formats.h"

#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        class CutsceneDb
        {
        public:

        };

        class CutsceneReader : public BaseFormat
        {
        public:
            CutsceneReader(std::shared_ptr<CutsceneDb> data);

            virtual bool read(base::stream& stream, Context& context) override;

        private:
            std::shared_ptr<CutsceneDb> m_data;
        };
    }
}
