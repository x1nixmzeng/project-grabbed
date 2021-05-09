#include "formats.h"

#include "base/assert.h"

namespace grabbed
{
    namespace ghoulies
    {
        const char* FormatHelper::toString(ResourceType type)
        {
            switch (type)
            {
            case ResourceType::eResTexture:
                return "texture";
            case ResourceType::eResAnim:
                return "anim";
            case ResourceType::eResUnknown3:
                return "unknown_3";
            case ResourceType::eResModel:
                return "model";
            case ResourceType::eResAnimEvents:
                return "anim events";

            case ResourceType::eResCutscene:
                return "cutscene";
            case ResourceType::eResCutsceneEvents:
                return "cutscene events";

            case ResourceType::eResMisc:
                return "misc";
            case ResourceType::eResActorGoals:
                return "actor goals";
            case ResourceType::eResMarker:
                return "marker";
            case ResourceType::eResFxCallout:
                return "fx callout";
            case ResourceType::eResAidList:
                return "aid list";

            case ResourceType::eResLoctext:
                return "loctext";

            case ResourceType::eResXSoundbank:
                return "soundbank";
            case ResourceType::eResXDSP:
                return "dsp";
            case ResourceType::eResXCueList:
                return "cut list";
            case ResourceType::eResFont:
                return "font";
            case ResourceType::eResGhoulybox:
                return "ghouly box"; //?
            case ResourceType::eResGhoulyspawn:
                return "ghouly spawn"; //?
            case ResourceType::eResScript:
                return "script";
            case ResourceType::eResActorAttribs:
                return "actor attribs";
            case ResourceType::eResEmitter:
                return "emitter";
            case ResourceType::eResParticle:
                return "particles";
            case ResourceType::eResRumble:
                return "rumble";
            case ResourceType::eResShakeCam:
                return "shakecam";

            default:
                assert_always_once("unknown resource type", type);
                return "unknown type";
            }
        }

        void FormatHelper::findResourceType(base::stream& fs)
        {
            ResourceType resType{ getResourceType(fs) };

            printf("resource: %s\n", toString(resType));
        }

        ResourceType FormatHelper::getResourceType(base::stream& fs)
        {
            ResourceType resType;
            fs.read(resType);

            return resType;
        }

        BaseFormat::BaseFormat(ResourceType type)
            : m_underlyingType(type)
        {
        }

        bool BaseFormat::readBundled(base::stream& stream, Context& context)
        {
            assert_true(context.header.type == m_underlyingType, "Type mismatch");

            // resource already exists
            if (canAdd(context.name) == false) {
                return false;
            }

            return read(stream, context);
        }

        bool BaseFormat::readLoose(base::stream& stream, std::string& name)
        {
            // resource already exists
            if (canAdd(name) == false) {
                return false;
            }

            Context context;
            
            context.name = name;

            stream.read(context.header);
            assert_true(context.header.type == m_underlyingType, "Type mismatch");

            context.fileChunkOffset = 0;
            context.resourceChunkOffset = sizeof(ResourceHeader);
            
            if (context.header.resourceChunk.size != 0)
            {
                stream.seek(context.header.resourceChunk.offset);
                
                ResourceChunkHeader resourceHeader;
                stream.read(resourceHeader);

                auto& headers = context.resourceHeaders;
                headers.resize(resourceHeader.itemCount);
                stream.readAll(headers);
            }

            return read(stream, context);
        }
    }
}
