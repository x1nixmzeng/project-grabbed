#include "hashutils.h"

#include "base/output.h"
#include "base/assert.h"

#include <algorithm>

namespace grabbed
{
    namespace ghoulies
    {
        namespace hashutils
        {
            namespace
            {
                void testForLooseResource(const std::string& source, u32 hash)
                {
#if _DEBUG
#define CSV_MACRO(context, hash, name) case hash: if(name[0] == 0) { output("Discovered hash '%s' (" context ") == %08X\n", source.c_str(), hash); } break;

                    switch (hash)
                    {
#include "hashlookup.hpp"

                    default:
                        break;
                    }
#endif
                }
            }

            namespace
            {
                constexpr u32 makeStaticHash(u32 hash, const char* filename, size_t index)
                {
                    if (filename[index])
                    {
                        if (index > 3)
                        {
                            hash = (hash << 4) + (filename[index] & 0xDF);

                            u32 hiBit = (hash & 0xF0000000U);
                            if (hiBit) {
                                hash ^= hiBit | (hiBit >> 24);
                            }
                        }

                        return makeStaticHash(hash, filename, index + 1);
                    }
                    else
                    {
                        return hash;
                    }
                }

                constexpr auto aid_filename = makeStaticHash(0, "aid_texture_ghoulies_scoreboard_eventicons_dontgethit", 0);
                static_assert(aid_filename == 0x00559fa4, "Failed to get static hash");

                u32 getCharValue(const char c)
                {
                    if ((c >= '0') && (c <= '9')) {
                        return c - '0';
                    }

                    if ((c >= 'a') && (c <= 'f')) {
                        return 10 + (c - 'a');
                    }

                    // we don't really allow uppercaser, but here we go:
                    if ((c >= 'A') && (c <= 'F')) {
                        return 10 + (c - 'A');
                    }

                    return 0;
                }
            }

            constexpr u32 makeFileNameHashStatic(const char* filename)
            {
                return makeStaticHash(0, filename, 0);
            }

            u32 makeFileNameHash(const std::string& filename)
            {
                u32 hash = 0;

                assert_true(filename.substr(0, 4) == "aid_", "Incorrect filename prefix");

                // Skip prefix
                const char *str(filename.c_str());
                str += 4;

                for (; *str; ++str)
                {
                    hash = 16 * hash + (*str & 0xDF);

                    u32 hiBit = (hash & 0xF0000000);
                    if (hiBit != 0) {
                        hash ^= hiBit | (hiBit >> 24);
                    }
                }

                testForLooseResource(filename, hash);

                return hash;
            }

            u32 getHashFromString(const std::string& hashString)
            {
                u32 hash = s_invalidHash;

                hash |= (getCharValue(hashString[0]) & 0xF) << 28;
                hash |= (getCharValue(hashString[1]) & 0xF) << 24;
                hash |= (getCharValue(hashString[2]) & 0xF) << 20;
                hash |= (getCharValue(hashString[3]) & 0xF) << 16;
                hash |= (getCharValue(hashString[4]) & 0xF) << 12;
                hash |= (getCharValue(hashString[5]) & 0xF) << 8;
                hash |= (getCharValue(hashString[6]) & 0xF) << 4;
                hash |= (getCharValue(hashString[7]) & 0xF);

                return hash;
            }

            u32 makeHash(const std::string& val)
            {
                assert_true(!val.empty());
                const bool is_already_hash(val[0] == '0' && val.size() == 8);

                if (is_already_hash) {
                    return getHashFromString(val);
                }
                else if (val.substr(0, 4) == "aid_") {
                    return makeFileNameHash(val);
                }
                else {
                    return s_invalidHash;
                }
            }

            std::string makeHashString(const std::string& val)
            {
                const u32 hash(makeHash(val));

                return makeStringFromHash(hash);
            }

            std::string makeStringFromHash(u32 hash)
            {
                std::string str;
                str.resize(8);
                
                const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

                str[0] = hex[(hash >> 28) & 0xf];
                str[1] = hex[(hash >> 24) & 0xf];
                str[2] = hex[(hash >> 20) & 0xf];
                str[3] = hex[(hash >> 16) & 0xf];
                str[4] = hex[(hash >> 12) & 0xf];
                str[5] = hex[(hash >> 8) & 0xf];
                str[6] = hex[(hash >> 4) & 0xf];
                str[7] = hex[(hash >> 0) & 0xf];

                return str;
            }
        }
    }
}
