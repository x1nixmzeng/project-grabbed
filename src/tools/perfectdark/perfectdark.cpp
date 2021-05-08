#include "perfectdark.h"

#include "base/output.h"
#include "base/filestream.h"

#include "perfectdark/str.h"
#include "perfectdark/caff.h"
#include "perfectdark/package.h"

#include "args.h"

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

            auto pkg{ std::make_shared<perfectdark::PackageData>() };

            if (args.read("perfectdarkpkg", path)) {
                if (perfectdark::PackageReader(pkg).read(*openFile(path))) {
                    // process caff

                    auto db = std::make_shared<perfectdark::CaffDB>();
                    perfectdark::CaffReader(db).read(pkg->stream);
                }
            }

            if (args.read("perfectdarkstr", path)) {
                
                auto file = openFile(path);
                if (file->isOpen()) {
                    
                    auto db = std::make_shared<perfectdark::StrDB>();

                    perfectdark::StrReader(db).read(*file);

                    output("Have %u items", db->data.size());
                }
            }

            if (args.read("perfectdarkcaff", path))
            {
                auto file = openFile(path);
                if (file->isOpen()) {

                    auto db = std::make_shared<perfectdark::CaffDB>();

                    perfectdark::CaffReader(db).read(*file);
                }
            }

            return 0;
        }
    }
}
