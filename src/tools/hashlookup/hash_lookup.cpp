#include "hash_lookup.h"

#include "base/filestream.h"
#include "base/fileutils.h"
#include "base/stringutils.h"

#include "ghoulies/hashutils.h"
#include "kameo/hashutils.h"

#include "base/output.h"
#include "base/assert.h"

namespace grabbed
{
    namespace
    {
        size_t getTotalCount(const hashutils::type& resources)
        {
            return resources.size();
        }

        size_t getResolvedCount(const hashutils::type& resources)
        {
            size_t result{ 0 };

            for (const auto& resource : resources) {
                if (!resource.second.source.empty()) {
                    ++result;
                }
            }

            return result;
        }

        void outputHashStats(const hashutils::type& resources)
        {
            size_t allHashes{ getTotalCount(resources) };
            size_t knownHashes{ getResolvedCount(resources) };
            
            f32 resolvedPerc{ 100.0f / allHashes * knownHashes };
            
            output("Hash Count:     %u\n", allHashes);
            output("Resolved Count: %u\n", knownHashes);
            output("Resolved Perc:  %.2f%%\n", resolvedPerc);
        }
    }

    void hashlookup::loadLooseGhoulies(const std::string& path)
    {
        base::filestream fs(path);

        if (fs.isOpen()) {
            hashutils::loadHashFile(fs, m_ghouliesHashes, ghoulies::hashutils::makeHash);
        }
    }

    void hashlookup::loadLooseKameo(const std::string& path)
    {
        base::filestream fs(path);

        if (fs.isOpen()) {
            hashutils::loadHashFile(fs, m_kameoHashes, kameo::hashutils::makeHash);
        }
    }

    void hashlookup::loadLoosePinata(const std::string& path)
    {
        base::filestream fs(path);

        if (fs.isOpen()) {
            hashutils::loadHashFile(fs, m_pinataHashes, ghoulies::hashutils::makeHash);
        }
    }
    
    void hashlookup::outputStats()
    {
        if (!m_ghouliesHashes.empty()) {
            output("Grabbed by the Ghoulies\n");
            outputHashStats(m_ghouliesHashes);
        }

        if (!m_kameoHashes.empty()) {
            output("Kameo: Elements of Power\n");
            outputHashStats(m_kameoHashes);
        }

        if (!m_pinataHashes.empty()) {
            output("Viva Piñata\n");
            outputHashStats(m_pinataHashes);
        }

        output("\n");
    }

    bool hashlookup::lookupString(const std::string& filename)
    {
        bool resolved{ false };

        std::string cleaned;

        auto lastSlash = filename.find_last_of('/');
        if (lastSlash != filename.npos) {
            cleaned = filename.substr(lastSlash + 1);
        }
        else {
            cleaned = filename;
        }

        if (cleaned.size() > 0)
        {
            auto ghouliesHash = ghoulies::hashutils::makeHash(cleaned);

            printf("%08x\n", ghouliesHash);

            if (ghouliesHash != ghoulies::hashutils::s_invalidHash) {
                if (lookupHash("ghoulies", ghouliesHash, cleaned, m_ghouliesHashes)) {
                    resolved = true;
                }

                if(lookupHash("pinata", ghouliesHash, cleaned, m_pinataHashes)) {
                    resolved = true;
                }
            }
            
            auto kameoHash = kameo::hashutils::makeHash(cleaned);
            if (kameoHash != kameo::hashutils::s_invalidHash) {
                if(lookupHash("kameo", kameoHash, cleaned, m_kameoHashes)) {
                    resolved = true;
                }
            }

            if (!resolved) {
                output_fail("Failed to resolve %s\n", cleaned.c_str());
            }
        }

        return resolved;
    }

    bool hashlookup::lookupHash(const char* origin, u32 hash, const std::string& source, const hashutils::type& hashes)
    {
        auto result{ hashes.find(hash) };

        if (result != hashes.end()) {

            // Check the result actually has a string
            if (result->second.source.empty()) {
                // If the source is not a hash, the key is discovered!
                if (source[0] != '0') {
                    output("Discovered hash for %s with key '%s' hash '%0x'", origin, source.c_str(), hash);
                    assert_always("Discovered hash");
                    return true;
                }
                else {
                    // If the source was a hash, the key is still unknown
                    return false;
                }
            }

            // Ignore string hashes and check the source string
            if (source[0] != '0') {
                if (source.compare(result->second.source) != 0) {
                    assert_always("Hash was resolved but the text does not match the known source (a collision occured)");
                    output_fail("Collision for %s between '%s' and '%s'", origin, result->second.source, source.c_str());
                }
            }

            output("Resolved %s hash %08X to %s\n", origin, hash, result->second.source.c_str());
            return true;
        }
        else
        {
            return false;
        }
    }
    
}
