#pragma once

#include "base/types.h"

#include <vector>

namespace grabbed
{
    namespace base
    {
        class streamstats
        {
        protected:
            using infoPair = std::pair<size_t, size_t>;
            std::vector<infoPair> m_info;

        public:

            void read(size_t pos, size_t length);

            size_t getTotalRead() const;
        };
    }
}
