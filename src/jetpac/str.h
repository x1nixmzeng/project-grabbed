#pragma once

#include "base/types.h"
#include "base/stream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace jetpac
    {
        class StrDB
        {
        public:
            std::map<std::string, std::string> data;
            std::string debugName;
        };

        class StrReader
        {
        public:
            StrReader(std::shared_ptr<StrDB> data);

            bool read(base::stream& stream);

        protected:
            std::shared_ptr<StrDB> m_data;
        };
    }
}
