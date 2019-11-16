#include "emitter.h"

namespace grabbed
{
    namespace ghoulies
    {
        EmitterReader::EmitterReader()
            : BaseFormat(ResourceType::eResEmitter)
        {
        }

        bool EmitterReader::read(base::stream& stream, Context& context)
        {
            return false;
        }
    }
}
