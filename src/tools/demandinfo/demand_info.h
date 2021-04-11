#pragma once

#include "ghoulies/bundle_formats.h"
#include "ghoulies/texture.h"

#include "base/filestream.h"
#include "base/types.h"

#include <memory>
#include <filesystem>

#include "args.h"

namespace grabbed
{
    class demandInfo
    {
    public:
        demandInfo();

        std::unique_ptr<base::filestream> openFile(const string& filename);

        void parseAll(const string& path);

        void parseFile(std::filesystem::path filePath);

    private:
        std::shared_ptr<ghoulies::TextureDB> m_textureDb;
        ghoulies::BundleFormatRegistry m_formatRegistry;
    };

    namespace tool
    {
        int main(args::splittype& args)
        {
            string result;
            if (args.read("path", result)) {
                demandInfo demandInfo;
                demandInfo.parseAll(result);
            }
            
            for(const auto& arg : args.loose) {
                demandInfo demandInfo;
                demandInfo.parseFile(arg);
            }

            return 0;
        }
    }
}
