#include "archive.h"

#include "base/memorystream.h"

#include "base/assert.h"
#include "base/zlibutils.h"
#include "base/byte_order.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace banjokazooie_nb
    {
        namespace
        {
            constexpr const u32 sc_magic = 0x7CB48C43;

            struct ArchiveHeader
            {
                base::endianBig<u32> version; // 0xc
                base::endianBig<u32> fileCount;
                base::endianBig<u32> unknown_hash;
                base::endianBig<u32> poolCount;
            };

#pragma pack(push,1)
            struct ArchiveEntry
            {
                base::endianBig<u32> id; // possible file name hash
                base::endianBig<u32> offset;
                base::endianBig<u32> size;
            };
#pragma pack(pop)
        }

        ArchiveReader::ArchiveReader(std::shared_ptr<ArchiveDB> data)
            : m_data(data)
        {
        }

        bool ArchiveReader::read(base::stream& stream)
        {
            auto magic{ stream.read<u32>() };
            if (magic != sc_magic) {
                return false;
            }

            auto header{ stream.read<ArchiveHeader>() };
            if (header.fileCount == 0) {
                return true;
            }

            stream.skip(header.poolCount * sizeof(u32));
            
            auto entries = stream.readArray<ArchiveEntry>(header.fileCount);
            for (const auto& entry : entries) {

                // some files are blank with 2 stub entries
                if (entry.id == 0 && entry.size == 0) {
                    continue;
                }

                ArchiveItem newItem;
                newItem.id = entry.id;
                newItem.data.resize(entry.size);

                stream.seek(entry.offset);
                stream.readImpl(newItem.data.getData(), newItem.data.getSize());

                m_data->files.emplace_back(std::move(newItem));
            }

            return true;
        }
    }
}
