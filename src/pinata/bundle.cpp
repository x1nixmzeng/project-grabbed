#include "bundle.h"

#include "base/memorystream.h"

#include "base/assert.h"
#include "base/zlibutils.h"
#include "base/byte_order.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace pinata
    {
        namespace
        {
            enum class HeaderByteOrder : u8
            {
                little = 0,
                big = 1,
            };

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
                u32 unknown_6; // 0
                u32 unknown_7; // 0
                u32 unknown_8; // 0
                HeaderByteOrder byteOrder;
                u8 flag_2;
                u8 flag_3;
                u8 flag_4;
                u32 unknown_10; // 6
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
            template <template<typename T> class var = base::endianLittle>
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
            bool readBundle(base::stream& stream, Header<var>& hdr, std::shared_ptr<BundleDB> bundleDb)
            {
                stream.seek(hdr.chunk1_offset);

                base::memorystream chunk1; // data chunk
                {
                    buffer inBuffer(hdr.chunk1_zsize);
                    stream.readAll(inBuffer);

                    chunk1.resize(hdr.chunk1_size);
                    assert_true(base::zlibutils::decompress(inBuffer, chunk1));
                }

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
                        assert_true(two == 2); // not endian specific
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

                    chunk1.ensureFullyRead();
                }

                // ignored: just patterns of 12 bytes (u32, u32, u32(1)) - also padded
                //base::memorystream chunk2;
                //{
                //}

                stream.skip(hdr.chunk2_zsize);

                assert_true(chunk3_size > 0);
                assert_true(chunk3_zsize > 0);

                // LSBL

                base::memorystream chunk3;
                {
                    buffer inBuffer(chunk3_zsize);
                    stream.readAll(inBuffer);

                    chunk3.resize(chunk3_size);
                    assert_true(base::zlibutils::decompress(inBuffer, chunk3));
                }


                stream.ensureFullyRead();


                // process chunk3
                {
                    for (const LSBLInfo<var>& item : offsets)
                    {
                        std::vector<string> names;
                        std::vector<wstring> values;

                        assert_true(item.flag_1 == 1);
                        assert_true(item.flag_2 == 2);

                        chunk3.seek(item.offset);

                        Chunk1<var> c1{ chunk3.read<Chunk1<var>>() };
                        //assert_true(c1.unknown_1 == 12, "unknown extra data");

                        // Usually 12 bytes
                        chunk3.skip(c1.unknown_1 - sizeof(c1));

                        // after LSBL header
                        size_t startPos{ chunk3.getPosition() };

                        auto localHeader{ chunk3.read<LSBLHeader<var>>() };
                        
                        assert_true(localHeader.magic == c_magic, "Unexpected magic valid");
                        assert_true(localHeader.unknown_2 == 28, "Expected 28");
                        assert_true(localHeader.unknown_3 == 4, "Expected 4");
                        assert_true(localHeader.unknown_4 == 28, "Expected 28");
                        
                        auto actualCount = localHeader.count +1;
                        auto headers{ chunk3.readArray<Chunk2<var>>(actualCount) };
                        
                        values.resize(actualCount);
                        for (size_t i = 0; i < actualCount; ++i)
                        {
                            auto wstr{ readWCString<var>(chunk3) };
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

        BundleReader::BundleReader(std::shared_ptr<BundleDB> data)
            : m_data(data)
        {
        }

        bool BundleReader::read(base::stream& stream)
        {
            auto caffHeader{ stream.readString(4) };
            if (caffHeader.compare("CAFF") != 0) {
                return false;
            }

            auto caffVersion{ stream.readString(16) };
            if (caffVersion.compare("07.08.06.0036") != 0) {
                return false;
            }
            
            u32 checksum = calculateChecksum(stream);

            auto header{ stream.read<Header<>>() };
            if (header.byteOrder == HeaderByteOrder::little) {
                if (header.checksum != checksum) {
                    assert_always("Checksum failed");
                    return false;
                }
                return readBundle(stream, header, m_data);
            }
            else {
                auto hdrData{ reinterpret_cast<Header<base::endianBig>&>(header) };
                if (hdrData.checksum != checksum) {
                    assert_always("Checksum failed");
                    return false;
                }

                return readBundle(stream, hdrData, m_data);
            }
        }
    }
}
