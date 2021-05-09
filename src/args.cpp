#include "args.h"

namespace grabbed
{
    namespace args
    {
        type createArgs(int argc, char** argv)
        {
            int offsetCount{ argc - 1 };
            type result(offsetCount);

            for (int i(0); i < offsetCount; ++i) {
                result[i] = argv[i + 1];
            }
            return result;
        }

        type createArgs(std::string& argList)
        {
            type result;
            size_t last = 0;
            for (; last < argList.size(); )
            {
                auto pos = argList.find("\r\n", last);
                if (pos == argList.npos) {
                    // remaining
                    if (last < argList.size()) {
                        result.push_back(argList.substr(last));
                    }
                    break;
                }
                
                auto size = pos - last;
                if (size > 0) { // and not all whitespace?
                    result.push_back(argList.substr(last, size));
                }

                last = pos + 2;
            }
            
            return result;
        }

        bool splittype::read(const std::string& name, std::string& result) const
        {
            const auto& cit{ mapped.find(name) };
            if (cit != mapped.cend()) {
                result = cit->second;
                return true;
            }

            result.clear();
            return false;
        }

        bool splittype::existsLoose(const std::string& name) const
        {
            return (std::find(loose.begin(), loose.end(), name) != loose.end());
        }

        splittype splitArgs(const type& args)
        {
            splittype result;

            for (const std::string& arg : args) {
                if (arg.empty()) {
                    continue;
                }
                if (arg[0] == '#') {
                    continue;
                }
                auto found{ arg.find('=') };
                if (found != arg.npos) {
                    result.mapped[arg.substr(0, found)] = arg.substr(found + 1);
                }
                else {
                    result.loose.push_back(arg);
                }
            }

            return result;
        }

        splittype createSplitArgs(int argc, char** argv)
        {
            auto args = createArgs(argc, argv);
            return splitArgs(args);
        }
    }
}
