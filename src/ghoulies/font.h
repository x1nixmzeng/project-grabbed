#pragma once

#include "ghoulies/formats.h"
#include "ghoulies/texture.h"

#include "base/filestream.h"

#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        struct GlyphData
        {
            buffer m_textureData;
            XboxD3DFormat m_format;
            u16 m_charCode;
            u16 width;
            u16 height;

            u8 padXLeft;
            u8 padXRight;

            u32 internalSize;
            u32 internalOffset;
        };

        class Font
        {
        public:
            std::vector<GlyphData> m_glyphs;
            
            u32 m_frames;
            u32 m_width;
            u32 m_height;
            string m_name;
            
            bool hasGlyph(u16 codename) const;
        };

        class FontDb
        {
        public:
            void registerFont(Font& font)
            {
                m_fonts.push_back(font);
            }

            const std::vector<Font>& editDb() const {
                return m_fonts;
            }

            std::vector<Font>& editDb() {
                return m_fonts;
            }

        private:
            std::vector<Font> m_fonts;
        };

        class FontReader : public BaseFormat
        {
            struct INFO_HEADER
            {
                u32 first_character;
                u32 unknown_1;
                u32 glyph_frames;
                u32 glyph_width;
                u32 glyph_height;
                u32 unknown_2a : 4;
                u32 unknown_2b : 4;
                u32 unknown_2x : 16;
            };
            static constexpr auto INFO_HEADER_SIZE = sizeof(INFO_HEADER);
            static_assert(INFO_HEADER_SIZE == 24, "INFO_HEADER must be 24-bytes");

            struct GLYPH_INFO
            {
                u32 offset;
                union
                {
                    i32 info;

                    struct
                    {
                        u32 info_size : 8;
                        u32 unknown_2 : 24;
                    };
                };
                u32 xMin;
                u32 xMaxOffset;
                u32 boundWidth;
                u32 boundHeight;
            };
            static constexpr auto GLYPH_INFO_SIZE = sizeof(GLYPH_INFO);
            static_assert(GLYPH_INFO_SIZE == 24, "GLYPH_INFO must be 24-bytes");

        public:
            FontReader(std::shared_ptr<FontDb> database);

            virtual bool read(base::stream& stream, ghoulies::Context& context) override;
            virtual bool canAdd(string& name) const override;

        private:
            std::shared_ptr<FontDb> m_fonts;
        };
    }
}
