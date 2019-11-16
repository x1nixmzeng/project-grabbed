#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/texturetypes.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace conker_lr
    {
        struct CaffEntry
        {
            buffer data;
            size_t width = 0;
            size_t height = 0;
            XboxD3DFormat format;
        };

        class CaffDB
        {
        public:
            std::map<string, CaffEntry> data;
        };

        class CaffReader
        {
        public:
            CaffReader(std::shared_ptr<CaffDB> data);

            bool read(base::stream& stream);

        protected:
            std::shared_ptr<CaffDB> m_data;
        };
    }
}
