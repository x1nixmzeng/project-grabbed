#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/texturetypes.h"

#include <memory>
#include <vector>

namespace grabbed
{
    namespace onevs
    {
        class MetadataDB
        {
        public:

            struct TextureData
            {
                buffer data;
                
                size_t width;
                size_t height;
                GenericTextureType format;
            };

            struct Metadata
            {
                //string fileName;
                std::string originalName;

                std::vector<std::string> subfiles;
                std::vector<std::string> strings;

                std::vector<TextureData> textures;
            };

            std::vector<Metadata> data;
        };

        class BundleReader
        {
        public:
            BundleReader(std::shared_ptr<MetadataDB> metadata);

            bool read(base::stream& stream);

        protected:
            std::shared_ptr<MetadataDB> m_metadataData;
        };
    }
}
