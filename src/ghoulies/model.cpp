#include "model.h"

#include "base/output.h"
#include "base/assert.h"
#include "base/textureutils.h"

#include <stack>
#include <list>

namespace grabbed
{
    namespace ghoulies
    {
        namespace 
        {
            // 32-bytes (all tail items begin from offset 32)
            struct Header
            {
                u32 tailOffset;
                u32 tailCount;
                u32 unknown[6];
            };
            
            enum class TailType : u32
            {
                Main = 0,
                unknown_1 = 1, // faulty type?
                unknown_2 = 2,

                // 0xc==0xd
                // 0xb == 0x17

                unknown_4 = 4,
                unknown_5 = 5,
                unknown_6 = 6,
                Texture = 7,
                unknown_8 = 8,
                unknown_9 = 9,
                unknown_10 = 10,
                unknown_11 = 11,
                unknown_12 = 12,

                unknown_14 = 14,
                unknown_15 = 15,
                unknown_16 = 16,
                unknown_17 = 17,
                unknown_18 = 18,

                unknown_22 = 22,
            };

            struct TailStruct
            {
                TailType type;
                u32 offset;
            };
        }

        bool ModelInstance::hasParam(const string& name) const
        {
            auto cit = std::find_if(params.begin(), params.end(), [&](const auto& item)
            {
                return item.first == name;
            });

            return (cit != params.cend());
        }

        bool ModelInstance::readParam(const string& name, u32& value) const
        {
            auto cit = std::find_if(params.begin(), params.end(), [&](const auto& item)
            {
                return item.first == name;
            });

            if (cit != params.cend()) {
                if (cit->second.size() == sizeof(u32)) {
                    // bit horrid
                    value = *reinterpret_cast<const u32*>(cit->second.data());
                    return true;
                }
            }

            assert_always_once("Param lookup failed (u32)");
            return false;
        }

        bool ModelInstance::readParam(const string& name, u8& value) const
        {
            auto cit = std::find_if(params.begin(), params.end(), [&](const auto& item)
            {
                return item.first == name;
            });

            if (cit != params.cend()) {
                if (cit->second.size() == sizeof(u8)) {
                    value = *reinterpret_cast<const u8*>(cit->second.data());
                    return true;
                }
            }

            assert_always_once("Param lookup failed (u8)");
            return false;
        }

        ModelReader::ModelReader(std::shared_ptr<ModelDb> modelData, std::shared_ptr<TextureDB> textureData)
            : m_modelDb(modelData)
            , m_textureDb(textureData)
        {
            assert_true(m_modelDb != nullptr);
        }

        bool ModelReader::read(base::stream& stream, Context& context)
        {
            auto basePos = stream.getPosition();

            Header header;
            stream.read(header);

            assert_true(header.tailOffset < context.getFileSize(), "tail offset sanity check");
            assert_true(header.tailOffset + (sizeof(TailStruct) * header.tailCount) == context.getFileSize(), "tail count is incorrect");

            stream.seek(basePos + header.tailOffset);
            
            // Create new instance by name
            ModelInstance& instance = m_modelDb->models[context.name];

            auto tailItems = stream.readArray<TailStruct>(header.tailCount);

            Params params{ instance, stream, basePos, context };

            for (const auto& item : tailItems)
            {
                stream.seek(basePos + item.offset);

                if (m_onlyTextures) {
                    if (item.type != TailType::Texture) {
                        continue;
                    }
                }

                switch (item.type)
                {
                case TailType::Main:
                    read_main(params);
                    break;

                case TailType::Texture:
                    read_textures(params);
                    break;

                case TailType::unknown_1:
                case TailType::unknown_2:
                case TailType::unknown_4:
                case TailType::unknown_5:
                case TailType::unknown_6:
                case TailType::unknown_9:
                case TailType::unknown_10:
                case TailType::unknown_11:
                case TailType::unknown_12:

                case TailType::unknown_14:
                case TailType::unknown_15:
                case TailType::unknown_16:
                case TailType::unknown_17:
                case TailType::unknown_18:

                case TailType::unknown_22:
                    // stub
                    break;

                default:
                    assert_always_once("unhandled case", static_cast<u32>(item.type));
                }
            }
            
            return true;
        }

        bool ModelReader::canAdd(const string& name) const
        {
            return m_modelDb->models.find(name) == m_modelDb->models.end();
        }

        void ModelReader::read_textures(Params& params)
        {
            auto count = params.stream.read<u32>();
            assert_true(count > 0);

            auto offset = params.stream.read<u32>();
            params.stream.seek(params.base + offset);

            std::vector<u32> textureInfo(count);
            params.stream.readAll(textureInfo);
            
            //output("Found %u textures\n", count);
            //output("Have %u resource headers (may mismatch?)\n\n", params.context.resourceHeaders.size());

            if (!m_onlyTextures) {
                u32 paramCount{ 0 };
                if (params.instance.readParam("numTextures", paramCount)) {
                    //output("Have %u textures (named)\n", paramCount);
                }
            }

            for (u32 i = 0; i < count; ++i)
            {
                params.stream.seek(params.base + textureInfo[i]);

                auto texture{ params.stream.read<TEXTURE_HEADER>() };
                assert_true(texture.headerSize == sizeof(TEXTURE_HEADER));

                switch (texture.format)
                {
                case XboxD3DFormat::X_D3DFMT_DXT1:
                case XboxD3DFormat::X_D3DFMT_DXT3:
                case XboxD3DFormat::X_D3DFMT_DXT5:
                case XboxD3DFormat::X_D3DFMT_LIN_A8R8G8B8:
                case XboxD3DFormat::X_D3DFMT_LIN_X8R8G8B8:
                {
                    // Note: models that do not have power of 2 textures are not included as resources (!!)
                    
                    //output("Found %s (%ux%u)\n", base::textureutils::toString(texture.format), texture.width, texture.height);
                    //output("Offset %u\n", texture.texture_offset);
                    //output("Size %u\n", texture.dataSize);
                    
                    if (texture.dataSize == 0) {
                        assert_always_once("Invalid texture size");
                    }
                    else {

                        auto guessedOffset{ params.context.resourceHeaders[0].offset + params.context.resourceChunkOffset };

                        // offset this a bit
                        params.stream.seek(guessedOffset + texture.texture_offset);

                        string name;
                        name.append("texturefile");
                        name.append(std::to_string(i));

                        if (!m_onlyTextures) {
                            assert_true(params.instance.hasParam(name));
                        }

                        TextureData textureItem;
                        textureItem.info = texture;
                        textureItem.context = params.context.name;
                        textureItem.name = name;

                        textureItem.rawData.resize(texture.dataSize);
                        params.stream.readAll(textureItem.rawData);

                        m_textureDb->m_storage.push_back(textureItem);
                    }
                }
                    break;
                default:
                    assert_always("unknown texture format");
                }
            }
        }

        void ModelReader::read_main_info(Params& params)
        {
            u32 count = params.stream.read<u32>();
            u32 offset = params.stream.read<u32>();

            // not sure this is always true
            assert_true(params.base + offset == params.stream.getPosition(), "no need to seek");
            params.stream.seek(params.base + offset);

            struct InfoStruct
            {
                u32 stringOffset;
                u32 metadataOffset; // or end of string block (aligned to 4 bytes)
                u32 metadataSize;
            };

            std::vector<InfoStruct> allInfo(count);
            params.stream.readAll(allInfo);
            
            for (const auto& info : allInfo)
            {
                params.stream.seek(params.base + info.stringOffset);

                auto name{ params.stream.readCString() };
                //output("Reading %s\n", name.c_str());

                ModelInstance::paramtype result;
                
                result.first = name;
                auto& buf = result.second;

                buf.resize(info.metadataSize);

                params.stream.seek(params.base + info.metadataOffset);
                params.stream.readAll(buf);

                params.instance.params.push_back(result);
            }
        }

        bool ModelReader::read_group(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndGroup") == 0, "expected group name");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            // todo: start from here

            //// todo: start new group from here - need to track when this ends?!
            //assert_always_once("todo: track new group");

            return true;
        }

        bool ModelReader::read_pushBuffer(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndPushBuffer") == 0, "expected push buffer");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);
            
            assert_true(params.instance.submodels.size() > 0, "submodel list is empty, this is really bad");
            auto& instance = params.instance.submodels.back();

            if (headerItem.un_other6 == 1) {
                
                struct PushBuffer1
                {
                    u32 triCount; // [0]
                    u32 unknown_2;
                    u32 unknown_3;
                };

                auto B = params.stream.read<PushBuffer1>();
                auto count = B.triCount;

                // indice list:
                auto list = params.stream.readArray<u16>(count);

                //auto bounds = std::minmax_element(list.begin(), list.end());

                SubModel sub;

                assert_true(params.instance.vertices.size() > 0);
                
                sub.startOffset = params.instance.submodels.back().indicies.size();
                sub.count = count;
                sub.type = SubModel::Type::tris;

                instance.models.push_back(sub);

                for (auto& listItem : list) {
                    params.instance.submodels.back().indicies.push_back(listItem);
                }
            }
            else {
                auto countList = params.stream.readArray<u32>(headerItem.un_other6);
                auto offsetList = params.stream.readArray<u32>(headerItem.un_other6);
                auto typeList = params.stream.readArray<u32>(headerItem.un_other6);
                
                std::vector<u16> faceList;

                for (size_t m = 0; m < headerItem.un_other6; ++m) {
                    params.stream.seek(params.base + offsetList[m]);

                    switch (typeList[m])
                    {
                    case 7:
                    {
                        auto indicies = params.stream.readArray<u16>(countList[m]);
                        
                        for (auto& listItem : indicies) {
                            faceList.push_back(listItem);
                        }

                    }
                        break;
                    case 6:
                        // read more halves?
                        params.stream.skip(2 * countList[m]);
                        break;
                    default:
                        assert_always("unknown size");
                    }
                }

                SubModel sub;
                sub.startOffset = params.instance.submodels.back().indicies.size();
                sub.count = faceList.size();
                sub.type = SubModel::Type::quads;

                instance.models.push_back(sub);

                for (auto& face : faceList) {
                    params.instance.submodels.back().indicies.push_back(face);
                }
            }

            return true;
        }

        bool ModelReader::read_BGPushBuffer(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndBGPushBuffer") == 0, "expected ng push buffer");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            assert_true(params.instance.submodels.size() > 0, "submodel list is empty, this is really bad");
            auto& instance = params.instance.submodels.back();

            if (headerItem.un_other6 == 1) {

                struct PushBuffer1
                {
                    u32 triCount;
                    u32 unknown_2;
                    u32 unknown_3;
                };

                auto B = params.stream.read<PushBuffer1>();
                auto count = B.triCount;

                // indice list:
                auto list = params.stream.readArray<u16>(count);

                //auto bounds = std::minmax_element(list.begin(), list.end());
                auto verts = params.instance.vertices.size();

                // todo: investiate why vert count is invalid
                assert_true_once(verts > 0);
                if (verts > 0) {
                    //assert_true(*bounds.second < verts);
                }

                SubModel sub;
                sub.startOffset = params.instance.submodels.back().indicies.size();
                sub.count = list.size();
                sub.type = SubModel::Type::tris;

                instance.models.push_back(sub);

                for (auto& listItem : list) {
                    params.instance.submodels.back().indicies.push_back(listItem);
                }
            }
            else
            {
                // defines quads
                assert_always("unimplemented");
            }

            return true;
        }

        bool ModelReader::read_vertexBuffer(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndVertexBuffer") == 0, "invalid vertex buffer");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            params.stream.seek(params.base + headerItem.un_other6);

            u32 count = headerItem.un_other7;

            enum class SectionInfoType : u8
            {
                skin = 0x0,
                skinWeight = 0x8,
                vertex = 0x9,
                unknown_10 = 0xa,
                unknown_11 = 0xb,
                UV = 0xd,
                unknown_14 = 0xe,
                unknown_15 = 0xf,
                unknown_16 = 0x10,
            };

            struct SectionInfo
            {
                union
                {
                    struct
                    {                       
                        u32 infoSize : 8;
                        u32 infoType : 8;
                        u32 _other : 16;
                    };

                    u32 length;
                };
                u32 unknown[3];
                u32 offset;
                u32 size;
            };
            
            // need a cleaner way to get this
            auto guessedOffset{ params.context.resourceHeaders[0].offset + params.context.resourceChunkOffset };

            std::vector<SectionInfo> sectionInfo(count);
            params.stream.readAll(sectionInfo);

            for (const auto& info : sectionInfo)
            {
                // NOTES: this may be split into bytes
                // Lower 8-bits seem to be the length

                auto infoType = static_cast<SectionInfoType>(info.infoType);

                switch (infoType)
                {
                case SectionInfoType::vertex:
                {
                    assert_true(info.infoSize == 12);
                    u32 count = info.size / info.infoSize;
                    
                    // ensure previous vertex reads are not trashed
                    assert_true(params.instance.vertices.size() == 0);

                    params.stream.seek(guessedOffset + info.offset);
                    params.instance.vertices.resize(count);
                    params.stream.readAll(params.instance.vertices);

                    // 3xf32 "vert pos list"
                    output("verts %u\n", count);
                }
                break;

                case SectionInfoType::UV:
                {
                    assert_true(info.infoSize == 8);
                    u32 count = info.size / info.infoSize;

                    params.stream.seek(guessedOffset + info.offset);
                    params.instance.uvs.resize(count);
                    params.stream.readAll(params.instance.uvs);

                    // 2xf32 uv list
                    output("uv list %u\n", count);
                }
                break;

                case SectionInfoType::skin:
                {
                    assert_true(info.infoSize == 8 || info.infoSize == 16);
                    
                    if (info.size > 0) {
                        u32 count = info.size / info.infoSize;

                        output("skin %u\n", count);
                    }
                }
                break;

                case SectionInfoType::skinWeight:
                {
                    assert_true(info.infoSize == 8 || info.infoSize == 16);
                    
                    if (info.size > 0) {
                        u32 count = info.size / info.infoSize;

                        output("skin weights %u\n", count);
                    }
                }
                break;

                case SectionInfoType::unknown_11:
                {
                    assert_true(info.infoSize == 4);

                    if (info.size > 0) {
                        u32 count = info.size / info.infoSize;

                        output("unknown_11 %u\n", count);
                    }
                }
                break;

                case SectionInfoType::unknown_14:
                case SectionInfoType::unknown_15:
                case SectionInfoType::unknown_16:
                {
                    assert_true(info.infoSize == 8);

                    if (info.size > 0) {
                        u32 count = info.size / 8;

                        // 2xf32 ??
                        output("unknown_14/15/16 %u\n", count);
                    }
                }
                break;

                case SectionInfoType::unknown_10:
                {
                    assert_true(info.infoSize == 12);

                    if (info.size > 0) {
                        u32 count = info.size / 12;

                        // 3xf32 ??
                        output("unknown_10 %u\n", count);
                    }
                }
                break;

                default:
                {
                    assert_true(info.size == 0, "missing section with data");
                    break;
                }
                }
            }

            return true;
        }

        bool ModelReader::read_skeleton(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndSkeleton") == 0, "expected skeleton");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            params.stream.seek(params.base + headerItem.un_other7);

            u32 count = headerItem.un_other6;
            output("contains %u bones\n", count);

            assert_true(params.instance.bones.size() == 0);

            params.instance.bones.resize(count);
            params.stream.readAll(params.instance.bones);

            // todo: we should be able to NAME these bones

            for (size_t bone = 0; bone < params.instance.bones.size(); ++bone)
            {
                const auto& boneParam{ params.instance.params[bone] };

                u32 id{ 0 };
                assert_true(params.instance.readParam(boneParam.first, id));
                //assert_true(id == instance.bones[bone].id);

                //output("Bone %u (%u) %s\n", bone, id, boneParam.first.c_str());
            }

            return true;
        }

        bool ModelReader::read_rigidSkinIdx(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndRigidSkinIdx") == 0, "expected rigid skin index");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);
            
            params.stream.seek(params.base + headerItem.un_other6);


            return true;
        }

        bool ModelReader::read_mtxArray(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndMtxArray") == 0, "expected array");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            params.stream.seek(params.base + headerItem.un_other6);

            struct Info
            {
                i16 id;
                i16 un1;
                i16 un2;
                i16 un3;
            };

            auto info1 = params.stream.read<Info>();

            return true;
        }

        bool ModelReader::read_shaderParam2(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndShaderParam2") == 0, "expected shader param 2");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            params.stream.seek(params.base + headerItem.un_other6);

            struct UnknownStruct1
            {
                u32 unknown_1;  // B[0]
                u32 unknown_2;  // B[1]
                u32 offset;  // B[2]
                u32 count;  // B[3]
                u32 unknown_5;  // B[4]
                u32 unknown_6;  // B[5]
                u32 unknown_7;  // B[6]
                u32 unknown_8;  // B[7]
                u32 unknown_9;  // B[8]
                u32 offset2; // B[9]
                u32 count2; // B[10]
                u32 unknown_12; // B[11]
                u32 unknown_13; // B[12]
            };
            
            auto B{ params.stream.read<UnknownStruct1>() };
            if (B.offset != 0) {
                params.stream.seek(params.base + B.offset);

                assert_true(params.instance.submodels.size() > 0, "submodel list is empty, this is really bad");
                auto& textureLookup = params.instance.submodels.back().materials;
                
                size_t lookupOffset = textureLookup.size();
                
                for (int m = 0; m < B.count; ++m) {
                    struct TextureReference
                    {
                        u32 index;
                        u32 unknown_2;
                        u32 unknown_3;
                        u32 unknown_4;
                        u32 unknown_5;
                        u32 unknown_6;
                        u32 unknown_7;
                    };

                    auto textureRef{ params.stream.read<TextureReference>() };
                    //output("Uses texture index %u\n", textureRef.index);

                    TextureDetail detail;
                    detail.index = textureRef.index;
                    detail.type = TextureDetailType::other;

                    textureLookup.push_back(detail);
                }

                params.stream.seek(params.base + B.offset2);

                struct ShaderInstances
                {
                    u32 nameOffset;
                    u32 unknown_1; // 0
                    u32 index;
                    i32 unknown_3; // -1
                };

                auto vecC{ params.stream.readArray<ShaderInstances>(B.count2) };

                for (const auto& instance : vecC) {
                    assert_true_once(instance.unknown_1 == 0);
                    assert_true_once(instance.unknown_3 == -1);
                    
                    params.stream.seek(params.base + instance.nameOffset);
                    auto shaderName = params.stream.readCString();
                    
                    auto& detail = textureLookup[lookupOffset + instance.index];
                    detail.shaders.push_back(shaderName);
                    
                    if (shaderName == "colour0" || shaderName == "colour1") {
                        detail.type = TextureDetailType::diffuse;
                    }
                    else if (shaderName == "RARE_lightmap0" || shaderName == "RARE_lightmap-1") {
                        detail.type = TextureDetailType::ao;
                    }
                }
            }

            return true;
        }

        bool ModelReader::read_vertexShader(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndVertexShader") == 0, "expected vertex shader");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            // todo: read other params recursively



            return true;
        }

        bool ModelReader::read_shader2(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndShader2") == 0, "expected shader 2");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            return true;
        }

        bool ModelReader::read_blendShape(Params& params, HeaderItem& headerItem)
        {
            params.stream.seek(params.base + headerItem.offset);

            auto str = params.stream.readString(16);
            assert_true(str.compare("ndBlendShape") == 0, "expected blend shape");
            output("%s %u\n", str.c_str(), headerItem.offset + 64);

            return true;
        }

        void ModelReader::read_main(Params& params)
        {
            struct UnknownStruct1
            {
                u32 unknown_0;
                u32 unknown_1;
                u32 unknown_2;
                u32 unknown_3;
                u32 unknown_4;
                u32 unknown_5;
                u32 unknown_6;
                u32 unknown_7;
            };

            UnknownStruct1 unknown_B;
            params.stream.read(unknown_B);

            struct UnknownStruct2
            {
                u32 unknown_0;
                u32 unknown_1;
                u32 unknown_2;
                u32 unknown_3;

                u32 offset;
                u32 unknown_5;
                u32 unknown_6;
                u32 unknown_7;

                u32 unknown_8;
                u32 unknown_9;
                u32 unknown_10;
                u32 unknown_11;

                u32 unknown_12;
            };
            
            UnknownStruct2 unknown_C;
            params.stream.read(unknown_C);

            auto back = params.stream.getPosition();

            // this is seeking to the tail end of the block
            // it reads count, offset - contains (count * sizeof(InfoStruct)) items, followed by strings 
            params.stream.seek(params.base + 64 + unknown_C.offset);

            Params newParams{ params.instance, params.stream, params.base + 64, params.context };
            read_main_info(newParams);

            params.stream.seek(back);
            
            struct inlineHeader
            {
                HeaderItem hdr;
                std::vector<inlineHeader*> children;
            };

            class inlineRead
            {
            public:
                std::list<inlineHeader> items;

                using pairRef = std::pair<size_t, inlineHeader*>;

                void readItem(base::stream& stream, size_t base, pairRef& owner)
                {
                    stream.seek(base + owner.first + 64);

                    inlineHeader item;
                    stream.read(item.hdr);
                    items.push_back(item);

                    if (owner.second) {
                        owner.second->children.push_back(&items.back());
                    }
                }

                void read(base::stream& stream, size_t base)
                {
                    // parent too?
                    std::stack<std::pair<size_t, inlineHeader*>> stack;

                    // first item
                    stack.push({ sizeof(UnknownStruct2), nullptr });

                    while (!stack.empty())
                    {
                        auto top = stack.top();
                        stack.pop();

                        readItem(stream, base, top);

                        if (items.back().hdr.nextSibling != 0) {
                            stack.push({ items.back().hdr.nextSibling, &items.back() });
                        }

                        if (items.back().hdr.nextChild != 0) {
                            stack.push({ items.back().hdr.nextChild, &items.back() });
                        }
                    }
                }
            };
            
            inlineRead reader;
            reader.read(params.stream, params.base);

            bool flag23 = false;
                        
            for (auto& item : reader.items)
            {
                switch (item.hdr.type)
                {
                case Section::ndShader2:
                    read_shader2(newParams, item.hdr);
                    break;

                case Section::ndVertexBuffer:

                    if (!flag23) {
                        newParams.instance.submodels.push_back(SubModelInstance());
                    }

                    read_vertexBuffer(newParams, item.hdr);
                    break;
                case Section::ndSkeleton:
                    read_skeleton(newParams, item.hdr);
                    break;
                case Section::ndShaderParam2:
                    read_shaderParam2(newParams, item.hdr);
                    break;
                case Section::ndPushBuffer:
                    read_pushBuffer(newParams, item.hdr);
                    break;

                case Section::ndBlendShape:
                    
                    flag23 = true;
                    newParams.instance.submodels.push_back(SubModelInstance());

                    read_blendShape(newParams, item.hdr);
                    break;
                }
            }
        }
    }
}
