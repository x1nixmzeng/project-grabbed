#include "caff.h"

#include "base/memorystream.h"

#include "base/assert.h"
#include "base/zlibutils.h"
#include "base/byte_order.h"

#include "base/fileutils.h"
#include "base/output.h" // debug

#include "img/img.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace banjokazooie_nb
    {
        namespace
        {
            constexpr const char sc_magic[] = "CAFF";
            constexpr const char sc_version[] = "07.08.06.0036";

            constexpr const char sc_textureMagic[] = "texture";
            constexpr const char sc_textureVersion[] = "04.05.05.0032";

            constexpr const char sc_dataChunk[] = ".data";
            constexpr const char sc_textureChunk[] = ".texturegpu";
            constexpr const char sc_gpuChunk[] = ".gpu";
            constexpr const char sc_gpuCachedChunk[] = ".gpucached";
            constexpr const char sc_streamChunk[] = ".stream";

            struct CaffFileHeader
            {
                char magic[4];
                char version[16];
            };

            enum class HeaderByteOrder : u8
            {
                little = 0,
                big = 1,
            };

            base::memorystream readChunk(base::stream& stream, size_t zsize, size_t size)
            {
                base::memorystream chunk;

                // Read compressed size
                buffer buffer(zsize);
                stream.readAll(buffer);

                // Allocate for uncompressed size
                chunk.resize(size);

                // Some games are not further compressed (X360)
                if (zsize == size) {
                    chunk.write(buffer.size(), buffer.data());
                    chunk.seek(0);
                }
                else {
                    assert_true(base::zlibutils::decompress(buffer, chunk));
                }

                return std::move(chunk);
            }

            string readAlignedCString(base::stream& stream)
            {
                auto result{ stream.readCString() };

                size_t aligned{ (result.size() + 1) % 4 };
                if (aligned != 0) {
                    stream.skip(4 - aligned);
                }

                return result;
            }

            template <template<typename T> class var = base::endianLittle>
            struct Header
            {
                var<u32> chunk1_offset;
                var<u32> checksum;
                var<u32> count_1;
                var<u32> count_2;
                u32 unknown_2; // 0
                u32 unknown_3; // 0
                var<u32> count_3;
                var<u32> count_ex;
                u32 unknown_4; // 0
                u32 unknown_5; // 0
                u32 count_4; // 0
                u32 count_5; // 0
                u32 unknown_8; // 0

                HeaderByteOrder byteOrder;
                u8 sectionCount;
                u8 flag_3;
                u8 flag_4;

                var<u32> unknown_9; // 6 - NOT THE RESOURCE TYPE
                var<u32> chunk1_size;
                u32 unknown_12;
                u32 unknown_13; // 0
                u32 unknown_14; // 0
                var<u32> chunk1_zsize;
                var<u32> chunk2_size;
                u32 unknown_17;
                u32 unknown_18; // 0
                u32 unknown_19; // 0
                var<u32> chunk2_zsize;
            };

            constexpr static size_t HeaderSize = sizeof(Header<>);

            template <template<typename T> class var = base::endianLittle>
            struct Chunk1
            {
                var<u32> unknown_1;
                var<u32> unknown_2;
                var<u32> unknown_3;
            };

#pragma pack(push,1)
            template <template<typename T> class var = base::endianLittle>
            struct LSBLInfo
            {
                var<u32> index;
                var<u32> offset;
                var<u32> length;
                u8 flag_1; // 1
                u8 flag_2; // 2
            };
#pragma pack(pop)
            constexpr size_t LSBLInfoSize{ sizeof(LSBLInfo<>) };
            static_assert(LSBLInfoSize == 14U, "LSBLInfo must be packed");


#pragma pack(push,1)
            template <template<typename T> class var = base::endianLittle>
            struct LSBLHeader
            {
                var<u32> magic;
                var<u32> unknown_2; // 28
                var<u32> unknown_3; // 4
                var<u32> unknown_4; // 28
                var<u32> textOffset;
                var<u32> commentsOffset; // optional
                var<u32> tailOffset; // optional
                var<u32> offset;
                var<u32> count;
            };
#pragma pack(pop)

            constexpr size_t LSBLHeaderSize = sizeof(LSBLHeader<>);
            static_assert(LSBLHeaderSize == 36, "LSBLHeader size mismatch");

            constexpr const u32 c_magic = 'LBSL';

#pragma pack(push,1)
            template <template<typename T> class var = endianLittle>
            struct Chunk2
            {
                var<u32> index;
                var<u16> offset;
            };
#pragma pack(pop)

#pragma pack(push,1)
            struct Chunk3
            {
                u16 id;
                u16 index;
                u32 offset;
            };
#pragma pack(pop)

            template <template<typename T> class var>
            wstring readWCString(base::stream& stream)
            {
                wstring result;
                result.reserve(16);

                while (stream.canRead(sizeof(u16)))
                {
                    auto chr{ stream.read<var<u16>>() };
                    if (chr == 0) {
                        break;
                    }

                    result.push_back(static_cast<wchar_t>(chr));
                }

                return result;
            }

            template <template<typename T> class var>
            bool readLocBundle(base::stream& stream, Header<var>& hdr, std::shared_ptr<BundleLocDB> bundleDb)
            {
                stream.seek(hdr.chunk1_offset);

                auto chunk1{ readChunk(stream, hdr.chunk1_zsize, hdr.chunk1_size) };

                var<u32> chunk3_size = 0;
                var<u32> chunk3_zsize = 0;

                std::vector<var<u32>> stringNameOffsets;
                buffer stringNames;
                std::vector<LSBLInfo<var>> offsets;

                {
                    {
                        auto zero{ chunk1.read<u32>() };
                        assert_true(zero == 0);
                    }

                    {
                        auto two{ chunk1.read<u32>() };
                        //assert_true(two == 4); // not endian specific
                    }

                    chunk1.skip(1);
                    chunk3_size = chunk1.read<var<u32>>();
                    chunk1.skip(16); // zeros
                    chunk3_zsize = chunk1.read<var<u32>>();

                    auto data{ chunk1.readString(6) };
                    assert_true(data == ".data");

                    auto stringNameSize{ chunk1.read<var<u32>>() };
                    stringNameOffsets = chunk1.readArray<var<u32>>(hdr.count_1);

                    stringNames.resize(stringNameSize);
                    chunk1.readAll(stringNames);

                    {
                        auto zero{ chunk1.read<u32>() };
                        assert_true(zero == 0);
                    }

                    offsets = chunk1.readArray<LSBLInfo<var>>(hdr.count_1);

                    //chunk1.ensureFullyRead();
                }

                // ignored: just patterns of 12 bytes (u32, u32, u32(1)) - also padded
                //base::memorystream chunk2;
                //{
                //}

                stream.skip(hdr.chunk2_zsize);

                assert_true(chunk3_size > 0);
                assert_true(chunk3_zsize > 0);

                // LSBL

                auto chunk3{ readChunk(stream, hdr.chunk3_zsize, chunk3_size) };

                //stream.ensureFullyRead();


                // process chunk3
                {
                    for (const LSBLInfo<var>& item : offsets)
                    {
                        std::vector<string> names;
                        std::vector<wstring> values;

                        assert_true(item.flag_1 == 1);
                        //assert_true(item.flag_2 == 4); // was 2

                        chunk3.seek(item.offset);

                        Chunk1<var> c1{ chunk3.read<Chunk1<var>>() };
                        //assert_true(c1.unknown_1 == 12, "unknown extra data");

                        // Usually 12 bytes
                        chunk3.skip(c1.unknown_1 - sizeof(c1));

                        // after LSBL header
                        size_t startPos{ chunk3.getPosition() };

                        // no endian swap? eeh
                        auto localHeader{ chunk3.read<LSBLHeader<base::endianLittle>>() };

                        assert_true(localHeader.magic == c_magic, "Unexpected magic valid");
                        assert_true(localHeader.unknown_2 == 28, "Expected 28");
                        assert_true(localHeader.unknown_3 == 4, "Expected 4");
                        assert_true(localHeader.unknown_4 == 28, "Expected 28");

                        auto actualCount = localHeader.count + 1;
                        auto headers{ chunk3.readArray<Chunk2<base::endianLittle>>(actualCount) };

                        values.resize(actualCount);
                        for (size_t i = 0; i < actualCount; ++i)
                        {
                            auto wstr{ readWCString<base::endianLittle>(chunk3) };
                            values[i] = wstr;
                        }

                        auto pos1{ (chunk3.getPosition() - startPos) };

                        // difference here: instead of reading an 'extra' thing here, we ignore it
                        chunk3.skip(4 * 2);

                        std::vector<Chunk3> lookupInfo{ chunk3.readArray<Chunk3>(localHeader.count) };

                        names.resize(localHeader.count);
                        for (size_t i = 0; i < localHeader.count; ++i)
                        {
                            auto str{ chunk3.readCString() };
                            names[i] = str;
                        }

                        // fixup:
                        std::vector<size_t> lookup;
                        lookup.resize(values.size());

                        // eh?
                        for (size_t i = 0; i < localHeader.count; ++i)
                        {
                            lookup[lookupInfo[i].index - 1] = i;
                        }

                        for (size_t i = 0; i < localHeader.count; ++i)
                        {
                            auto& val = values[lookup[i]];
                            auto& key = names[i];

                            bundleDb->data[key] = val;
                        }

                        if (localHeader.tailOffset != 0)
                        {
                            // not endian specific?!
                            auto tail{ chunk3.read<u32>() };
                            // skip tail?
                            chunk3.skip(tail);
                        }
                    }
                }

                return true;
            }

            struct CaffType
            {
                char magic[8];
                char version[16];
            };

            // Ported from RareView

#pragma pack(push,1)
            template <template<typename T> class var = base::endianLittle>
            struct TextureInfo
            {
                var<u32> textureFormat; // packed 
                var<u32> textureType; // unused?
                var<u32> unknown_1;
                var<u16> width;
                var<u16> height;
                var<u32> offset;
                var<i32> mipMapLevels; // -1 ?
                var<u32> unknown_4; // 1
                var<u32> unknown_5; // sd
                u8 padding[56];
            };
#pragma pack(pop)

            constexpr size_t infoSize = sizeof(TextureInfo<>);

            size_t GetTextureDataSize(int _width, int _height, X360TextureFormat _textureFormat)
            {
                int textureDataSize = 0;
                switch (_textureFormat)
                {
                case X360TextureFormat::A8L8:
                    textureDataSize = (_width * _height) * 2;
                    break;
                case X360TextureFormat::L8:
                    textureDataSize = (_width * _height);
                    break;
                case X360TextureFormat::DXT1:
                    textureDataSize = (_width * _height / 2);
                    break;
                case X360TextureFormat::DXT3:
                case X360TextureFormat::DXT5:
                case X360TextureFormat::DXN:
                    textureDataSize = (_width * _height);
                    break;
                case X360TextureFormat::A8R8G8B8:
                    textureDataSize = (_width * _height) * 4;
                    break;
                case X360TextureFormat::CTX1:
                    textureDataSize = (_width * _height / 2);
                    break;
                }

                return textureDataSize;
            }

#pragma pack(push,1)
            template <template<typename T> class var = base::endianLittle>
            struct sectionInfoHeader
            {
                u8 unknown_1;
                var<u32> offset;
                var<u32> unknown_3;
                var<u32> size1; // 1
                var<u32> unknown_4;
                var<u32> unknown_5;
                var<u32> unknown_6;
                var<u32> unknown_7;
                var<u32> size2; // 2
            };
#pragma pack(pop)

            constexpr size_t sectionInfoHeaderSize{ sizeof(sectionInfoHeader<>) };
            static_assert(sectionInfoHeaderSize == 33U, "sectionInfoHeader must be packed");

            template <template<typename T> class var>
            bool testReadData(base::stream& stream, Header<var>& hdr)
            {
                // Read chunk 1

                stream.seek(hdr.chunk1_offset);
                auto chunk1{ readChunk(stream, hdr.chunk1_zsize, hdr.chunk1_size) };
                
                auto sectionInfo{ chunk1.readArray<sectionInfoHeader<var>>(hdr.sectionCount) };
                auto sectionNames{ chunk1.readArray<string>(hdr.sectionCount) };


                for (size_t i = 0; i < hdr.sectionCount; ++i) {
                    // store this away for the template to deduce correctly
                    //size_t zsize = sectionInfo[i].size1;
                    //output("%s (%u bytes)\n", sectionNames[i].c_str(), zsize);
                }

                auto poolSize{ chunk1.read<var<u32>>() }; // total size
                auto nameOffsets{ chunk1.readArray<var<u32>>(hdr.count_1) };

                // Read all filenames to string pool
                base::memorystream stringPool(poolSize);
                chunk1.read(poolSize, stringPool);

                std::vector<string> names;

                // todo: process string pool names (c-strings)
                for (size_t i = 0; i < hdr.count_1; ++i) {
                    stringPool.seek(nameOffsets[i]);

                    auto name{ stringPool.readCString() };
                    if (name.length() == 0) {
                        return false;
                    }
                    names.emplace_back(name);
                    output("Got file %s\n", name.c_str());
                }
                stringPool.seek(0);

                // what follows is strange padding - aligned to position of file rather than pool size?

                //// Not aligned to the pool, but the file
                //if ((chunk1.getPosition() & 3) != 0) {
                //    chunk1.skip(4 - (chunk1.getPosition() & 3));
                //}
                
                // maybe this isn't for alignment
                auto debugNames{ chunk1.read<var<u32>>() };
                if (debugNames > 0) {
                    buffer data(debugNames);
                    chunk1.readAll(data);

                    output("%s\n", data.data());
                }


                //stream.seek(hdr.chunk2_offset);
                //auto chunk2{ readChunk(stream, hdr.chunk2_zsize, hdr.chunk2_size) };

                return true;
            }

            template <template<typename T> class var>
            bool readDataBundle(base::stream& stream, Header<var>& hdr, std::shared_ptr<BundleTextureDB> bundleDb)
            {
                stream.seek(hdr.chunk1_offset);
                
                stream.skip(hdr.chunk1_size);
                stream.skip(hdr.chunk2_size);

                if (!stream.canRead(sizeof(CaffType))) {
                    // todo: just dump it out

                    stream.seek(0);
                    base::fileutils::saveToDisk(stream, stream.getSize(), "bad_Caff.caff");
                }

                auto preInfo{ stream.read<CaffType>() };

                // skip if not a texture
                if (strncmp(preInfo.magic, sc_textureMagic, sizeof(sc_textureMagic) - 1) != 0) {
                    return false;
                }

                if (strncmp(preInfo.version, sc_textureVersion, sizeof(sc_textureVersion) - 1) != 0) {
                    return false;
                }

                auto info{ stream.read<TextureInfo<var>>() };

                if (info.offset > 0) {
                    stream.skip(info.offset);
                }

                auto format = static_cast<X360TextureFormat>(info.textureFormat & 0xFF);

                BundleTextureDB::Texture texture;
                texture.width = info.width;
                texture.height = info.height;
                texture.format = format;

                size_t size = GetTextureDataSize(texture.width, texture.height, format);
                if (size == 0) {
                    return false;
                }

                //dxt1 works fine.. lets check this one
                if (format == X360TextureFormat::CTX1) {

                    buffer temp(size);
                    stream.readImpl(temp.data(), size);

                    img::convertToRGBA(texture.data, temp, texture.width, texture.height, format);
                }
                else if (format == X360TextureFormat::DXT1) {

                    buffer temp(size);
                    stream.readImpl(temp.data(), size);

                    img::convertToRGBA(texture.data, temp, texture.width, texture.height, format);
                }
                else
                {
                    return false;
                    // rawr
                }



                //if (format == TextureFormat360::TEXTURE_FORMAT_DXT1) {
                //    texture.data.resize(size);
                //    stream.readImpl(texture.data.data(), size);

                //    auto newLinear = ConvertToLinearTexture(texture.data, texture.width, texture.height, static_cast<int>(format));

                //    texture.data = DecodeDXT1(newLinear, texture.width, texture.height);
                //} else 
                // if (format == TextureFormat360::TEXTURE_FORMAT_DXN) {
                //    texture.data.resize(size);
                //    stream.readImpl(texture.data.data(), size);

                //    auto newLinear = ConvertToLinearTexture(texture.data, texture.width, texture.height, static_cast<int>(format));

                //    texture.data = DecodeDXN(newLinear, texture.width, texture.height);
                //}

                bundleDb->data.emplace_back(std::move(texture));

                return true;
            }

            void checksumByte(u32& checksum, const u8 currentByte)
            {
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
                constexpr size_t sc_length = 120;

                buffer data(sc_length);

                const auto originalPosition{ stream.getPosition() };

                stream.seek(0);
                stream.readAll(data);

                u32 checksum = 0;

                size_t index = 0;

                // Checksum the first 24 bytes (magic, version string, offset)
                while (index < 24) {
                    checksumByte(checksum, data[index]);
                    ++index;
                }

                // Skip the stored checksum value
                while (index < 28) {
                    checksumByte(checksum, 0);
                    ++index;
                }

                // Checksum the remaining header data
                while (index < sc_length) {
                    checksumByte(checksum, data[index]);
                    ++index;
                }

                stream.seek(originalPosition);

                return checksum;
            }

        }

        BundleReader::BundleReader(std::shared_ptr<BundleLocDB> locData, std::shared_ptr<BundleTextureDB> textureData)
            : m_locData(locData)
            , m_textureData(textureData)
        { }

        bool BundleReader::read(base::stream& stream)
        {
            auto fileHead{ stream.read<CaffFileHeader>() };

            if (strncmp(fileHead.magic, sc_magic, sizeof(sc_magic) - 1) != 0) {
                return false;
            }

            if (strncmp(fileHead.version, sc_version, sizeof(sc_version) - 1) != 0) {
                return false;
            }

            u32 checksum = calculateChecksum(stream);

            auto header{ stream.read<Header<>>() };
            if (header.byteOrder == HeaderByteOrder::little) {
                if (header.checksum != checksum) {
                    assert_always("Checksum failed");
                    return false;
                }

                return testReadData(stream, header);
                //return readDataBundle(stream, header, m_textureData);
            }
            else {
                auto hdrData{ reinterpret_cast<Header<base::endianBig>&>(header) };
                if (hdrData.checksum != checksum) {
                    assert_always("Checksum failed");
                    return false;
                }

                return testReadData(stream, hdrData);
                //return readDataBundle(stream, hdrData, m_textureData);
            }

            return false;
        }
    }
}
