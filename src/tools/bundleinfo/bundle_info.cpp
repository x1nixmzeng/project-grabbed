#include "bundle_info.h"

#include "ghoulies/formats.h"
#include "ghoulies/bundle_formats.h"

#include "base/output.h"

namespace grabbed
{
    bundleinfo::bundleinfo()
        : bundleResources(std::make_shared<ghoulies::BundleDb>())
        , textureResources(std::make_shared<ghoulies::TextureDB>())
    {

    }

    std::unique_ptr<base::filestream> bundleinfo::openFile(const std::string& filename)
    {
        return std::make_unique<base::filestream>(filename);
    }

    void bundleinfo::readBundle(const std::string& filename)
    {
        auto file = openFile(filename);
        if (file->isOpen())
        {
            auto formatRegistry = std::make_shared<ghoulies::BundleFormatRegistry>(textureResources);

            auto reader = std::make_unique<ghoulies::BundleReader>(formatRegistry, bundleResources);
            reader->read(*file);
        }
    }

    void bundleinfo::listFiles()
    {
        for (const auto& fileInfo : bundleResources->m_files)
        {
            output("%08X\t%u\t%s\n", fileInfo.hash, fileInfo.resources.size(), fileInfo.name.c_str());
        }

        if (bundleResources->m_files.size() > 0)
        {
            output("Summary (%u files)\n", bundleResources->m_files.size());

            auto formatHelper = ghoulies::FormatHelper();
            for (const auto& typeInfo : bundleResources->m_types)
            {
                output("%u\t\t%s\n", typeInfo.second, formatHelper.toString(typeInfo.first));
            }
        }
    }
}
