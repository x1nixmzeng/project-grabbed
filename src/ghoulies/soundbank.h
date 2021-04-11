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
            string name;
            std::vector<string> owningBanks;
            size_t count;
        };

        class SoundbankDb
        {
        public:
            std::map<string, SoundbankItem> items;
        };

        class SoundbankReader : public BaseFormat
        {
        public:
            SoundbankReader(std::shared_ptr<SoundbankDb> data);

            virtual bool read(base::stream& stream, Context& context) override;
            virtual bool canAdd(const string& name) const override;

        private:
            std::shared_ptr<SoundbankDb> m_data;
        };
    }
}
