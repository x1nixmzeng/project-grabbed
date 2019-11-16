#include "banjokazooie_nb.h"

#include "base/filestream.h"
#include "base/fileutils.h"

#include "banjokazooie_nb/caff.h"
#include "banjokazooie_nb/archive.h"

#include <memory>

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
            auto locData = std::make_shared<banjokazooie_nb::BundleLocDB>();
            auto textureData = std::make_shared<banjokazooie_nb::BundleTextureDB>();

            auto archiveData = std::make_shared<banjokazooie_nb::ArchiveDB>();

            string path;
            if (args.read("banjokazooie_nb", path)) {
                auto file = openFile(path);
                if (file->isOpen()) {
                    banjokazooie_nb::BundleReader reader(locData, textureData);
                    reader.read(*file);
                }
            }

            if (args.read("banjokazooie_nb2", path)) {
                auto file = openFile(path);
                if (file->isOpen()) {
                    banjokazooie_nb::ArchiveReader reader(archiveData);
                    reader.read(*file);
                }
            }

            size_t i = 0;

            // forward archives onto caff reader
            for (auto& fileData : archiveData->files) {
                
                if (args.existsLoose("dumparchives")) {
                    string name = "archive_dump_" + std::to_string(i++);
                    base::fileutils::saveToDisk(fileData.data, fileData.data.getSize(), name);
                }
                else {
                    banjokazooie_nb::BundleReader reader(locData, textureData);
                    if (!reader.read(fileData.data)) {

                        fileData.data.seek(0);

                        // dump out example
                        string name = "bad_file_" + std::to_string(i++);
                        base::fileutils::saveToDisk(fileData.data, fileData.data.getSize(), name);
                    }
                }
            }
            
            return 0;
        }
    }
}
