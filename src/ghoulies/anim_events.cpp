#include "anim_events.h"

namespace grabbed
{
    namespace ghoulies
    {

        AnimEventsReader::AnimEventsReader()
            : BaseFormat(ResourceType::eResAnimEvents)
            , m_data(std::make_shared<AnimEventsDB>())
        {
        }

        bool AnimEventsReader::read(base::stream& stream, Context& context)
        {
            size_t read = 0;
            while (read < context.getFileSize())
            {
                auto size = stream.read<u32>();
                read += 4;

                auto time = stream.read<f32>();
                read += 4;

                if (size > 0)
                {
                    onBlock(stream, size, time);
                    read += size - 8;
                }
            }
            
            return false;
        }

        bool AnimEventsReader::onBlock(base::stream& stream, size_t size, f32 time)
        {
            switch (size)
            {
            case 152: // sfx label
            case 24:
            case 20:
            {
                AnimEventFrame frame;
                frame.time = time;

                // todo: read out event

                m_data->data.emplace_back(frame);
            }
            case 16: // stub? eo-frame?
                break;
            }

            stream.skip(size - 8);
            return true;
        }
    }
}

