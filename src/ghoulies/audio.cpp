#include "audio.h"

#include "base/types.h"
#include "base/assert.h"

namespace grabbed
{
    namespace ghoulies
    {
        namespace
        {
            enum class EntryFormat : u16
            {
                PCM = 0,
            };

            union CHUNK_ID
            {
                u32 raw;
                char code[4];
            };

            struct CHUNK_HEADER
            {
                CHUNK_ID id;
                u32 size;
            };

            struct WAVRIFFHEADER
            {
                CHUNK_HEADER hdr;
                CHUNK_ID riffType;
            };

            struct FORMATCHUNK
            {
                CHUNK_HEADER hdr;
                u16 wFormatTag;
                u16 wChannels;
                u32 dwSamplesPerSec;
                u32 dwAvgBytesPerSec;
                u16 wBlockAlign;
                u16 wBitsPerSample;
            };

            struct DATACHUNK
            {
                CHUNK_HEADER hdr;
            };

            const u16 WAVE_FORMAT_PCM = 0x0001;

            size_t GetWavSize(size_t datalen)
            {
                return (sizeof(WAVRIFFHEADER) + sizeof(FORMATCHUNK) + sizeof(DATACHUNK) + datalen);//- sizeof(CHUNK_HEADER) ;
            }

            void WriteWav(base::memorystream& ms, u32 channels, u32 bits, u32 freq, u32 fsize)
            {
                WAVRIFFHEADER mainHeader;
                memcpy(&mainHeader.hdr.id.raw, "RIFF", 4);
                mainHeader.hdr.size = GetWavSize(fsize) - sizeof(CHUNK_HEADER);
                memcpy(&mainHeader.riffType.raw, "WAVE", 4);

                ms.write(sizeof(WAVRIFFHEADER), &mainHeader);

                FORMATCHUNK fmt;
                memcpy(&fmt.hdr.id.raw, "fmt ", 4);
                fmt.hdr.size = 16;
                fmt.wFormatTag = WAVE_FORMAT_PCM;
                fmt.wChannels = channels;
                fmt.dwSamplesPerSec = freq;
                fmt.wBlockAlign = channels * (bits / 8);
                fmt.dwAvgBytesPerSec = fmt.wBlockAlign * freq; // out of order
                fmt.wBitsPerSample = bits;

                ms.write(sizeof(FORMATCHUNK), &fmt);

                DATACHUNK data;
                memcpy(&data.hdr.id.raw, "data", 4);
                data.hdr.size = fsize;

                ms.write(sizeof(DATACHUNK), &data);
            }

            u32 RepairChannel(EntryFormat format, u32 magic)
            {
                switch (format)
                {
                case EntryFormat::PCM:
                {
                    u32 tmp = (magic - 4) % 32;

                    return (tmp == 0) ? 1 : 2;
                }
                default:
                    return 1;
                }
            }

            u32 RepairSampleRate(EntryFormat format, u32 magic, u32 channels)
            {
                switch (format)
                {
                case EntryFormat::PCM:
                {
                    u32 expected_channels = magic & 0xF;

                    // gbtg doesn't use any 1 channel audio. that IS strange.

                    //4 == 1 channel
                    //8 == 2 channel? why are some coming through as 4?

                    assert_true((expected_channels == 4) || (expected_channels == 8));
                    //          assert_true(expected_channels == 4);

                    u32 sample_rate = (magic >> 5) & 0xFFFF;
                    return sample_rate;
                }
                default:
                    return 0;
                }
            }
        }

        AudioReader::AudioReader(std::shared_ptr<AudioDb> data)
            : m_data(data)
        {
            assert_true(m_data != nullptr);
        }

        namespace
        {
            constexpr const u32 WAVEBANK_TYPE_BUFFER         = 0x00000000U;      // In-memory buffer
            constexpr const u32 WAVEBANK_TYPE_STREAMING      = 0x00000001U;      // Streaming
            constexpr const u32 WAVEBANK_TYPE_MASK           = 0x00000001U;
                                                             
            constexpr const u32 WAVEBANK_FLAGS_ENTRYNAMES    = 0x00010000U;      // Bank includes entry names
            constexpr const u32 WAVEBANK_FLAGS_COMPACT       = 0x00020000U;      // Bank uses compact format
            constexpr const u32 WAVEBANK_FLAGS_SYNC_DISABLED = 0x00040000U;      // Bank is disabled for audition sync
            constexpr const u32 WAVEBANK_FLAGS_SEEKTABLES    = 0x00080000U;      // Bank includes seek tables.
            constexpr const u32 WAVEBANK_FLAGS_MASK          = 0x000F0000U;
        }

        bool AudioReader::read(base::stream& stream)
        {
            union BlockName
            {
                u32 raw;
                char value[4];
            };

            struct AudioHeader
            {
                BlockName name;
                u32 version;
                u32 soHead1;
                u32 soHead2;
                u32 detailsDir;
                u32 detailsDirLength;
                u32 filenameDir;
                u32 filenameDirLen;
                u32 fileOffset;
                u32 unknown_0;
                u32 flags;
                u32 noEntries;
            };
            
            AudioHeader header;
            stream.read(header);
            assert_true(header.name.raw == 'DNBW');

            auto internalName{ stream.readString(16) };

            struct StructureSizes
            {
                u32 entrySize;    // 24
                u32 filenameSize; // 64
                u32 alignment;    // 512
                u32 unknown_0;
            };

            StructureSizes info;
            stream.read(info);

            assert_true(header.detailsDirLength == (header.noEntries * info.entrySize));

            struct Entry
            {
                u16 channel;
                EntryFormat format;
                u32 magic;
                u32 offset;
                u32 size;
                u8 padding[8];
            };
            
            auto entries{ stream.readArray<Entry>(header.noEntries) };

            // no names are stored in ghoulies data
            if (header.filenameDir != 0)
            {
                assert_true(header.filenameDirLen == (header.noEntries * info.filenameSize));
            }

            for (u32 i = 0; i< header.noEntries; ++i)
            {
                u32 fixedChannel = RepairChannel(entries[i].format, entries[i].magic);
                u32 sampleRate = RepairSampleRate(entries[i].format, entries[i].magic, entries[i].channel);

                assert_true((sampleRate == 44100) || (sampleRate == 22050));

                switch (entries[i].format)
                {
                case EntryFormat::PCM:
                {
                    size_t data_size = entries[i].size;
                    size_t file_size = GetWavSize(data_size);

                    auto newData = std::make_shared<AudioDb::Instance>();
                    
                    // Owning bank:
                    newData->source = internalName;

                    // Temporary name (until the cue lists can be resolved)
                    newData->name = internalName;
                    newData->name.append("_");
                    newData->name.append(std::to_string(i));

                    auto& buffer = newData->data;
                    buffer.resize(file_size);

                    size_t off = header.fileOffset + entries[i].offset;
                    stream.seek(off);

                    WriteWav(buffer, fixedChannel, 16, sampleRate, data_size);

                    // read entire file into file
                    stream.readImpl(buffer.getData(), data_size);

                    newData->channels = fixedChannel;
                    newData->freq = sampleRate;

                    // Reset the internal offset
                    buffer.seek(0);
                    
                    m_data->items.push_back(newData);

                    break;
                }
                default:
                    assert_always("unhandled");
                    break;
                }
            }

            return true;
        }
    }
}
