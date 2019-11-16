#include "soundbank.h"

#include "base/types.h"
#include "base/assert.h"

#include "base/fileutils.h"

#include <iterator>

namespace grabbed
{
    namespace ghoulies
    {
        namespace
        {
            constexpr u32 c_magic = 'KBDS';
            constexpr u16 c_version = 11;
            constexpr size_t c_nameSize = 16;

            // 56-bytes (40 + bankName)
            struct Header
            {
                u32 magic;
                u16 version; // Only 11 is supported
                u16 checksum;
                u32 filenamesOffset;
                u32 offset_2;
                u32 offset_3;
                u32 fileSize;
                u16 value_1;
                u16 value_2; // variation of cueCount?!
                u16 cueCountHeaders;
                u16 cueCountData;
                u16 value_5;
                u16 resourceCount;
                u32 unknown_0;
                char bankName[c_nameSize];
            };

            // 20-bytes
            struct DataHeader
            {
                u16 unknown_0; // 0
                i16 index;
                u32 filenameOffset;
                i32 unknown_1; // -1
                u32 unknown_2; // 0
                u32 unknown_3;
            };
            static_assert(sizeof(DataHeader) == 20, "Unexpected size of DataHeader");

            // 20-bytes
            struct ItemHeader
            {
                u32 offset;
                u16 unknown_1;
                u16 unknown_2;
                u16 unknown_3;
                u8 flags_1; // 0x2?
                u8 flags_2; // 0x1 is valid?!
                u16 unknown_5;
                u16 unknown_6;
                u16 unknown_7;
                u16 unknown_8;
            };
            static_assert(sizeof(ItemHeader) == 20, "Unexpected size of ItemHeader");
        }

        SoundbankReader::SoundbankReader(std::shared_ptr<SoundbankDb> data)
            : BaseFormat(ResourceType::eResXSoundbank)
            , m_data(data)
        {
            assert_true(m_data != nullptr);
        }

        bool SoundbankReader::read(base::stream& stream, Context& context)
        {
            const auto initialOffset = context.getFileOffset();

            stream.seek(initialOffset);

            Header header;
            stream.read(header);

            assert_true(header.magic == c_magic);
            assert_true(context.header.fileChunk.size == header.fileSize);
            
            // Soundbank name
            auto& newItem = m_data->items[context.name];

            newItem.name = header.bankName;
            newItem.count = header.cueCountData;
            
            std::vector<DataHeader> dataHeaders(header.cueCountData);
            stream.readAll(dataHeaders);

            std::vector<ItemHeader> itemHeaders(header.cueCountHeaders);
            stream.readAll(itemHeaders);

            size_t expected = initialOffset + header.filenamesOffset;
            size_t current = stream.getPosition();

            assert_true(expected == current, "mismatched pointers");

            // fallback on this
            stream.seek(expected);
            
            std::map<size_t, string> mappedNames;

            for (size_t i = 0; i < header.resourceCount; ++i) {
                auto offset = stream.getPosition() - initialOffset;
                auto owningBank = stream.readString(c_nameSize);
                mappedNames[offset] = owningBank;
            }

            // Store all defined names (excluding default one, read next)
            for (const auto& definedName : mappedNames) {
                newItem.owningBanks.push_back(definedName.second);
            }
            
            // Filenames offset always has a default name of "Null"
            {
                auto offset = stream.getPosition() - initialOffset;
                auto nullName = stream.readString(4);
                mappedNames[offset] = nullName;
            }
            
            // Resolve filenames (these always refer to the default)
            for (const auto& data : dataHeaders) {
                auto it = mappedNames.find(data.filenameOffset);
                assert_true(it != mappedNames.end());
            }
            
            if (0)
            {
                stream.seek(context.getFileOffset());
                base::fileutils::saveToDisk(stream, context.header.fileChunk.size, context.name);
            }
            
            return true;
        }

        bool SoundbankReader::canAdd(string& name) const
        {
            assert_true(m_data != nullptr);
            return (m_data->items.find(name) == m_data->items.end());
        }
    }
}
