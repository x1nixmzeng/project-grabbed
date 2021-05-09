#pragma once

#include "base/types.h"
#include "base/stream.h"

namespace grabbed
{
    namespace ghoulies
    {
        enum class ResourceType : u32
        {
            // aid_objparams are read from "aid_objparams/blah"?

            // aid_objparams_ghoulies_fx_fadeout
            // aid_objparams_ghoulies_actor_cameron
            // aid_objparams_ghoulies_misc_buttonbat
            // aid_objparams_ghoulies_fx_solidredflash

            eResTexture = 1,
            eResAnim = 2,
            eResUnknown3 = 3,
            eResModel = 4,
            eResAnimEvents = 5,

            eResCutscene = 7,
            eResCutsceneEvents = 8,

            eResMisc = 10,
            eResActorGoals = 11,
            eResMarker = 12,
            eResFxCallout = 13,
            eResAidList = 14,

            eResLoctext = 16,

            eResXSoundbank = 18,
            eResXDSP = 19, // may be unused
            eResXCueList = 20, // this is important for audio filenames
            eResFont = 21,
            eResGhoulybox = 22,
            eResGhoulyspawn = 23,
            eResScript = 24,
            eResActorAttribs = 25,
            eResEmitter = 26,
            eResParticle = 27,
            eResRumble = 28,
            eResShakeCam = 29,

            eResCount
        };

        struct ChunkHeader
        {
            u32 offset;
            u32 size;
        };

        struct ResourceHeader
        {
            ResourceType type;
            u32 un_1;
            u32 un_2;
            u32 chunkCount; // All files define 2 chunks
            ChunkHeader fileChunk;
            ChunkHeader resourceChunk;
        };

        struct ResourceChunkHeader
        {
            u32 size;
            u32 itemCount;
        };
        
        class FormatHelper
        {
        public:
            const char* toString(ResourceType type);

            void findResourceType(base::stream& fs);
            ResourceType getResourceType(base::stream& fs);
        };

        struct Context
        {
            std::string name;
            ResourceHeader header;

            size_t fileChunkOffset{ 0 };
            size_t resourceChunkOffset{ 0 };

            std::vector<ChunkHeader> resourceHeaders;
            
            size_t getFileOffset() const
            {
                return fileChunkOffset + header.fileChunk.offset;
            }

            size_t getResourceOffset() const
            {
                return resourceChunkOffset;
            }

            size_t getFileSize() const
            {
                return header.fileChunk.size;
            }

            size_t getResourceSize() const
            {
                return header.resourceChunk.size;
            }
        };

        class BaseFormat
        {
        protected:
            ResourceType m_underlyingType;
        public:
            BaseFormat(ResourceType type);

            virtual ~BaseFormat() {}

            virtual bool read(base::stream& stream, Context& context) = 0;
            virtual bool canAdd(const std::string& name) const { return true; }

            bool readBundled(base::stream& stream, Context& content);
            bool readLoose(base::stream& stream, std::string& name);
        };

        template<ResourceType Type>
        class KnownFormat : public BaseFormat
        {
        public:
            KnownFormat()
                : BaseFormat(Type)
            { }
        };
    }
}
