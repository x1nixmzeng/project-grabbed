#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/texturetypes.h"

#include <map>
#include <memory>
#include <vector>

namespace grabbed
{
    namespace banjokazooie_nb
    {
        class BundleLocDB
        {
        public:
            std::map<std::string, std::wstring> data;
        };
        
        class BundleTextureDB
        {
        public:
            struct Texture
            {
                buffer data;

                size_t width;
                size_t height;
                X360TextureFormat format;
            };

            std::vector<Texture> data;
        };

        class BundleReader
        {
        public:
            BundleReader(std::shared_ptr<BundleLocDB> locData, std::shared_ptr<BundleTextureDB> textureData);

            bool read(base::stream& stream);

        protected:
            std::shared_ptr<BundleLocDB> m_locData;
            std::shared_ptr<BundleTextureDB> m_textureData;
        };
    }
}
