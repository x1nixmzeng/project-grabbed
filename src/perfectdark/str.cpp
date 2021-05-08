#include "str.h"

#include "base/assert.h"
#include "base/memorystream.h"
#include "base/byte_order.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace perfectdark
    {
        namespace
        {
            enum class HeaderByteOrder : u8
            {
                little = 0,
                big = 1,
            };

            using u32b = base::endianBig<u32>;
            using u16b = base::endianBig<u16>;

            struct Header
            {
                u32b unknown_1; // hash?
                u32b unknown_2; // 1
                u32b unknown_3; // 1
                u32b unknown_4; // 0
                u32b unknown_5; // 0
                u32b unknown_6; // 1
                u32b unknown_7; // 1
                u32b unknown_8; // 0
                u32b textChunkSize;
                u32b textChunkOffset;
                HeaderByteOrder byteOrder;
                u8 flag_2;
                u8 flag_3;
                u8 flag_4;
            };

            constexpr static size_t HeaderSize = sizeof(Header);

            struct Chunk1
            {
                u32b unknown_1;
                u32b unknown_2;
                u32b unknown_3;
            };

#pragma pack(push,1)
            struct LSBLInfo
            {
                u32b index;
                u32b offset;
                u32b length;
                u8 flag_1; // 1
                u8 flag_2; // 2
            };
#pragma pack(pop)
            constexpr size_t LSBLInfoSize{ sizeof(LSBLInfo) };
            static_assert(LSBLInfoSize == 14U, "LSBLInfo must be packed");


#pragma pack(push,1)
            struct LSBLHeader
            {
                u32 magic;
                u32 unknown_2; // 28
                u32 unknown_3; // 4
                u32 unknown_4; // 28
                u32 textOffset;
                u32 commentsOffset; // optional
                u32 tailOffset; // optional
            };
#pragma pack(pop)

            constexpr size_t LSBLHeaderSize = sizeof(LSBLHeader);
            static_assert(LSBLHeaderSize == 28, "LSBLHeader size mismatch");

            constexpr const u32 c_magic = 'LBSL';

#pragma pack(push,1)
            struct Chunk2
            {
                i16 index;
                u32 offset;
            };
#pragma pack(pop)

            bool readText(base::stream& stream, Header& hdr, std::shared_ptr<StrDB> strDb)
            {
                stream.seek(hdr.textChunkOffset);
                
                auto textMagic{ stream.readCString() };
                assert_true(textMagic == "text");
                // skip this info
                stream.skip(32 - 5);

                auto localHeader{ stream.read<LSBLHeader>() };

                assert_true(localHeader.magic == c_magic, "Unexpected magic valid");
                assert_true(localHeader.unknown_2 == 28, "Expected 28");
                assert_true(localHeader.unknown_3 == 4, "Expected 4");
                assert_true(localHeader.unknown_4 == 28, "Expected 28");
                
                auto currentPos = stream.getPosition();

                auto offset = stream.read<u32>();
                auto count = stream.read<u32>();

                // why 2 extra?
                auto actualCount = count +2;
                auto headers{ stream.readArray<Chunk2>(actualCount) };
                        
                for (size_t i = 0; i < count; ++i)
                {
                    std::string keyName;
                    keyName.append("key_");
                    keyName.append(std::to_string(i));

                    auto value { stream.readWCString() };
                    strDb->data[keyName] = value;
                }

                //auto totalSize{ stream.getPosition() - currentPos };
                //assert_true(totalSize == offset);

                //auto debugNameLen{ stream.read<u32b>() };
                //auto unknown{ stream.read<u32b>() };
                //assert_true(unknown == 0);

                //auto debugName{ stream.readString(debugNameLen) };
                //strDb->debugName = debugName;

                return true;
            }
        }

        StrReader::StrReader(std::shared_ptr<StrDB> data)
            : m_data(data)
        {
        }

        bool StrReader::read(base::stream& stream)
        {
            auto caffHeader{ stream.readString(4) };
            if (caffHeader.compare("CAFF") != 0) {
                return false;
            }

            auto caffVersion{ stream.readString(16) };
            if (caffVersion.compare("28.01.05.0031") != 0) {
                return false;
            }

            auto header{ stream.read<Header>() };
            assert_true(header.byteOrder == HeaderByteOrder::big, "little endian not supported");

            return readText(stream, header, m_data);
        }
    }
}
