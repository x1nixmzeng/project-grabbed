#include "texture.h"

#include "base/assert.h"

#include <algorithm>

namespace grabbed
{
    namespace ghoulies
    {
        TextureReader::TextureReader(std::shared_ptr<ghoulies::TextureDB> database)
            : BaseFormat(ResourceType::eResTexture)
            , m_textures(database)
        {
            assert_true(m_textures != nullptr);
        }

        bool TextureReader::read(base::stream& stream, Context& context)
        {
            assert_true(context.resourceHeaders.size() == 1, "Expected one resource");

            // Setup the stream to point to the correct data
            stream.seek(context.getFileOffset());

            auto descriptorInfo{ stream.read<TEXTURE_HEADER>() };
            assert_true(descriptorInfo.headerSize == sizeof(TEXTURE_HEADER));

            size_t resourceBase = context.getResourceOffset();
            for (const auto& resource : context.resourceHeaders)
            {
                stream.seek(resourceBase + resource.offset);

                TextureData data{ context.name, "", descriptorInfo, {} };

                data.rawData.resize(resource.size);
                stream.readAll(data.rawData);

                m_textures->m_storage.push_back(data);
            }

            return true;
        }

        bool TextureReader::canAdd(string& name) const
        {
            const auto& data{ m_textures->m_storage };
            auto it = std::find_if(data.begin(), data.end(), [=](const TextureData& item)
            {
                return item.name == name;
            });

            if (it == data.end()) {
                // the texture name was not found
                return true;
            }

            return false;
        }
    }
}
