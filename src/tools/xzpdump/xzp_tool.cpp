#include "xzp_tool.h"

#include "base/output.h"
#include "base/filestream.h"
#include "base/fileutils.h"

#include "xzp/xzp.h"

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
            std::unique_ptr<base::filestream> openFile(const std::string& filename)
            {
                return std::make_unique<base::filestream>(filename);
            }
        }
        
        int main(args::splittype& args)
        {
            std::string path;

            auto database{ std::make_shared<xzp::XZPDB>() };

            if (args.loose.size())
            {
                path = args.loose[0];
            }

            if (std::filesystem::is_directory(path))
            {
                for (auto& file : std::filesystem::directory_iterator(path))
                {
                    if (file.is_regular_file())
                    {
                        auto file = openFile(path);
                        if (file->isOpen()) {
                            xzp::XZPReader(database).read(*file);
                        }
                    }
                }
            }
            else if (std::filesystem::is_regular_file(path))
            {
                auto file = openFile(path);
                if (file->isOpen()) {
                    xzp::XZPReader(database).read(*file);
                }
            }

            for (const auto& file: database->m_files)
            {
                printf("%s\n", file.first.c_str());

                for (const auto& item : file.second.m_items)
                {
                    base::fileutils::saveToDisk(item.data, item.path);
                    printf("  Exported %s\n", item.path.c_str());
                }
            }

            return 0;
        }
    }
}
