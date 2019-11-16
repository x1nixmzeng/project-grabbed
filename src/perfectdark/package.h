#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/texturetypes.h"

#include "base/memorystream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace perfectdark
    {
        struct PackageData
        {
            base::memorystream stream;
        };

        class PackageReader
        {
        public:
            PackageReader(std::shared_ptr<PackageData> data);
            bool read(base::stream& stream);

        protected:
            std::shared_ptr<PackageData> m_data;
        };
    }
}
