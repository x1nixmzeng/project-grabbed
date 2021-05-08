#pragma once

#include "ghoulies/bundle_formats.h"
#include "ghoulies/texture.h"

#include "base/output.h"

#include "base/filestream.h"
#include "base/types.h"
#include "base/textureutils.h"

#include <memory>
#include <filesystem>
#include <functional>

#include "args.h"

#include "img/img.h"

namespace grabbed
{
    class ghoulieDump
    {
    public:
        ghoulieDump();

        std::unique_ptr<base::filestream> openFile(const std::string& filename);

        void parseAll(const std::string& path);
        void parseFile(std::filesystem::path filePath);

        void iterateTextures(std::function<void(ghoulies::TextureData& data)> method);

    private:
        void parseBundle(base::stream& stream);

    private:
        std::shared_ptr<ghoulies::TextureDB> m_textureDb;
        ghoulies::BundleFormatRegistry m_formatRegistry;
    };

    namespace tool
    {
        int main(args::splittype& args)
        {
            grabbed::output("ghouliedump\n");
            grabbed::output("Texture extractor for Grabbed by the Ghoulies\n");
            grabbed::output("Written by x1nixmzeng/WRS\n");
            
            ghoulieDump ghoulieDump;

            std::string result;
            if (args.read("path", result)) {
                grabbed::output("Scanning path %s...\n", result.c_str());
                ghoulieDump.parseAll(result);
            }
            
            for(const auto& arg : args.loose) {
                grabbed::output("Reading file %s...\n", arg.c_str());
                ghoulieDump.parseFile(arg);
            }

            ghoulieDump.iterateTextures([](ghoulies::TextureData& data)
            {
                grabbed::output("%s\n", data.context.c_str());
                grabbed::output("%u x %u (%u frames) %s... ",
                    data.info.width, 
                    data.info.height, 
                    data.info.tileCount,
                    base::textureutils::toString(data.info.format));

                std::string filename;
                filename = data.context;
                if (data.name.size() > 0) {
                    filename += "_";
                    filename += data.name;
                }
                filename += ".png";

                size_t trueWidth = data.info.width;
                size_t trueHeight = data.info.height * data.info.tileCount;

                if (img::convertAndSavePng(data.rawData, filename, trueWidth, trueHeight, data.info.format)) {
                    grabbed::output("saved\n");
                }
                else {
                    grabbed::output("failed\n");
                }
            });

            return 0;
        }
    }
}
