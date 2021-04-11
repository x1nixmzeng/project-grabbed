#include "demand_info.h"

#include "ghoulies/formats.h"
#include "ghoulies/hashutils.h"

#include "base/output.h"

namespace grabbed
{
    demandInfo::demandInfo()
        : m_textureDb(std::make_shared<ghoulies::TextureDB>())
        , m_formatRegistry(m_textureDb)
    {
    }

    std::unique_ptr<base::filestream> demandInfo::openFile(const string& filename)
    {
        return std::make_unique<base::filestream>(filename);
    }

    string resolveFile(u32 hash)
    {
        auto resolved = [=]() -> const char*
        {
            switch (hash)
            {
#define CSV_MACRO(context, namehash, name) case namehash: return name;
#include "ghoulies/hashlookup.hpp"
            default:
                return nullptr;
            };
        }();

        if (resolved)
            return { resolved };

        return {};
    }

    void demandInfo::parseAll(const string& path)
    {
        std::filesystem::path root{ path };
        if (std::filesystem::exists(root)) {
            std::filesystem::directory_iterator dir{ root };
            for (auto& file : dir) {
                parseFile(file.path());
            }
        }
    }

    void demandInfo::parseFile(std::filesystem::path filePath)
    {
        auto entry = openFile(filePath.generic_string());
        if (entry) {
            auto fullName{ filePath.filename().generic_string() };

            auto hash = ghoulies::hashutils::makeHash(fullName);
            if (hash != ghoulies::hashutils::s_invalidHash) {
                auto resolved = resolveFile(hash);
                if (resolved.length() > 0) {
                    fullName = resolved;
                }

                m_formatRegistry.readLoose(*entry, fullName.c_str());
            }
        }
    }
}
