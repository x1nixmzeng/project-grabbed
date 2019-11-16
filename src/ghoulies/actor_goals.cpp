#include "actor_goals.h"

namespace grabbed
{
    namespace ghoulies
    {

        ActorGoalsReader::ActorGoalsReader()
            : BaseFormat(ResourceType::eResActorGoals)
        {
        }

        bool ActorGoalsReader::read(base::stream& stream, Context& context)
        {
            struct Goal
            {
                char name[64];
            };
            
            // stub
            u32 rem = 0;
            while (rem > 0)
            {
                u32 length{ 0 };
                stream.read(length);
                rem -= 4;
                
                switch (length)
                {
                case 120:
                {
                    Goal goal1, goal2;
                    stream.read(goal1);
                    rem -= sizeof(Goal);
                    stream.read(goal2);
                    rem -= sizeof(Goal);

                    u32 weight;
                    stream.read(weight);
                    rem -= 4;
                }
                break;
                case 72:
                {
                    Goal goal1;
                    stream.read(goal1);
                    rem -= sizeof(Goal);
                }
                break;
                }
            }
            
            return false;
        }
    }
}

