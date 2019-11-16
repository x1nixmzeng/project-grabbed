#include "rumble.h"

namespace grabbed
{
    namespace ghoulies
    {
        RumbleReader::RumbleReader()
            : BaseFormat(ResourceType::eResRumble)
        {
        }

        bool RumbleReader::read(base::stream& stream, Context& context)
        {
            return false;
        }
    }
}

