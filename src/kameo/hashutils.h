#pragma once

#include "base/types.h"

namespace grabbed
{
    namespace kameo
    {
        namespace hashutils
        {
            constexpr u32 s_invalidHash = 0;

            // Makes hash from aid filename
            constexpr u32 makeFileNameHashStatic(const char* filename);

            // Makes hash from aid filename
            u32 makeFileNameHash(const std::string& filename);

            // Convert from string hash to hash
            u32 getHashFromString(const std::string& hashString);

            // Convert from string/string hash to hash
            u32 makeHash(const std::string& val);

            // Convert from string/string hash to string hash
            std::string makeHashString(const std::string& val);

            // Convert from hash to string hash
            std::string makeStringFromHash(u32 hash);
        }

    }
}
