#pragma once

#include "base/stream.h"
#include "base/types.h"

namespace grabbed::base::fileutils
{
    void saveToDisk(stream& stream, size_t length, const std::string& filename);
    void saveToDisk(const buffer& buffer, const std::string& filename);
}
