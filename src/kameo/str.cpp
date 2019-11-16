#include "str.h"

#include "base/assert.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace kameo
    {
        namespace
        {
            constexpr const char sc_magic[] = "LSBL";

            struct FileHeader
            {
                char magic[4];
                u32 size_1;     // 28
                u32 unknown_1;  // 4
                u32 size_2;     // 28
                u32 unknown_2;  // 0
                u32 unknown_3;  // 0
                u32 unknown_4;  // 0
            };

#pragma pack(push,1)
            struct EntryInfo
            {
                i16 index;
                u32 offset; // In characters, not bytes
            };
#pragma pack(pop)
        }

        StrReader::StrReader(std::shared_ptr<StrDB> data)
            : m_data(data)
        { }

        bool StrReader::read(base::stream& stream)
        {
            FileHeader fileHead;
            stream.read(fileHead);

            if (strncmp(fileHead.magic, sc_magic, sizeof(sc_magic) - 1) != 0) {
                return false;
            }
            
            auto streamSize = stream.getSize();
            auto dataSize = stream.read<u32>();
            assert_true((sizeof(FileHeader) + dataSize) == streamSize);

            auto count = stream.read<u32>();

            auto info{ stream.readArray<EntryInfo>(count + 1) };
            
            auto dataOffset = stream.getPosition();
            for (const auto& entry : info)
            {
                auto actualOffset = dataOffset + (2 * entry.offset);
                stream.seek(actualOffset);
                auto wstr = stream.readWCString();
                m_data->data[entry.index] = wstr;
            }

            stream.ensureFullyRead();
            return true;
        }
    }
}
