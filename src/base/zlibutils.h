#pragma once

#include "base/memorystream.h"
#include "base/types.h"

namespace grabbed
{
    namespace base
    {
        namespace zlibutils
        {
            // Decompress to preallocated memory stream (faster, requires uncompressed size)
            bool decompress(buffer& in, base::memorystream& msOut);

            // Decompress all to empty buffer
            bool decompress(buffer& in, buffer& out);
        }
    }
}
