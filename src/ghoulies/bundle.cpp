#include "bundle.h"

#include "formats.h"
#include "model.h"
#include "aid_list.h"
#include "cue_list.h"
#include "rumble.h"
#include "actor_goals.h"
#include "hashutils.h"

#include "base/assert.h"
#include "base/fileutils.h"
#include "base/zlibutils.h"

#include <algorithm>

namespace grabbed
{
    namespace ghoulies
    {
        BundleEntry::BundleEntry(const char* bundleName)
        {
            name = bundleName;
            hash = hashutils::makeHash(name);
            resourceOffset = 0;
        }

        bool BundleDb::containsFileHash(hash fileHash) const
        {
            auto result = std::find_if(m_files.begin(), m_files.end(), [=](const auto& target)
            {
                return target.hash == fileHash;
            });

            return (result != m_files.end());
        }

        bool BundleDb::containsFilename(const std::string& filename) const
        {
            u32 fname_hash{ hashutils::makeHash(filename) };
            
            return containsFileHash(fname_hash);
        }
        
        bool BundleReader::ReadHeader(base::stream& stream, base::memorystream& ms)
        {
            stream.read(m_header);
            assert_true(m_header.unknown_1 == 0x1);

            size_t compressedSize{ stream.getSize() - stream.getPosition() };

            buffer inBuffer(compressedSize);
            stream.readAll(inBuffer);

            size_t uncompressedSize{ m_header.chunk_1.size
                + m_header.chunk_2.size
                + m_header.chunk_3.size
                + m_header.chunk_4.size };

            ms.resize(uncompressedSize);

            bool result = base::zlibutils::decompress(inBuffer, ms);
            if (result)
            {
                // fixup pointers (we split the headers and zlib data)
                m_header.chunk_1.offset -= sizeof(BNL_HEADER);
                m_header.chunk_2.offset -= sizeof(BNL_HEADER);
                m_header.chunk_3.offset -= sizeof(BNL_HEADER);
                m_header.chunk_4.offset -= sizeof(BNL_HEADER);
            }

            return result;
        }

        void BundleReader::loadFile(base::stream& stream, std::vector<CHUNK_1_HEADER>& files)
        {
            for (CHUNK_1_HEADER& file : files)
            {
#if DEBUG
                auto hash = hashutils::makeHash(file.name);
#endif

                auto res = std::find_if(std::begin(m_data->m_files), std::end(m_data->m_files), [&file](const BundleEntry& be)
                {
                    return (be.name.compare(file.name) == 0);
                });

                Context context;

                context.name = file.name;
                context.header = file.common;
                context.fileChunkOffset = m_header.chunk_4.offset;
                
                assert_true(res != m_data->m_files.end());

                // Copy out resource headers
                if (res != m_data->m_files.end()) {
                    context.resourceHeaders = res->resources;
                    context.resourceChunkOffset = res->resourceOffset;
                }
        
                m_provider->readBundle(stream, context);
            }
        }

        BundleReader::BundleReader(std::shared_ptr<IBundleProvider> provider, std::shared_ptr<BundleDb> data)
            : m_provider(provider)
            , m_data(data)
            , m_dumpData(false)
        {
            assert_true(m_provider != nullptr);
            assert_true(m_data != nullptr);
        }

        void BundleReader::allowDebugDump(bool allow)
        {
            m_dumpData = allow;
        }

        bool BundleReader::read(base::stream& stream)
        {
            base::memorystream ms;

            if (ReadHeader(stream, ms)) {

                if (m_dumpData) {
                    std::string name;
                    name.append("bundle_dump.bnlunc");
                    base::fileutils::saveToDisk(ms, ms.getSize(), name);
                    ms.seek(0);
                }

                const u32 c1_count(m_header.chunk_1.size / sizeof(CHUNK_1_HEADER));

                // must match the header
                assert_true(c1_count == m_header.file_count);
                assert_true(c1_count > 0);

                // read all chunk_1
                auto chunk_1{ ms.readArray<CHUNK_1_HEADER>(c1_count) };

                auto it = chunk_1.begin();

                while (it != chunk_1.end()) {

                    BundleEntry entry(it->name);

                    entry.type = it->common.type;
                    
                    // fetch resources
                    if (it->common.resourceChunk.size != 0)
                    {
                        const u32 startPos = m_header.chunk_2.offset + it->common.resourceChunk.offset;

                        ms.seek(startPos);

                        ResourceChunkHeader res_header;
                        ms.read(res_header);

                        assert_true(res_header.size == sizeof(ResourceChunkHeader) + (res_header.itemCount * sizeof(ChunkHeader)));

                        entry.resources.resize(res_header.itemCount);
                        ms.readAll(entry.resources);

                        entry.resourceOffset = m_header.chunk_3.offset;
                    }

                    m_data->m_files.push_back(entry);
                    m_data->m_types[entry.type] += 1;

                    ++it;
                }

                loadFile(ms, chunk_1);

                return true;
            }

            return false;
        }
    }

}

