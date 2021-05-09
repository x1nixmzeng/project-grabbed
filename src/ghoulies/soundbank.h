#pragma once

#include "base/stream.h"
#include "base/memorystream.h"

#include "ghoulies/formats.h"

#include <map>

namespace grabbed
{
    namespace ghoulies
    {
        struct SoundbankItem
        {
            std::string name;
            std::vector<std::string> owningBanks;
            size_t count;
        };

        class SoundbankDb
        {
        public:
            std::map<std::string, SoundbankItem> items;
        };

        class SoundbankReader : public BaseFormat
        {
        public:
            SoundbankReader(std::shared_ptr<SoundbankDb> data);

            virtual bool read(base::stream& stream, Context& context) override;
            virtual bool canAdd(const std::string& name) const override;

        private:
            std::shared_ptr<SoundbankDb> m_data;
        };
    }
}
