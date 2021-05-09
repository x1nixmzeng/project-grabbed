#pragma once

#include "base/filestream.h"

#include "ghoulies/texture.h"
#include "ghoulies/bundle.h"

#include <memory>

#include "args.h"

namespace grabbed
{
    class bundleinfo
    {
    public:
        bundleinfo();

        std::unique_ptr<base::filestream> openFile(const std::string& filename);

        void readBundle(const std::string& filename);

        void listFiles();

    protected:

        std::shared_ptr<ghoulies::BundleDb> bundleResources;
        std::shared_ptr<ghoulies::TextureDB> textureResources;
    };

    namespace tool
    {
        int main(args::splittype& args)
        {
            bundleinfo bundleInfo;
            
            std::string path;
            if (args.read("bundleinfo", path))
            {
                bundleInfo.readBundle(path);
            }

            bundleInfo.listFiles();

            return 0;
        }
    }
}
