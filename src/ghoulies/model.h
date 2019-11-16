#pragma once

#include "base/types.h"
#include "base/stream.h"

#include "ghoulies/formats.h"
#include "ghoulies/texture.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        constexpr static const i16 sc_unlinkedBoneId = -1;

        struct BoneInstance
        {
            i16 parentid;
            i16 id;
            f32 matrix[3];
            f32 un1[3];
            u8 un2[4];
        };

        struct Vec3
        {
            float x;
            float y;
            float z;
        };

        struct Vec2
        {
            float u;
            float v;
        };

        enum class TextureDetailType
        {
            diffuse,
            ao,
            other,
        };

        struct TextureDetail
        {
            u32 index{ 0 };
            TextureDetailType type;
            std::vector<string> shaders;
        };

        struct SubModel
        {
            enum class Type
            {
                tris,
                quads,
            };

            size_t startOffset;
            size_t count;
            Type type;
        };

        struct SubModelInstance
        {
            std::vector<SubModel> models;
            std::vector<TextureDetail> materials;

            std::vector<u16> indicies;
        };

        struct ModelInstance
        {
            using paramtype = std::pair<string, buffer>;
            std::vector<paramtype> params;
            std::vector<BoneInstance> bones;
            
            std::vector<SubModelInstance> submodels;

            std::vector<Vec3> vertices;
            std::vector<Vec2> uvs;

            bool hasParam(const string& name) const;
            bool readParam(const string& name, u32& value) const;
            bool readParam(const string& name, u8& value) const;
        };

        class ModelDb
        {
        public:
            std::map<string, ModelInstance> models;
        };

        class ModelReader : public KnownFormat<ResourceType::eResModel>
        {
        public:
            ModelReader(std::shared_ptr<ModelDb> modelData, std::shared_ptr<TextureDB> textureData);

            virtual bool read(base::stream& stream, Context& context) override;

            virtual bool canAdd(string& name) const override;

            void setOnlyTextures(bool enabled) { m_onlyTextures = enabled; }

        private:

            bool m_onlyTextures{ false };

            enum class Section : u16
            {
                ndGroup = 1,
                ndSkeleton = 2,

                ndRigidSkinIdx = 11,
                ndMtxArray = 12,

                ndShader2 = 17,
                ndShaderParam2 = 18,
                ndVertexBuffer = 19,
                ndPushBuffer = 20,
                ndVertexShader = 21,
                ndBGPushBuffer = 22,
                ndBlendShape = 23,

                // Other named types:

                // ndPixelShaderParam
                // ndPixelShader
                // ndVisibility
                // ndTransform
                // nd2D
                // ndVtxDesc
            };

            struct HeaderItem
            {
                u32 offset;
                Section type;
                u16 unknown_1; // >= 1
                u32 unknown_2; // A[3]
                u32 un_other1; // A[4]
                u32 un_other2; // A[5]
                u32 nextChild; // A[6]
                u32 nextSibling; // A[7]
                u32 un_other5; // A[8]
                u32 un_other6; // A[9]
                u32 un_other7; // A[10]
            };

            constexpr static size_t HeaderItemSize = sizeof(HeaderItem);

            struct Params
            {
                ModelInstance& instance;
                base::stream& stream;
                size_t base;
                Context& context;
            };

            void read_textures(Params& params);
            void read_main_info(Params& params);
            bool read_group(Params& params, HeaderItem& headerItem);
            bool read_pushBuffer(Params& params, HeaderItem& headerItem);
            bool read_BGPushBuffer(Params& params, HeaderItem& headerItem);
            bool read_vertexBuffer(Params& params, HeaderItem& headerItem);
            bool read_skeleton(Params& params, HeaderItem& headerItem);
            bool read_rigidSkinIdx(Params& params, HeaderItem& headerItem);
            bool read_mtxArray(Params& params, HeaderItem& headerItem);
            bool read_shaderParam2(Params& params, HeaderItem& headerItem);
            bool read_vertexShader(Params& params, HeaderItem& headerItem);
            bool read_shader2(Params& params, HeaderItem& headerItem);
            bool read_blendShape(Params& params, HeaderItem& headerItem);

            void read_main(Params& params);

            std::shared_ptr<ModelDb> m_modelDb;
            std::shared_ptr<TextureDB> m_textureDb;
        };
    }
}
