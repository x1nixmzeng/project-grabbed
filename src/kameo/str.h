#pragma once

#include "base/types.h"
#include "base/stream.h"

#include "ghoulies/texture.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace kameo
    {
        class StrDB
        {
        public:
            std::map<i16, std::wstring> data;
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
