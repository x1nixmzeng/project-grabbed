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
            u32 makeFileNameHash(const string& filename);

            // Convert from string hash to hash
            u32 getHashFromString(const string& hashString);

            // Convert from string/string hash to hash
            u32 makeHash(const string& val);

            // Convert from string/string hash to string hash
            string makeHashString(const string& val);

            // Convert from hash to string hash
            string makeStringFromHash(u32 hash);
        }

    }
}
