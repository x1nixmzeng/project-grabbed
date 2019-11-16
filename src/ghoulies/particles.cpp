#include "particles.h"

namespace grabbed
{
    namespace ghoulies
    {
        ParticlesReader::ParticlesReader()
            : BaseFormat(ResourceType::eResParticle)
        {
        }

        bool ParticlesReader::read(base::stream& stream, Context& context)
        {
            return false;
        }
    }
}
