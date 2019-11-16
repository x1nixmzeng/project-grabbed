#pragma once

#include "base/types.h"
#include "base/stream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace perfectdark
    {
        class StrDB
        {
        public:
            std::map<string, wstring> data;
            string debugName;
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
