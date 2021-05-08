#include "caff.h"

#include "base/memorystream.h"
#include "base/streamview.h"

#include "base/assert.h"
#include "base/zlibutils.h"
#include "base/byte_order.h"
#include "base/datautils.h"
#include "base/fileutils.h"
#include "base/output.h"
#include "base/textureutils.h"

#include "img/img.h"

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

namespace grabbed
{
    namespace onevs
    {
        namespace
        {
            struct Area
            {
                const std::string_view name;
                const std::string_view version;
            };

            constexpr std::string_view SECTION_STREAM{ ".stream" };
            constexpr std::string_view SECTION_DATA{ ".data" };
            constexpr std::string_view SECTION_GPU{ ".gpu" };

            constexpr const Area CAFF_AREA{ "CAFF", "07.08.06.0036" };
            //set xml=%InetRoot%\public\ext\r1sdk\win32\plugins\dr\font.xml
            //
            //previewer2 -plugin fontplugin2 .\TCB_____.ttf -nodeploy -xml %xml% 
            // -baseOptions/output=.\TCB_____.ttf.bin -input/face="Tw Cen MT Bold" -input/size="64"
            // -characterSet/charSetSelect=2 -characterSet/unicodeRange="0-124" -fontStyle/antialias=1
            // -bitmapFontOptions/antialiasFilterSize="3" -bitmapFontOptions/outputTextureFormat=14

            constexpr const Area RENDER_GRAPH_AREA{ "rendergraph", "01.11.06.0036" };
            constexpr const Area LIGHTING_AREA{ "lighting", "28.10.05.0034" };
            constexpr const Area TEXTURE_AREA{ "texture", "04.05.05.0032" };
            constexpr const Area POSE_AREA{ "pose", "19.12.06.0036" };
            constexpr const Area ANIM_AREA{ "animation", "19.12.06.0036" };
            constexpr const Area FONT_AREA{ "bfont", "22.11.06.0036" };

            constexpr std::string_view ANIM_GROUP1{ "QUAT_UNCOMPRESSED" };
            constexpr std::string_view ANIM_GROUP2{ "UNCOMPRESSED" };
            constexpr std::string_view ANIM_GROUP3{ "BITSTREAM32" };

            // Name is not always fixed? Or name is padded to 8 bytes?
            struct CaffArea
            {
                char Name[10];
                char Version[14];
            };

            struct CaffAreaSrc
            {
                std::string Name;
                std::string Version;
            };

            template <template<typename> class var>
            struct AnimationGroup
            {
                char GroupName[20];
                var<u32> unknown_1;
                var<u32> size; // Of remaining data
            };

            template <template<typename> class var>
            struct Animation
            {
                var<u32> unknown_1;
                var<f32> duration;
                var<u16> unknown_2a;
                var<u16> groupCount; // possibly
                var<u32> groupOffset; // from animation chunk header (from "animation" offset, 40 bytes)
            };

            enum class HeaderByteOrder : u8
            {
                little = 0,
                big = 1,
            };

#pragma pack(push,1)
            template <template<typename> class var>
            struct ResourceHeader
            {
                var<u32> index; // from 1
                var<u32> offset; // relevant to chunk
                var<u32> size;
                u8 chunkId; // index from 1
                u8 unknown; // ?
            };
#pragma pack(pop)

            constexpr static auto ResourceHeaderSize{ sizeof(ResourceHeader<base::endianLittle>) };
            static_assert(ResourceHeaderSize == 14U, "ResourceHeader must be packed to 14 bytes");

            template <template<typename> class var>
            struct ChunkHeader
            {
                var<u32> size;
                u32 unknown_12;
                u32 unknown_13; // 0
                u32 unknown_14; // 0
                var<u32> zsize;
            };

            template <template<typename> class var>
            struct Header
            {
                char magic[4];
                char version[16];

                var<u32> headerSize;

                var<u32> checksum;
                var<u32> count_1; // number of names in pool
                var<u32> count_2; // number of data headers
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

                ChunkHeader<var> chunk1;
                ChunkHeader<var> chunk2;
            };

            using HeaderBe = Header<base::endianBig>;
            using HeaderLe = Header<base::endianLittle>;

            constexpr static auto HeaderSize{ sizeof(HeaderLe) };

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
            template <template<typename> class var>
            struct sectionInfoHeader
            {
                u8 unknown_1;
                var<u32> offset;
                var<u32> unknown_3;  // possible type (0 == stream, 4 == data, 2 == gpu)
                var<u32> size1; // 1 zsize?
                var<u32> unknown_4;
                var<u32> unknown_5;
                var<u32> unknown_6;
                var<u32> unknown_7;
                var<u32> size2; // 2 size?
            };
#pragma pack(pop)

            template <template<typename> class var>
            struct AnimHeader
            {
                var<u32> un;
                var<f32> duration;
                var<u16> un1;
                var<u16> count;
                var<u32> setions[3]; // section offsets
                //var<u32> setion_2; // section offsets
                //var<u32> setion_3; // section offsets
                var<u32> zero1;
                var<u32> offset4; // ??
                var<u32> offset5; // ??

                u32 zeros[3];

                var<u32> totalSize; // header + section data sizes

                u32 un3[4];

                var<f32> playback; // 1.0
                var<u32> zero2;
            };

            constexpr static auto animHeaderSize = sizeof(AnimHeader<base::endianLittle>);

            template <template<typename> class var>
            struct RenderGraphHeader
            {
                var<u32> zero1;
                var<u32> offset1; //
                var<u32> zero2;
                u32 un1[3];
                var<f32> un2[7];
            };

            constexpr static auto RenderGraphHeaderSize = sizeof(RenderGraphHeader<base::endianLittle>);

            template <template<typename> class var>
            struct PoseHeader
            {
                var<u32> boneCount;
                var<u32> zero1;
                var<u32> offset1; // offset to end of PoseInitial (from "animation" root)
                var<u32> zero2;
                var<u32> zero3;
                var<u32> offset2; // offset to begin of PoseInitial struct (from "animation" root)
                var<u32> num;
            };

            constexpr static auto postHeaderSize = sizeof(PoseHeader<base::endianLittle>);

            struct PoseInitial
            {
                f32 a[3];
                f32 b[3];
                f32 c[3];
                f32 something;
                i16 indexes[6]; // bone parents, indexes etc
            };

            constexpr size_t PoseInitialSize = sizeof(PoseInitial);

            // Covers several variations here
            struct PoseTris
            {
                u32 a, b, c;
            };

            template <template<typename> class var>
            struct TextureHeader
            {
                u8 flags[3];
                u8 format;
                var<u32> zero1;
                var<u32> zero2;
                var<u16> width;
                var<u16> height;
                var<i32> unknown_3;
                var<i32> unknown_4;
                var<u32> unknown_5; // 8-bit flags
                var<u32> unknown_6;
            };

            constexpr static auto TextureHeaderSize = sizeof(TextureHeader<base::endianLittle>);

            template <template<typename> class var>
            struct AnimGroupHeader
            {
                char name[20];

                var<u32> un1;
                var<u32> un2;
            };

            constexpr static auto sectionInfoHeaderSize{ sizeof(sectionInfoHeader<base::endianLittle>) };
            static_assert(sectionInfoHeaderSize == 33U, "sectionInfoHeader must be packed");

            CaffAreaSrc readArea(base::stream& stream)
            {
                CaffAreaSrc area;

                area.Name = stream.readCString();

                if (area.Name[0] == 0) {
                    return area;
                }

                base::datautils::sanityCheck(area.Name);
                if (area.Name == "pose") {
                    stream.skip(5);// pose heading is aligned for some reason
                }

                area.Version = stream.readCString();
                base::datautils::sanityCheck(area.Version);

                stream.align(4);

                return area;
            }

            template <template<typename T> class var>
            bool readGpuChunk(base::stream& stream, size_t chunkOffset, const sectionInfoHeader<var>& infoHeader, MetadataDB::Metadata& inst)
            {
                // base really
                const auto so{ chunkOffset };

                // stub

                return false;
            }

            template <template<typename T> class var>
            bool readDataChunk(base::stream& stream, size_t chunkOffset, const sectionInfoHeader<var>& infoHeader, MetadataDB::Metadata& inst)
            {
                // base really
                const auto so{ chunkOffset };

                auto area{ readArea(stream) };

                if (area.Name == ANIM_AREA.name)
                {
                    auto animHdr{ stream.read<AnimHeader<var>>() };

                    // just read these names
                    for (int i = 0; i < 3; ++i)
                    {
                        stream.seek(so + animHdr.setions[i]);
                        auto groupHdr = stream.read<AnimGroupHeader<var>>();
                        base::datautils::sanityCheck(groupHdr.name);

                        printf("%s\n", groupHdr.name);
                    }

                    const auto so2{ so + animHdr.totalSize };
                    stream.seek(so2);

                    auto area2{ readArea(stream) };

                    auto poseHdr{ stream.read<PoseHeader<var>>() };
                    
                    auto o2 = so + poseHdr.offset2;
                    assert_true(o2 == stream.getPosition());

                    // offset1 ends here
                    auto poseInitial{ stream.readArray<PoseInitial>(poseHdr.boneCount) };

                    auto o1 = so + poseHdr.offset1;
                    assert_true(o1 == stream.getPosition());

                    // offset2 ends here

                    /* count */ stream.read<var<u32>>();
                    auto offset3 = stream.read<var<u32>>(); // to first PoseTris chunk
                    /* other */ stream.read<var<u32>>();
                    auto offset4 = stream.read<var<u32>>(); // to second PoseTris chunk

                    auto o3 = so + offset3;
                    assert_true(o3 == stream.getPosition());

                    // offset to string, zero, index (0 based)
                    /* ?? */ stream.readArray<PoseTris>(poseHdr.boneCount);

                    auto o4 = so + offset4;
                    assert_true(o4 == stream.getPosition());

                    // unknown, zero, offset to PoseTris item (above) - mapping of sorts
                    /* ?? */ stream.readArray<PoseTris>(poseHdr.boneCount);

                    // then strings
                    for (int i = 0; i < poseHdr.boneCount; ++i)
                    {
                        auto boneName{ stream.readCString() };
                        base::datautils::sanityCheck(boneName);

                        inst.subfiles.push_back(boneName);
                    }
                    
                    stream.align(4);

                    /* ?? */ stream.read<var<u32>>();
                    /* count */ stream.read<var<u32>>();
                    /* off1 */ stream.read<var<u32>>();
                    /* zero */ stream.read<var<u32>>();
                    /* off2 */ stream.read<var<u32>>();

                    /* ?? */ stream.readArray<PoseTris>(poseHdr.boneCount);

                    /* ?? */ stream.readArray<PoseTris>(poseHdr.boneCount);

                    // then strings
                    for (int i = 0; i < poseHdr.boneCount; ++i)
                    {
                        auto jointName{ stream.readCString() };
                        base::datautils::sanityCheck(jointName);

                        inst.subfiles.push_back(jointName);
                    }

                    stream.align(4);

                    // then eof
                    assert_true(stream.getPosition() == stream.getSize());

                    return true;
                }
                else if (area.Name == RENDER_GRAPH_AREA.name)
                {
                    auto rgHdr{ stream.read<RenderGraphHeader<var>>() };

                    stream.seek(so + rgHdr.offset1);

                    /* count */ stream.read<var<u32>>(); // not of offsets. unknown
                    auto offset1 = stream.read<var<u32>>(); // 
                    auto offset2 = stream.read<var<u32>>(); // 
                    auto offset3 = stream.read<var<u32>>(); // 
                    
                    // skip this data (size variable)
                    stream.skip((offset3 - offset2) * 3);

                    auto jointCount = stream.read<var<u32>>(); // ??
                    auto offset4 = stream.read<var<u32>>(); // 

                    stream.seek(so + offset4);

                    /* count */ stream.read<var<u32>>(); // ??
                    auto offset5 = stream.read<var<u32>>(); // 

                    stream.seek(so + offset5);

                    // more often than not, locators
                    for (size_t i = 0; i < jointCount; ++i)
                    {
                        auto str{ stream.readCString() };
                        base::datautils::sanityCheck(str);

                        inst.strings.push_back(str);
                    }

                    return true;
                }
                else if (area.Name == FONT_AREA.name)
                {
                    // Known; not supported
                    return false;
                }

                return false;
            }

            template <template<typename T> class var>
            bool readCaff(base::stream& stream, Header<var>& hdr, MetadataDB::Metadata& inst)
            {
                stream.seek(hdr.headerSize);

                const auto sectionInfo{ stream.readArray<sectionInfoHeader<var>>(hdr.sectionCount) };
                const auto sectionNames{ stream.readArray<std::string>(hdr.sectionCount) };
                
                // Map the chunk data into these data views
                base::streamview chunk_1(stream, hdr.chunk1.size);
                base::streamview chunk_2(stream, hdr.chunk2.size); // not handled yet

                // Process chunk 1
                const auto poolSize{ chunk_1.read<var<u32>>() };
                const auto nameOffsets{ chunk_1.readArray<var<u32>>(hdr.count_1) };

                // Names match the number of resources - similar to other caff mapping
                if (poolSize > 0) {
                    base::streamview namePool(chunk_1, poolSize);

                    for (size_t i = 0; i < nameOffsets.size(); ++i) {
                        namePool.seek(nameOffsets[i]);

                        auto name{ namePool.readCString() };
                        base::datautils::sanityCheck(name);

                        inst.subfiles.push_back(std::filesystem::path(name).filename().string());
                    }
                }

                const auto originalNameSize{ chunk_1.read<var<u32>>() };
                if (originalNameSize > 0) {
                    // Pool size includes nul-terminator
                    base::streamview originalNamePool(chunk_1, originalNameSize);

                    auto originalName{ originalNamePool.readCString() };
                    base::datautils::sanityCheck(originalName);

                    inst.originalName = std::filesystem::path(originalName).filename().string();
                }

                const auto resourceHeaders{ chunk_1.readArray<ResourceHeader<var>>(hdr.count_2) };

                // Sort resource headers by chunk index
                std::vector<std::vector<ResourceHeader<var>>> resourceList(hdr.sectionCount);
                
                for (const auto& res : resourceHeaders) {
                    auto owningSection{ res.chunkId - 1 }; // id is 1-index based
                    resourceList[owningSection].push_back(res);
                }

                // Resolve section offsets
                std::vector<size_t> offsets(hdr.sectionCount);
                {
                    size_t sizeAcc{ 0 };

                    for (size_t i = 0; i < hdr.sectionCount; ++i) {
                        offsets[i] = sizeAcc;
                        sizeAcc += sectionInfo[i].size1;
                    }
                }

                auto findIndex = [&sectionNames](const std::string& sectionName)
                {
                    size_t index{ 0 };
                    while (index < sectionNames.size()) {
                        if (sectionNames[index] == sectionName) {
                            break;
                        }
                        ++index;
                    }
               
                    assert_true(index < sectionNames.size(), "Section {} does not exist", sectionName);
                    return index;
                };

                const size_t dataIndex{ findIndex(".data") };
                
                const auto so{ hdr.headerSize + hdr.chunk1.size + hdr.chunk2.size };

                // Hacky workaround to extract textures from .data and .gpu sections
                for (const auto& res : resourceList[dataIndex])
                {
                    const auto localOffset{ so + offsets[dataIndex] + res.offset };

                    stream.seek(localOffset);
                    
                    auto area{ readArea(stream) };
                    if (area.Name == TEXTURE_AREA.name) {
                        const auto textureHeader{ stream.read<TextureHeader<var>>() };

                        MetadataDB::TextureData def;

                        def.width = textureHeader.width;
                        def.height = textureHeader.height;

                        // hot swap between common 360 format enum
                        auto tfmt = static_cast<X360TextureFormat>(textureHeader.format);

                        assert_true(tfmt == X360TextureFormat::DXT1 || tfmt == X360TextureFormat::DXT3 || tfmt == X360TextureFormat::DXT5 || tfmt == X360TextureFormat::A8R8G8B8);

                        def.format = base::textureutils::makeGenericType(tfmt);

                        const size_t gpuIndex{ findIndex(".gpu") };

                        for (const auto& gpures : resourceList[gpuIndex])
                        {
                            if (gpures.index == res.index)
                            {
                                const auto localGpuOffset{ so + offsets[gpuIndex] + gpures.offset };

                                stream.seek(localGpuOffset);

                                auto size = GetTextureDataSize(def.width, def.height, tfmt);

                                buffer test;
                                test.resize(size);
                                stream.readAll(test);

                                img::convertToRGBA(def.data, test, def.width, def.height, tfmt);
                               
                                break;
                            }
                        }

                        inst.textures.push_back(def);
                    }
                }

                //-------------------------------------------

                size_t offset{ 0 };

                for (size_t i = 0; i < hdr.sectionCount; ++i)
                {
                    if (sectionNames[i] == SECTION_DATA)
                    {
                        auto localOffset{ so + offset };

                        stream.seek(localOffset);

                        if (!readDataChunk<var>(stream, localOffset, sectionInfo[i], inst))
                        {
                            return false;
                        }
                        break;
                    }
                    else
                    {
                        offset += sectionInfo[i].size1;

                        if (sectionNames[i] == SECTION_GPU)
                        {
                            auto localOffset{ so + offset };

                            stream.seek(localOffset);

                            readGpuChunk<var>(stream, localOffset, sectionInfo[i], inst);
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
                buffer data(HeaderSize);

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
                while (index < HeaderSize) {
                    checksumByte(checksum, data[index]);
                    ++index;
                }

                stream.seek(originalPosition);

                return checksum;
            }
        }

        BundleReader::BundleReader(std::shared_ptr<MetadataDB> metadata)
            : m_metadataData(metadata)
        { }

        bool BundleReader::read(base::stream& stream)
        {
            auto header{ stream.read<HeaderLe>() };

            if (strncmp(header.magic, CAFF_AREA.name.data(), CAFF_AREA.name.size() - 1) != 0) {
                return false;
            }

            if (strncmp(header.version, CAFF_AREA.version.data(), CAFF_AREA.version.size() - 1) != 0) {
                return false;
            }

            const auto checksum = calculateChecksum(stream);

            MetadataDB::Metadata metadata;
            bool success{ false };

            if (header.byteOrder == HeaderByteOrder::little) {
                if (header.checksum != checksum) {
                    assert_always("Checksum failed");
                    return false;
                }

                success = readCaff(stream, header, metadata);
            }
            else {
                auto hdrData{ reinterpret_cast<HeaderBe&>(header) };
                if (hdrData.checksum != checksum) {
                    assert_always("Checksum failed");
                    return false;
                }

                success = readCaff(stream, hdrData, metadata);
            }

            if (success)
            {
                m_metadataData->data.push_back(metadata);
            }

            return success;
        }
    }
}
