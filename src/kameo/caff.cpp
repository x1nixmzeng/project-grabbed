#include "caff.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace kameo
    {
        namespace
        {
            constexpr const char sc_magic[] = "CAFF";
            constexpr const char sc_version[] = "24.09.03.0026";

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
                u32 unknown_1; // zero
                u32 unknown_2; // zero
                u32 unknown_3; // zero
                u32 unknown_4; // zero
                u32 unknown_5; // dupe of size?
            };

            union Flags
            {
                struct
                {
                    u8 unknown_1;
                    u8 headers;
                    u8 unknown_3;
                };

                u32 raw;
            };
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

            auto fileCount = stream.read<u32>(); // 0x14
            auto unknown_1 = stream.read<u32>(); // 0x18
            auto unknown_2 = stream.read<u32>(); // 0x1c

            auto unknown_3 = stream.read<u32>(); // 0x20
            auto unknown_4 = stream.read<u32>(); // 0x24
            auto unknown_5 = stream.read<u32>(); // 0x28

            // relative (to dataOffset)
            auto dataSourceOffset = stream.read<u32>(); // 0x2c

            // absolute
            auto dataOffset = stream.read<u32>(); // 0x30

            Flags flags;
            // 3 bytes, padded to 4 (last is uninitialized; 0xCD)
            stream.read(flags);

            if (flags.headers < 2) {
                return false;
            }

            InfoHeader dataInfo;
            stream.read(dataInfo);
            if (strcmp(dataInfo.name, ".data") != 0) {
                return false;
            }

            InfoHeader gpuInfo;
            stream.read(gpuInfo);
            if (strcmp(gpuInfo.name, ".gpu") != 0) {
                return false;
            }
                
            stream.seek(dataOffset);
            auto hintName = stream.readString(16);

            CaffEntry texture;

            if (hintName.compare("texture") == 0) {
                // texture data headers

                stream.skip(4 * 4);
                auto texType = static_cast<XboxD3DFormat>(stream.read<u32>());
                stream.skip(4 * 2);

                texture.format = texType;
                
                auto texWidth = stream.read<u16>();
                auto texHeight = stream.read<u16>();
                
                texture.width = texWidth;
                texture.height = texHeight;
            }

            stream.seek(dataOffset + dataSourceOffset);

            auto sourceNameSize = stream.read<u32>();

            // skip name offset (we can read them)
            stream.skip(fileCount * 4);

            std::vector<string> sourceNames(fileCount);
                
            for (size_t i = 0; i < fileCount; ++i) {
                sourceNames[i] = stream.readCString();
            }

            if (texture.width != 0 && fileCount == 1) {
                stream.seek(dataOffset + dataInfo.size);

                texture.data.resize(gpuInfo.size);
                stream.readAll(texture.data);

                m_data->data[sourceNames[0]] = std::move(texture);

                return true;
            }

            return false;
        }

        bool CaffReader::checkType(base::stream& stream, const string& desiredType)
        {
            FileHeader fileHead;
            stream.read(fileHead);

            if (strncmp(fileHead.magic, sc_magic, sizeof(sc_magic) - 1) != 0) {
                return false;
            }

            if (strncmp(fileHead.version, sc_version, sizeof(sc_version) - 1) != 0) {
                return false;
            }

            auto fileCount = stream.read<u32>(); // 0x14
            auto unknown_1 = stream.read<u32>(); // 0x18
            auto unknown_2 = stream.read<u32>(); // 0x1c

            auto unknown_3 = stream.read<u32>(); // 0x20
            auto unknown_4 = stream.read<u32>(); // 0x24
            auto unknown_5 = stream.read<u32>(); // 0x28

                                                // relative (to dataOffset)
            auto dataSourceOffset = stream.read<u32>(); // 0x2c

                                                       // absolute
            auto dataOffset = stream.read<u32>(); // 0x30

            Flags flags;
            // 3 bytes, padded to 4 (last is uninitialized; 0xCD)
            stream.read(flags);

            if (flags.headers < 2) {
                return false;
            }

            InfoHeader dataInfo;
            stream.read(dataInfo);
            if (strcmp(dataInfo.name, ".data") != 0) {
                return false;
            }

            InfoHeader gpuInfo;
            stream.read(gpuInfo);
            if (strcmp(gpuInfo.name, ".gpu") != 0) {
                return false;
            }

            stream.seek(dataOffset);
            auto hintName = stream.readString(16);

            return (hintName == desiredType);
        }
    }
}
