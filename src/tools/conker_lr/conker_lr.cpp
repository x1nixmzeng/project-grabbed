#include "conker_lr.h"

#include "base/filestream.h"

#include "conker_lr/caff.h"

#include <memory>

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
            auto caffData = std::make_shared<conker_lr::CaffDB>();

            std::string path;
            if (args.read("conkerlr", path)) {
                auto file = openFile(path);
                if (file->isOpen()) {
                    conker_lr::CaffReader reader(caffData);
                    reader.read(*file);
                }
            }

            return 0;
        }
    }
}
