#include "misc_helpers.h"

#include "hashutils.h"

#include "base/assert.h"

namespace grabbed
{
    namespace ghoulies
    {
        namespace
        {
            struct IntroData
            {
                char objectTag[64];
                float weighting; // ?
            };

            struct LightVolumeTable
            {
                char aidResource[128]; // could be 2x 64
                float unknown_1;
                float unknown_2;
                float unknown_3;
            };

            struct PartTable
            {
                char name[32];
                char rumbleSoundName[128];
                char particleSoundName[128];
                char aidRumble[128];
                char aidEmitter[128];
                u32 unknown_3;
                u32 unknown_4;
                u32 unknown_5;
            };
        }

        void MiscIntroData::read(base::stream& stream, Context& context)
        {
            stream.seek(context.getFileOffset());
            size_t readCount = 0;

            std::vector<IntroData> introData;

            while (readCount < context.header.fileChunk.size)
            {
                IntroData data;
                readCount += stream.read(data);

                introData.push_back(data);
            }

            assert_true(introData.size() == 25);
        }

        void MiscLightVolumeTable::read(base::stream& stream, Context& context)
        {
            stream.seek(context.getFileOffset());
            size_t readCount = 0;

            std::vector<LightVolumeTable> lightVolumeTable;

            while (readCount < context.header.fileChunk.size)
            {
                LightVolumeTable data;
                readCount += stream.read(data);

                hashutils::makeFileNameHash(data.aidResource);

                lightVolumeTable.push_back(data);
            }
        }

        void MiscPartTable::read(base::stream& stream, Context& context)
        {
            stream.seek(context.getFileOffset());
            size_t readCount = 0;

            std::vector<PartTable> partTable;

            while (readCount < context.header.fileChunk.size)
            {
                PartTable data;
                readCount += stream.read(data);

                if (data.aidEmitter[0] != 0) {
                    hashutils::makeFileNameHash(data.aidEmitter);
                }

                if (data.aidRumble[0] != 0) {
                    hashutils::makeFileNameHash(data.aidRumble);
                }

                partTable.push_back(data);
            }
        }
    }
}

