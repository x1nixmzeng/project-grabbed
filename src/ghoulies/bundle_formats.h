#pragma once

#include "ghoulies/bundle_provider.h"
#include "ghoulies/formats.h"
#include "ghoulies/texture.h"

#include "base/stream.h"

#include <map>
#include <memory>

namespace grabbed
{
    namespace ghoulies
    {
        class IBundleFormat
        {            
        public:
            virtual ~IBundleFormat() = default;
            virtual void read(base::stream& stream, Context& context) = 0;
        };
        
        class BundleFormatRegistry : public IBundleProvider
        {
            std::map<ResourceType, std::unique_ptr<IBundleFormat>> m_registry;
        public:

            BundleFormatRegistry(std::shared_ptr<ghoulies::TextureDB> textureData);

            bool readLoose(base::stream& stream, const char* name);
            bool readLoose(base::stream& stream, const char* name, ResourceHeader& header);

            virtual bool readBundle(base::stream& stream, Context& context) override;

        private:
            std::shared_ptr<ghoulies::TextureDB> m_textureDb;
        };
    }
}

