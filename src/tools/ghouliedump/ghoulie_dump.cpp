#include "ghoulie_dump.h"

#include "ghoulies/formats.h"
#include "ghoulies/hashutils.h"
#include "ghoulies/bundle.h"
#include "ghoulies/model.h"

#include "base/output.h"

namespace grabbed
{
    namespace
    {
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
    }

    ghoulieDump::ghoulieDump()
        : m_textureDb(std::make_shared<ghoulies::TextureDB>())
        , m_formatRegistry(m_textureDb)
    {
    }

    std::unique_ptr<base::filestream> ghoulieDump::openFile(const string& filename)
    {
        return std::make_unique<base::filestream>(filename);
    }

    void ghoulieDump::parseAll(const string& path)
    {
        std::filesystem::path root{ path };
        if (std::filesystem::exists(root)) {
            std::filesystem::directory_iterator dir{ root };
            for (auto& file : dir) {
                parseFile(file.path());
            }
        }
    }

    void ghoulieDump::parseFile(std::filesystem::path filePath)
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
            else {
                parseBundle(*entry);
            }
        }
    }

    void ghoulieDump::parseBundle(base::stream& stream)
    {
        auto ghouliesBundle = std::make_shared<ghoulies::BundleDb>();

        class BundleProvider : public ghoulies::IBundleProvider
        {
        public:
            std::shared_ptr<ghoulies::TextureDB> m_textureData;

            BundleProvider(std::shared_ptr<ghoulies::TextureDB> textureData)
                : m_textureData(textureData)
            { }

            virtual bool readBundle(base::stream& stream, ghoulies::Context& context)
            {
                if (context.header.type == ghoulies::ResourceType::eResTexture) {
                    ghoulies::TextureReader textureReader(m_textureData);
                    // todo: remove and expect bundle formats to seek to where they want
                    stream.seek(context.getFileOffset());
                    return textureReader.readBundled(stream, context);
                }
                else if (context.header.type == ghoulies::ResourceType::eResModel) {
                    auto modelData = std::make_shared<ghoulies::ModelDb>();
                    ghoulies::ModelReader modelReader(modelData, m_textureData);
                    modelReader.setOnlyTextures(true);

                    // todo: remove and expect bundle formats to seek to where they want
                    stream.seek(context.getFileOffset());
                    return modelReader.readBundled(stream, context);
                }

                // unhandled
                return false;
            }
        };

        auto bundleProvider = std::make_shared<BundleProvider>(m_textureDb);

        ghoulies::BundleReader reader(bundleProvider, ghouliesBundle);
        reader.read(stream);
    }

    void ghoulieDump::iterateTextures(std::function<void(ghoulies::TextureData& data)> method)
    {
        std::for_each(m_textureDb->m_storage.begin(), m_textureDb->m_storage.end(), method);
    }
}
