#include "caff.h"

#include "base/output.h"
#include "base/byte_order.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace perfectdark
    {
        using u32b = base::endianBig<u32>;
        using u16b = base::endianBig<u16>;

        namespace
        {
            constexpr const char sc_magic[] = "CAFF";
            constexpr const char sc_version[] = "28.01.05.0031";

            struct FileHeader
            {
                char magic[4];
                char version[16];
            };

            struct InfoHeader
            {
                char name[8];
                u32 size;
                u32 offset;
                u32 unknown_1; // 0
                u32 unknown_2; // 0
                u32 unknown_3; // 0
                u32 unknown_4; // 0
                u32 unknown_5; // dupe of size?
            };

            constexpr size_t InfoHeaderSize = sizeof(InfoHeader);

            struct EntryHeader
            {
                u32b offset;
                u32b unknown_2; // 0
                u32b unknown_3; // 0
                u32b unknown_4; // 0
                u32b unknown_5; // 0
                u32b unknown_6; // 0
                u32b unknown_7; // 0
                u32b unknown_8; // 0
            };

            constexpr size_t EntryHeaderSize = sizeof(EntryHeader);

            enum class HeaderByteOrder : u8
            {
                little = 0,
                big = 1,
            };

            union Flags
            {
                struct
                {
                    HeaderByteOrder byteOrder;
                    u8 headerCount;
                    u8 unknown_3;
                    u8 unknown_4;
                };

                u32 raw;
            };

            string readAlignedCString(base::stream& stream)
            {
                auto result{ stream.readCString() };

                size_t aligned{ (result.size() + 1) % 4 };
                if (aligned != 0 ) {
                    stream.skip(4 - aligned);
                }

                return result;
            }
        }

        CaffReader::CaffReader(std::shared_ptr<CaffDB> data)
            : m_data(data)
        { }

        bool CaffReader::read(base::stream& stream)
        {
            FileHeader fileHead;
            stream.read(fileHead);

            if (strncmp(fileHead.magic, sc_magic, sizeof(sc_magic) - 1) != 0) {
                return false;
            }

            if (strncmp(fileHead.version, sc_version, sizeof(sc_version) - 1) != 0) {
                return false;
            }

            auto version = stream.read<u32b>(); // 0x14

            auto fileCount = stream.read<u32b>(); // 0x18
            auto unknown_1 = stream.read<u32b>(); // 0x1c
            auto unknown_2 = stream.read<u32b>(); // 0x20
            auto unknown_3 = stream.read<u32b>(); // 0x24
            auto unknown_4 = stream.read<u32b>(); // 0x28
            auto unknown_5 = stream.read<u32b>(); // 0x2c
            auto unknown_6 = stream.read<u32b>(); // 0x30
            auto dataSourceOffset = stream.read<u32b>(); // 0x34
            auto dataOffset = stream.read<u32b>(); // 0x38

            Flags flags;
            stream.read(flags); // 0x3c

            // Would this get as far without be?
            if (flags.byteOrder != HeaderByteOrder::big) {
                return false;
            }

            InfoHeader dataInfo;
            stream.read(dataInfo);
            if (strcmp(dataInfo.name, ".data") != 0) {
                return false;
            }

            stream.seek(dataOffset);

            auto fileA{ stream.readString(16) };
            if (fileA != "darkpackage") {
                return false;
            }

            stream.skip(16);

            {
                auto actualCount = stream.read<u32b>();
                
                auto items{ stream.readArray<EntryHeader>(actualCount) };

                std::vector<string> stringList(actualCount);
                
                for (size_t i = 0; i < actualCount; ++i) {
                    stringList[i] = readAlignedCString(stream);
                    output("%s\n", stringList[i].c_str());
                }
            }

            return false;
        }
    }
}
