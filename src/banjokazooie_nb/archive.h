#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/memorystream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace banjokazooie_nb
    {
        struct ArchiveItem
        {
            u32 id;
            base::memorystream data;
        };

        class ArchiveDB
        {
        public:
            std::vector<ArchiveItem> files;
        };

        class ArchiveReader
        {
        public:
            ArchiveReader(std::shared_ptr<ArchiveDB> data);

            bool read(base::stream& stream);

        protected:
            std::shared_ptr<ArchiveDB> m_data;
        };
    }
}
