#include "shake_cam.h"

namespace grabbed
{
    namespace ghoulies
    {
        ShakeCamReader::ShakeCamReader()
            : BaseFormat(ResourceType::eResShakeCam)
        {
        }

        bool ShakeCamReader::read(base::stream& stream, Context& context)
        {
            return false;
        }
    }
}
