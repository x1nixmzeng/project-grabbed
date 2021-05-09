#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/texturetypes.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace perfectdark
    {
        struct CaffEntry
        {
        };

        class CaffDB
        {
        public:
            std::map<std::string, CaffEntry> data;
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
