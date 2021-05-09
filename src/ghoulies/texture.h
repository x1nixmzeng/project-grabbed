#pragma once

#include "base/types.h"
#include "base/texturetypes.h"
#include "base/stream.h"

#include "ghoulies/formats.h"

#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        // 28
        struct TEXTURE_HEADER
        {
            XboxD3DFormat format;
            u32 headerSize; // 28
            u16 width;
            u16 height;
            u32 flags; // 0x00000001
            u32 unknown_3a : 8;
            u32 tileCount : 8;
            u32 unknown_3c : 16;
            u32 texture_offset;
            u32 dataSize;

            // This structure is sometimes padded
        };
         
        struct TextureData
        {
            std::string context; // used for duplicate names across models
            std::string name;
            TEXTURE_HEADER info;
            buffer rawData;
        };

        struct TextureDB
        {
            std::vector<TextureData> m_storage;
        };

        class TextureReader : public BaseFormat
        {
        public:
            TextureReader(std::shared_ptr<ghoulies::TextureDB> database);

            virtual bool read(base::stream& stream, Context& context) override;

            virtual bool canAdd(const std::string& name) const override;

        private:
            std::shared_ptr<ghoulies::TextureDB> m_textures;
        };
    }
}
