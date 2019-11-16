#pragma once

#include "ghoulies/formats.h"
#include "ghoulies/bundle_provider.h"

#include "base/stream.h"
#include "base/memorystream.h"

#include <map>

namespace grabbed
{
    namespace ghoulies
    {
        class BundleEntry
        {
        public:
            BundleEntry() = default;
            explicit BundleEntry(const char* bundleName);

            string name;
            u32 hash;
            ResourceType type;
            std::vector<ChunkHeader> resources;
            size_t resourceOffset;
        };

        class BundleDb
        {
        public:
            std::vector<BundleEntry> m_files;
            std::map<ResourceType, unsigned> m_types;

            bool containsFileHash(hash fileHash) const;
            bool containsFilename(const string& filename) const;
        };

        class BundleReader
        {
            struct CHUNK_1_HEADER
            {
                char name[128];
                ResourceHeader common;
            };

            struct BNL_HEADER
            {
                u16 file_count;
                u8 unknown_1; // flags
                u8 unknown_2[5]; // null space
                ChunkHeader chunk_1;
                ChunkHeader chunk_2;
                ChunkHeader chunk_3;
                ChunkHeader chunk_4;
            };

            BNL_HEADER m_header;

            bool ReadHeader(base::stream& stream, base::memorystream& ms);

            void loadFile(base::stream& stream, std::vector<CHUNK_1_HEADER>& files);

        public:
            BundleReader(std::shared_ptr<IBundleProvider> provider, std::shared_ptr<BundleDb> data);

            void allowDebugDump(bool allow);

            bool read(base::stream& stream);

        private:
            std::shared_ptr<IBundleProvider> m_provider;
            std::shared_ptr<BundleDb> m_data;
            bool m_dumpData;
        };
    }
}
