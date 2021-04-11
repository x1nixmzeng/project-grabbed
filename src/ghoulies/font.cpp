#include "font.h"

#include "texture.h"

#include "base/assert.h"

#include <algorithm>
#include <limits>

namespace grabbed
{
    namespace ghoulies
    {
        bool Font::hasGlyph(u16 codename) const
        {
            auto result = std::find_if(m_glyphs.cbegin(), m_glyphs.cend(), [=](const GlyphData& glyph)
            {
                return (glyph.m_charCode == codename);
            });

            return (result != m_glyphs.cend());
        }
        
        FontReader::FontReader(std::shared_ptr<FontDb> database)
            : BaseFormat(ResourceType::eResFont)
            , m_fonts(database)
        { }

        bool FontReader::read(base::stream& stream, Context& context)
        {
            assert_true(context.resourceHeaders.size() == 1, "Expected one resource");

            // Setup the stream to point to the correct data
            stream.seek(context.getFileOffset());
            
            INFO_HEADER info;
            stream.read(info);

            Font fontDefinition;

            fontDefinition.m_name = context.name;
            fontDefinition.m_frames = info.glyph_frames;
            fontDefinition.m_height = info.glyph_height;
            fontDefinition.m_width = info.glyph_width;
            
            auto& fontGlyphs = fontDefinition.m_glyphs;
            fontGlyphs.reserve(255);
            
            u32 readCount = 0; // sum of read bytes
            u32 firstOffset = 0; // 

            u32 iterations = 0;
            while ((readCount == 0) || (readCount < firstOffset))
            {
                auto g = stream.read<GLYPH_INFO>();
                readCount += sizeof(GLYPH_INFO);

                // There are invalid GLYPH_INFO structures here
                // They will have an offset of 0 and/or info_size of 0xFF

                if (g.offset != 0) {
                    assert_true(g.info_size != 0xFF, "");

                    // this should happen once
                    if (firstOffset == 0) {
                        firstOffset = g.offset;
                    }

                    GlyphData new_glyph;

                    new_glyph.m_charCode = static_cast<u16>(info.first_character + iterations);

                    //assert_true(g.height < std::numeric_limits<decltype(new_glyph.boundHeight)>::max(), "Cannot pack boundHeight");
                    //assert_true(g.width < std::numeric_limits<decltype(new_glyph.boundWidth)>::max(), "Cannot pack boundWidth");

                    new_glyph.height = g.boundHeight;
                    new_glyph.width = g.boundWidth;

                    //assert_true(g.padXLeft < std::numeric_limits<decltype(new_glyph.xMin)>::max(), "Cannot pack xMin");
                    //assert_true(g.padXRight < std::numeric_limits<decltype(new_glyph.xMaxOffset)>::max(), "Cannot pack xMaxOffset");

                    new_glyph.padXLeft = g.xMin;
                    new_glyph.padXRight = g.xMaxOffset;

                    new_glyph.internalSize = g.info_size;
                    new_glyph.internalOffset = g.offset;

                    fontGlyphs.push_back(new_glyph);

                    if ((g.offset + g.info_size) == context.header.fileChunk.size) {
                        break;
                    }
                }

                ++iterations;
            }
            
            TEXTURE_HEADER texHeader;
            for (GlyphData& glyph : fontGlyphs)
            {
                // infoOffset will be correct on the first iteration
                auto infoOffset = context.getFileOffset() + glyph.internalOffset;
                stream.seek(infoOffset);
                
                // todo: merge with texture reading code
                stream.read(texHeader);

                assert_true_once(texHeader.width > 0);
                assert_true_once(texHeader.height > 0);
                assert_true_once(texHeader.tileCount > 0);

                auto finalWidth = glyph.padXLeft + glyph.width + glyph.padXRight;

                // Patch invalid glyph data (seen on 2 characters including the spacer ' ')
                if (finalWidth != texHeader.width) {
                    glyph.width = texHeader.width;
                    glyph.padXLeft = 0;
                    glyph.padXRight = 0;
                }

                // Cache off individual format
                glyph.m_format = texHeader.format;

                if (glyph.internalSize > sizeof(TEXTURE_HEADER)) {
                    size_t skipBytes{ glyph.internalSize - sizeof(TEXTURE_HEADER) };
                    stream.skip(skipBytes);
                }

                // A texture offset in the file header?
                stream.seek(context.getResourceOffset() + texHeader.texture_offset);

                // Glyph frame count is part of the font info
                size_t dataSize = info.glyph_frames * texHeader.dataSize;
                glyph.m_textureData.resize(dataSize);
                stream.readAll(glyph.m_textureData);
            }

            m_fonts->registerFont(fontDefinition);

            return true;
        }

        bool FontReader::canAdd(const string& name) const
        {
            const auto& data = m_fonts->editDb();

            auto result = std::find_if(data.begin(), data.end(), [&](const Font& font) {
                return font.m_name == name;
            });

            return result == data.end();
        }
    }
}
