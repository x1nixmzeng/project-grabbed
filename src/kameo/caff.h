#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/texturetypes.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace kameo
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
            bool checkType(base::stream& stream, const string& desiredType);

        protected:
            std::shared_ptr<CaffDB> m_data;
        };
    }
}
