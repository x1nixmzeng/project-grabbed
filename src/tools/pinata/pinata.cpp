#include "pinata.h"

#include "base/output.h"
#include "base/filestream.h"

#include "args.h"

#include "pinata/bundle.h"

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
            std::string path;

            if (args.read("pinatabnl", path)) {
                
                auto file = openFile(path);
                if (file->isOpen()) {
                    
                    auto db = std::make_shared<pinata::BundleDB>();

                    pinata::BundleReader(db).read(*file);

                    output("Have %u items", db->data.size());
                }
            }

            return 0;
        }
    }
}
