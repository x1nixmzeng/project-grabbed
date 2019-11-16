#include "args.h"

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
        grabbed::base::filestream fs("tool_args.txt");
        if (fs.isOpen()) {
            std::string contents;
            fs.readAll(contents);
            args = grabbed::args::createArgs(contents);
        }
    }

    auto splitArgs = grabbed::args::splitArgs(args);

    return grabbed::tool::main(splitArgs);
}
