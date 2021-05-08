#include "anim.h"

#include "base/assert.h"

namespace grabbed
{
    namespace ghoulies
    {
        namespace
        {
            struct Header
            {
                u32 magic;
                u32 constant;
                f32 duration;
                u32 offset_1; // Can be zero!
                u32 unknown_1; // 0
                u32 linkedBones;
                u16 unknown_2a;
                u16 unknown_2b;
                u32 flags; // unknown - typically 269E0780h
                u32 footerOffset; // Can be zero! - THIS PART SUGGESTS THERE IS MORE?
                u32 footerCount;
                u8 padding[24];
            };

            constexpr static auto hdrSize = sizeof(Header);

            struct Chunk
            {
                u16 unknown_1;
                u16 unknown_2;
                u16 unknown_3;
                u16 unknown_4;
                f32 unknown_5; // 1.0f - dilation?
            };

            constexpr u32 c_animMagic = 'ANIM';
            constexpr u32 c_animConstant = 0x3B3DE0D9;
        }

        AnimReader::AnimReader(std::shared_ptr<AnimDB> data)
            : BaseFormat(ResourceType::eResAnim)
            , m_data(data)
        {
        }

        bool AnimReader::read(base::stream& stream, Context& context)
        {
            auto hdr{ stream.read<Header>() };
            assert_true(hdr.magic == c_animMagic, "Unknown magic");
            assert_true(hdr.constant == c_animConstant, "Unknown constant");

            Anim instance;
            instance.duration = hdr.duration;
            instance.boneCount = hdr.linkedBones;

            auto chunk1{ stream.read<Chunk>() };
            
            // unknown data ..

            // secondary chunk
            if (hdr.offset_1 != 0) {
                stream.seek(context.getFileOffset() + hdr.offset_1);
            
                auto chunk2{ stream.read<Chunk>() };

                // unknown data ..
            }

            // footer data
            if (hdr.footerOffset != 0) {
                stream.seek(context.getFileOffset() + hdr.footerOffset);

                // always seems to use block of 256 data for offsets
                constexpr size_t offsetCount = 64;

                std::vector<u32> actualOffsets;

                auto offsets{ stream.readArray<u32>(offsetCount) };
                for (u32 offset : offsets) {
                    if (offset != 0) {
                        actualOffsets.emplace_back(offset);
                    }
                }
                
                for (u32 offset : actualOffsets) {
                    stream.seek(context.getFileOffset() + offset);

                    // floats may be used for anim frames?
                    // the values very small (10^-5 or so)

                    auto data{ stream.readArray<f32>(hdr.footerCount) };

                    // unknown data ..
                }

                // assert this is the end of the data?
            }
            
            // add instance:
            m_data->data[context.name] = instance;

            return true;
        }

        bool AnimReader::canAdd(const std::string& name) const
        {
            assert_true_once(m_data != nullptr);
            return m_data->data.find(name) == m_data->data.end();
        }
    }
}
