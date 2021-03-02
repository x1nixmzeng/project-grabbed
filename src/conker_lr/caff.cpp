#include "caff.h"

#include "base/assert.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace conker_lr
    {
        namespace
        {
            constexpr const char sc_magic[] = "CAFF";
            constexpr const char sc_version[] = "28.01.05.0031";

            // Demo version found in Jade Empire
            // Seems to be the exact same CAFF version as Kameo (Xbox alpha)
            constexpr const char sc_demoVersion[] = "24.09.03.0026";

            struct FileHeader
            {
                char magic[4]; 
                char version[16];
            };

            struct InfoHeader
            {
                char name[8];
                u32 unknown_1; // big endian value?
                u32 offset;
                u32 size;
                u32 unknown_2;
                u32 unknown_3;
                u32 unknown_4;
                u32 unknown_5;
                u32 unknown_6;
            };

            // Exact same as kameo caff
            struct InfoHeader_demo
            {
                char name[8];
                u32 size;
                u32 offset;
                u32 unknown_2;
                u32 unknown_3;
                u32 unknown_4;
                u32 unknown_5;
                u32 unknown_6; // dupe of size, possible total size
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

            // Posted this back in 2015 https://gist.github.com/x1nixmzeng/c55c248c384bf1c9df2a
            void checksumByte(u32& checksum, const u8 currentByte)
            {
                // Signed; values >= 0x80 will be subtracted from the current checksum
                u32 val = (checksum << 4) + static_cast<char>(currentByte);

                u32 val_mask = val & 0xF0000000;
                if (val_mask) {
                    /* copy the mask over 0xF00000F0 */
                    val_mask |= ((u32)val_mask >> 24);
                    val ^= val_mask;
                }

                checksum = val;
            }

            u32 calculateChecksum(base::stream& stream)
            {
                constexpr size_t sc_headerSize = 64 * 6;

                buffer data(sc_headerSize);

                stream.seek(0);
                stream.readAll(data);

                u32 checksum = 0;

                size_t index = 0;

                // Checksum the first 20 bytes (magic, version string)
                while (index < 20) {
                    checksumByte(checksum, data[index]);
                    ++index;
                }

                // Skip the stored checksum value
                while (index < 24) {
                    checksumByte(checksum, 0);
                    ++index;
                }

                // Checksum the remaining header data
                while (index < sc_headerSize) {
                    checksumByte(checksum, data[index]);
                    ++index;
                }

                return checksum;
            }
        }

        CaffReader::CaffReader(std::shared_ptr<CaffDB> data)
            : m_data(data)
        { }

        bool CaffReader::readRetail(base::stream& stream)
        {
            auto checksum = stream.read<u32>(); // 0x14

            auto offset = stream.getPosition();
            auto actualChecksum = calculateChecksum(stream);
            stream.seek(offset);

            assert_true(checksum == actualChecksum, "Checksum mismatch");
            if (checksum != actualChecksum) {
                return false;
            }

            auto fileCount = stream.read<u32>(); // 0x18

            auto unknown_1 = stream.read<u32>(); // 0x1c
            auto unknown_2 = stream.read<u32>(); // 0x20
            auto unknown_3 = stream.read<u32>(); // 0x24
            auto unknown_4 = stream.read<u32>(); // 0x28
            auto unknown_5 = stream.read<u32>(); // 0x2c
            auto unknown_6 = stream.read<u32>(); // 0x30

            // relative (to dataOffset)
            auto dataSourceOffset = stream.read<u32>(); // 0x34

            // absolute
            auto dataOffset = stream.read<u32>(); // 0x38

            Flags flags;
            stream.read(flags); // 0x3c

            if (flags.headers < 2) {
                return false;
            }

            InfoHeader dataInfo;
            stream.read(dataInfo); // 0x40
            if (strcmp(dataInfo.name, ".data") != 0) {
                return false;
            }

            InfoHeader gpuInfo;
            stream.read(gpuInfo); // 0x68
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

        bool CaffReader::readDemo(base::stream& stream)
        {
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

            InfoHeader_demo dataInfo;
            stream.read(dataInfo);
            if (strcmp(dataInfo.name, ".data") != 0) {
                return false;
            }

            InfoHeader_demo gpuInfo;
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

        bool CaffReader::read(base::stream& stream)
        {
            FileHeader fileHead;
            stream.read(fileHead);

            if (strncmp(fileHead.magic, sc_magic, sizeof(sc_magic) - 1) != 0) {
                return false;
            }

            auto retailVersion = strncmp(fileHead.version, sc_version, sizeof(sc_version) - 1);
            if (retailVersion == 0) {
                return readRetail(stream);
            }

            auto demoVersion = strncmp(fileHead.version, sc_demoVersion, sizeof(sc_demoVersion) - 1);
            if (demoVersion == 0) {
                return readDemo(stream);
            }

            return false;
        }
    }
}
