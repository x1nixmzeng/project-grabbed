#include "cutscene.h"
#include "hashutils.h"

#include "base/assert.h"

namespace grabbed
{
    namespace ghoulies
    {
        CutsceneReader::CutsceneReader(std::shared_ptr<CutsceneDb> data)
            : BaseFormat(ResourceType::eResCutscene)
            , m_data(data)
        {
            assert_true(m_data != nullptr);
        }

        bool CutsceneReader::read(base::stream& stream, Context& context)
        {
            stream.seek(context.getFileOffset());

            stream.skip(4);
            float duration = stream.read<f32>();

            auto unknown_16 = stream.read<u32>();
            assert_true_once(unknown_16 == 16);
            if (unknown_16 != 16) {
                return false;
            }
            
            auto unknown_24 = stream.read<u32>();
            assert_true_once(unknown_24 == 24);
            if (unknown_24 != 24) {
                return false;
            }
            
            auto aidOffset1 = stream.read<u32>();
            auto aidOffset1Data = stream.read<u32>();

            auto aidOffset2 = stream.read<u32>();
            auto aidOffset3 = stream.read<u32>(); // ??
            
            auto aidOffset3Data = stream.read<u32>(); // ??

            stream.seek(context.getFileOffset() + aidOffset1);
            auto aid1Name = stream.readCString(); // Oddly aligned to 4-bytes
            hashutils::makeFileNameHash(aid1Name);

            stream.seek(context.getFileOffset() + aidOffset2);
            auto aid2Name = stream.readCString(); // Oddly aligned to 4-bytes
            hashutils::makeFileNameHash(aid2Name);

            stream.seek(context.getFileOffset() + aidOffset3);
            auto aid3Name = stream.readCString(); // Oddly aligned to 4-bytes
            hashutils::makeFileNameHash(aid3Name);

            return true;
        }
    }
}
