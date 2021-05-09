#pragma once

#include "base/stream.h"
#include "base/memorystream.h"

#include <vector>
#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        class AudioDb
        {
        public:
            struct Instance
            {
                std::string name;
                std::string source;

                u32 channels;
                u32 freq;

                base::memorystream data;
            };

            std::vector<std::shared_ptr<Instance>> items;
        };

        class AudioReader
        {
        public:
            AudioReader(std::shared_ptr<AudioDb> data);

            bool read(base::stream& stream);

        private:
            std::shared_ptr<AudioDb> m_data;
        };
    }
}
