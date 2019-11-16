#include "bundle_formats.h"

#include "anim.h"
#include "anim_events.h"
#include "model.h"
#include "aid_list.h"
#include "cutscene.h"
#include "cue_list.h"
#include "rumble.h"
#include "actor_goals.h"
#include "loctext.h"
#include "texture.h"
#include "soundbank.h"
#include "font.h"
#include "misc_helpers.h"

#include "base/assert.h"
#include "base/output.h"

#include "base/fileutils.h"

namespace grabbed
{
    namespace ghoulies
    {
        namespace
        {
            class BundleBlockStub : public IBundleFormat
            {
            public:
                virtual void onBlock(base::stream& stream, size_t size)
                {
                    stream.skip(size - 4);
                }

                virtual void read(base::stream& stream, Context& context)
                {
                    size_t count = 0;
                    size_t read = 0;
                    while (read < context.getFileSize())
                    {
                        auto size = stream.read<u32>();
                        read += 4;

                        if (size)
                        {
                            onBlock(stream, size);
                            read += size - 4;
                        }

                        ++count;
                    }
                }
            };

            class BundleActorGoals : public BundleBlockStub { };

            class BundleAidList : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto tempDB{ std::make_shared<AidListDb>() };

                    auto reader{ AidListReader(tempDB) };

                    reader.readBundled(stream, context);

                    output("Loaded %u aid items\n", tempDB->m_resources.size());
                }
            };

            class BundleCutscene : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto tempDB{ std::make_shared<CutsceneDb>() };

                    auto reader{ CutsceneReader(tempDB) };

                    reader.readBundled(stream, context);
                }
            };


            class BundleAnim : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto tempDB{ std::make_shared<AnimDB>() };
                    
                    AnimReader(tempDB).readBundled(stream, context);

                    output("Anim count: %u \n", tempDB->data.size());
                }
            };

            class BundleAnimEvents : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto data = std::make_shared<AnimEventsDB>();

                    AnimEventsReader().readBundled(stream, context);
                }
            };

            class BundleCueList : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto data = std::make_shared<CueListDb>();
                    
                    CueListReader(data).readBundled(stream, context);
                }
            };

            class BundleFxCallout : public BundleBlockStub
            {
                virtual void onBlock(base::stream& stream, size_t size) override
                {
                    if (size == 332)
                    {
                        stream.skip(4);
                        auto name = stream.readString(64);

                        auto soundbank1 = stream.readString(64);
                        auto soundbank2 = stream.readString(64);

                        assert_true(soundbank2[0] == 0, "expected null string. maybe this is 128 bytes?");

                        u32 unknown = stream.read<u32>();

                        stream.skip(128);// null bytes
                    }
                    else
                    {
                        BundleBlockStub::onBlock(stream, size);
                    }
                }
            };

            class BundleParticle : public BundleBlockStub { };

            class BundleCutsceneEvents : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context) override
                {
                    bool dumpIt = false;

                    size_t read = 0;
                    while (read < context.getFileSize())
                    {
                        size_t frameread = 0;

                        auto blockSize = stream.read<u32>();
                        auto t1 = stream.read<f32>();
                        auto t2 = stream.read<f32>();
                        auto blockType = stream.read<u32>();

                        frameread = 4 * 4;

                        if (blockSize >= 144)
                        {
                            if (blockType == 12) {
                                auto unknown = stream.read<u32>();
                                frameread += 4;
                            }

                            auto str = stream.readString(128);
                            frameread += 128;

                            // Test for missing hashes
                            hashutils::makeHash(str);
                        }

                        auto rem{ blockSize - frameread };
                        if (rem != 0) {
                            stream.skip(rem);
                        }

                        read += blockSize;
                    }
                    
                    if (dumpIt) {
                        base::fileutils::saveToDisk(stream, context.getFileSize(), context.name);
                    }
                }
            };

            class BundleEmitter : public BundleBlockStub { };
            
            class BundleModel : public IBundleFormat
            {
                std::shared_ptr<TextureDB> m_textureDb;
            public:

                BundleModel(std::shared_ptr<TextureDB> textureData)
                    : m_textureDb(textureData)
                { }

                virtual void read(base::stream& stream, Context& context)
                {
                    auto models = std::make_shared<ModelDb>();

                    ModelReader(models, m_textureDb).readBundled(stream, context);
                }
            };

            class BundleSoundbank : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto data = std::make_shared<SoundbankDb>();

                    SoundbankReader(data).readBundled(stream, context);
                }
            };

            class BundleScript : public BundleBlockStub { };

            class BundleGhoulieBox : public BundleBlockStub { };

            class BundleGhoulieSpawn : public BundleBlockStub { };

            class BundleMarker : public BundleBlockStub { };
            
            class BundleTexture : public IBundleFormat
            {
                std::shared_ptr<TextureDB> m_textureDb;
            public:

                BundleTexture(std::shared_ptr<TextureDB> textureData)
                    : m_textureDb(textureData)
                { }

                virtual void read(base::stream& stream, Context& context)
                {
                    TextureReader(m_textureDb).readBundled(stream, context);
                }
            };

            class BundleLoctext : public IBundleFormat
            {
                virtual void read(base::stream& stream, Context& context)
                {
                    // stub
                }
            };

            class BundleUnknown3 : public IBundleFormat
            {
                virtual void read(base::stream& stream, Context& context)
                {
                    // No resources
                    assert_true(context.header.resourceChunk.size == 0);

                    auto unknown = stream.read<u32>();
                    auto count = stream.read<u16>();
                    stream.skip(10); // ushort values
                    
                    // stores offsets until 32+eof

                    std::vector<u32> offsets(count);
                    stream.readAll(offsets);

                    // unknown data:
                    {
                        auto un1 = stream.read<u16>();
                        auto count_2 = stream.read<u16>();
                        auto un2 = stream.read<u16>();

                        // pairs of i16 = (0,-1), (16,-1), (32,-1)?
                        stream.skip(count_2 * 2);
                    }

                    for (auto offset : offsets)
                    {
                        stream.seek(context.header.fileChunk.offset + offset);
                        
                        // a few bytes of data per item
                    }
                }
            };

            class BundleFont : public IBundleFormat
            {
            public:
                virtual void read(base::stream& stream, Context& context)
                {
                    auto fontData{ std::make_shared<FontDb>() };

                    FontReader(fontData).readBundled(stream, context);
                }
            };
        }

        BundleFormatRegistry::BundleFormatRegistry(std::shared_ptr<TextureDB> textureData)
            : m_textureDb(textureData)
        {
            //m_registry[ResourceType::eResActorGoals] = std::make_unique<BundleActorGoals>();
            //m_registry[ResourceType::eResAidList] = std::make_unique<BundleAidList>();
            m_registry[ResourceType::eResAnim] = std::make_unique<BundleAnim>();
            m_registry[ResourceType::eResAnimEvents] = std::make_unique<BundleAnimEvents>();
            //m_registry[ResourceType::eResCutscene] = std::make_unique<BundleCutscene>();
            //m_registry[ResourceType::eResCutsceneEvents] = std::make_unique<BundleCutsceneEvents>();
            //m_registry[ResourceType::eResGhoulybox] = std::make_unique<BundleGhoulieBox>();
            //m_registry[ResourceType::eResGhoulyspawn] = std::make_unique<BundleGhoulieSpawn>();
            //m_registry[ResourceType::eResEmitter] = std::make_unique<BundleEmitter>();
            //m_registry[ResourceType::eResXCueList] = std::make_unique<BundleCueList>();
            //m_registry[ResourceType::eResMarker] = std::make_unique<BundleMarker>();
            //m_registry[ResourceType::eResXSoundbank] = std::make_unique<BundleSoundbank>();
            //m_registry[ResourceType::eResScript] = std::make_unique<BundleScript>();
            m_registry[ResourceType::eResTexture] = std::make_unique<BundleTexture>(m_textureDb);
            //m_registry[ResourceType::eResModel] = std::make_unique<BundleModel>(m_textureDb);
            //m_registry[ResourceType::eResFxCallout] = std::make_unique<BundleFxCallout>();
            //m_registry[ResourceType::eResParticle] = std::make_unique<BundleParticle>();
            //m_registry[ResourceType::eResLoctext] = std::make_unique<BundleLoctext>();
            //m_registry[ResourceType::eResUnknown3] = std::make_unique<BundleUnknown3>();
            //m_registry[ResourceType::eResFont] = std::make_unique<BundleFont>();
        }

        bool BundleFormatRegistry::readLoose(base::stream& stream, const char* name)
        {
            ResourceHeader header;
            stream.read(header);
            assert_true(header.chunkCount == 2);

            return readLoose(stream, name, header);
        }

        bool BundleFormatRegistry::readLoose(base::stream& stream, const char* name, ResourceHeader& header)
        {
            Context context;
            context.name = name;
            context.header = header;
            context.fileChunkOffset = 0;
            context.resourceChunkOffset = header.resourceChunk.offset;
            if (header.resourceChunk.size > 0) {
                
                stream.seek(context.resourceChunkOffset);

                ResourceChunkHeader res_header;
                stream.read(res_header);
                assert_true(res_header.size == sizeof(ResourceChunkHeader) + (res_header.itemCount * sizeof(ChunkHeader)));

                auto& resources = context.resourceHeaders;

                resources.resize(res_header.itemCount);
                stream.readAll(resources);
            }

            return readBundle(stream, context);
        }

        bool BundleFormatRegistry::readBundle(base::stream& stream, Context& context)
        {
            auto resolved_type = m_registry.find(context.header.type);
            if (resolved_type != m_registry.end()) {
                //output("Reading %s\n", name);

                // todo: remove and expect bundle formats to seek to where they want
                stream.seek(context.getFileOffset());
                resolved_type->second->read(stream, context);

                if (0)
                {
                    stream.seek(context.getFileOffset());
                    base::fileutils::saveToDisk(stream, context.getFileSize(), context.name);
                }

                return true;
            }
            
            switch (context.header.type)
            {
            case ResourceType::eResMisc:
            {
                if (context.name.compare("aid_misc_ghoulies_actor_introdata") == 0) {
                    MiscIntroData().read(stream, context);
                }
                else if (context.name.compare("aid_misc_ghoulies_background_lightvolumetable") == 0) {
                    MiscLightVolumeTable().read(stream, context);
                }
                else if (context.name.compare("aid_misc_ghoulies_background_parttable") == 0) {
                    MiscPartTable().read(stream, context);
                }
            }
            break;
            case ResourceType::eResRumble:
            case ResourceType::eResXDSP:
            case ResourceType::eResActorAttribs:
            {
                if (0)
                {
                    stream.seek(context.getFileOffset());
                    base::fileutils::saveToDisk(stream, context.getFileSize(), context.name);
                }
            }
                break;
            default:
                //assert_always("unhandled file format");
                break;
            }

            return false;
        }
    }
}