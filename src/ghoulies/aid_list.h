#pragma once

#include "ghoulies/formats.h"
#include "ghoulies/hashutils.h"

#include "base/stream.h"

#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        struct AidListData
        {
            std::string res_name;
            hash res_hash = hashutils::s_invalidHash;
        };

        class AidListDb
        {
        public:
            std::vector<AidListData> m_resources;
        };

        class AidListReader : public BaseFormat
        {
        public:
            AidListReader(std::shared_ptr<AidListDb> data);

            virtual bool read(base::stream& stream, Context& context) override;

        private:
            std::shared_ptr<AidListDb> m_aidDatabase;

        };
    }
}
