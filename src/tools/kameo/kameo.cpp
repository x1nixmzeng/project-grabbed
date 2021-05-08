#include "kameo.h"

#include "base/filestream.h"

#include "kameo/caff.h"
#include "kameo/str.h"

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

            if (args.read("caff", path)) {
                auto caffData = std::make_shared<kameo::CaffDB>();

                auto file = openFile(path);
                if (file->isOpen()) {
                    kameo::CaffReader reader(caffData);
                    reader.read(*file);
                }
            }

            if (args.read("str", path)) {
                auto strData = std::make_shared<kameo::StrDB>();

                auto file = openFile(path);
                if (file->isOpen()) {
                    kameo::StrReader reader(strData);
                    reader.read(*file);
                }
            }

            return 0;
        }
    }
}
