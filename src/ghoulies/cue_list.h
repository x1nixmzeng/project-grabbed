#pragma once

#include "base/stream.h"

#include "ghoulies/formats.h"

#include <vector>
#include <map>
#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        class CueListDb
        {
        public:
            std::map<string, std::vector<string>> m_cuelist;
            size_t GetSize(const string src) const;
        };

        class CueListReader : public BaseFormat
        {
        public:
            CueListReader(std::shared_ptr<CueListDb> data);

            virtual bool read(base::stream& stream, Context& context) override;

        private:
            std::shared_ptr<CueListDb> m_data;
        };
    }
}
