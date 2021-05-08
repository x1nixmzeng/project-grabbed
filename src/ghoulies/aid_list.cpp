#include "aid_list.h"

namespace grabbed
{
    namespace ghoulies
    {
        AidListReader::AidListReader(std::shared_ptr<AidListDb> data)
            : BaseFormat(ResourceType::eResAidList)
            , m_aidDatabase(data)
        { }

        bool AidListReader::read(base::stream& stream, Context& context)
        {
            // Setup the stream to point to the correct data
            stream.seek(context.getFileOffset());

            struct AID_ITEM
            {
                char name[128];
            };

            size_t readCount{ 0 };

            AID_ITEM item;
            while (readCount < context.header.fileChunk.size)
            {
                stream.read(item);
                readCount += sizeof(AID_ITEM);

                if (item.name[0] == 0) {
                    break;
                }

                std::string name{ item.name };

                AidListData entry{ name, hashutils::makeHash(name) };

                m_aidDatabase->m_resources.push_back(entry);
            }

            return true;
        }
    }

}

