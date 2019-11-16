#pragma once

#include "base/stream.h"
#include "base/types.h"

namespace grabbed
{
    namespace base
    {
        namespace fileutils
        {
            void saveToDisk(stream& stream, size_t length, string filename);
        }
    }
}
