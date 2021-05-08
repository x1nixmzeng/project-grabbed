#include "cue_list.h"

#include "base/assert.h"

namespace grabbed
{
    namespace ghoulies
    {
        size_t CueListDb::GetSize(const std::string src) const
        {
            auto res = m_cuelist.find(src);
            if (res != m_cuelist.end()) {
                return (*res).second.size();
            }

            return 0;
        }

        CueListReader::CueListReader(std::shared_ptr<CueListDb> data)
            : BaseFormat(ResourceType::eResXCueList)
            , m_data(data)
        {
            assert_true(m_data != nullptr);
        }

        bool CueListReader::read(base::stream& stream, Context& context)
        {
            buffer buf(context.header.fileChunk.size);
            stream.readAll(buf);
            
            std::string key, name;

            key.reserve(16); // truncated to 16-characters
            name.reserve(64);

            bool is_key = true;

            auto it = buf.cbegin();

            while (it != buf.cend())
            {
                if (is_key) {
                    /* process end */
                    if ((*it) == '\t') {
                        is_key = false;
                        name.clear();
                        /* add to key buffer */
                    }
                    else {
                        key.push_back(*it);
                    }
                }
                else {
                    /* process end and add cue entry */
                    if ((*it) == '\r') {
                        ++it;
                        assert_true((*it) == '\n');

                        m_data->m_cuelist[key].push_back(name);

                        is_key = true;
                        key.clear();
                    }
                    else {
                        name.push_back(*it);
                    }
                }

                ++it;
            }

            return true;
        }
    }
}
