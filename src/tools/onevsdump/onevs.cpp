#include "onevs.h"

#include "base/output.h"
#include "base/filestream.h"

#include "onevs/caff.h"

#include "img/img.h"

#include "args.h"

#include <memory>
#include <filesystem>
#include <cstdint>

namespace grabbed
{
    namespace tool
    {
        namespace
        {
            std::unique_ptr<base::filestream> openFile(const string& filename)
            {
                return std::make_unique<base::filestream>(filename);
            }
        }
        
        int main(args::splittype& args)
        {
            string path;

            auto metadata{ std::make_shared<onevs::MetadataDB>() };

            if (!args.read("onevs", path))
            {
                if (args.loose.size())
                {
                    path = args.loose[0];
                }
            }

            if (std::filesystem::is_directory(path))
            {
                for (auto& file : std::filesystem::directory_iterator(path))
                {
                    if (file.is_regular_file())
                    {
                        auto file = openFile(path);
                        if (file->isOpen()) {
                            onevs::BundleReader(metadata).read(*file);
                        }
                    }
                }
            }
            else if (std::filesystem::is_regular_file(path))
            {
                auto file = openFile(path);
                if (file->isOpen()) {
                    onevs::BundleReader(metadata).read(*file);
                }
            }

            for (const auto& resource : metadata->data)
            {
                printf("%s\n", resource.originalName.c_str());

                std::vector<string> names;

                for (const auto& str : resource.subfiles)
                {
                    // ugly hack to resolve names; not correct
                    if (str.find("_0x0") != str.npos && str.find(".bin") != str.npos) {
                        names.push_back(str);
                    }
                }

                size_t texture{ 0 };
                for (const auto& f : resource.textures)
                {
                    if (f.data.size() > 0)
                    {
                        string filename;
                        if (texture < names.size()) {
                            filename = names[texture];
                        }
                        else {
                            filename = "texture_" + std::to_string(texture);
                        }

                        filename += ".png";

                        // flip rbga to rgba
                        auto* ptr = const_cast<u8*>(f.data.data());

                        for (size_t i = 0; i < f.data.size(); i += 4, ptr += 4) {
                            std::swap(ptr[0], ptr[2]);
                        }

                        auto result = img::savePngFromRGBA(filename, f.width, f.height, f.data);
                        if (result)
                        {
                            printf("  Exported %s (%zux%zu)\n", filename.c_str(), f.width, f.height);
                        }
                    }

                    ++texture;
                }
            }

            return 0;
        }
    }
}
