#include "args.h"

#include <filesystem>
#include <string_view>

#include "base/filestream.h"

namespace grabbed
{
    namespace tool
    {
        int main(args::splittype& args);
    }
}

auto main(int argc, char** argv) -> int
{
    auto args = grabbed::args::createArgs(argc, argv);
    if (argc == 1) {
        
        constexpr static std::string_view sc_defaultArgs = "tool_args.txt";
        
        if (std::filesystem::exists(sc_defaultArgs))
        {
            grabbed::base::filestream fs(sc_defaultArgs);
            if (fs.isOpen()) {
                std::string contents;
                fs.readAll(contents);
                args = grabbed::args::createArgs(contents);
            }
        }
    }

    auto splitArgs = grabbed::args::splitArgs(args);

    return grabbed::tool::main(splitArgs);
}
