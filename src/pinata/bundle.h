#pragma once

#include "base/types.h"
#include "base/stream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace pinata
    {
        class BundleDB
        {
        public:
            std::map<std::string, std::wstring> data;
        };

        class BundleReader
        {
        public:
            BundleReader(std::shared_ptr<BundleDB> data);

            bool read(base::stream& stream);

        protected:
            std::shared_ptr<BundleDB> m_data;
        };
    }
}
