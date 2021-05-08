#include "hashutils.h"

#include "base/fileutils.h"
#include "base/stringutils.h"

#include "base/assert.h"

namespace grabbed
{
    namespace hashutils
    {
        void loadHashLine(std::string& string, type& output, const delegate& method)
        {
            size_t col1 = string.find(',');
            if (col1 == string.npos) {
                return;
            }

            size_t col2 = string.find(',', col1 + 1);
            if (col2 == string.npos) {
                return;
            }

            auto hash{ std::strtoul(string.c_str(), nullptr, 16) };
            if (output.find(hash) != output.end()) {
                return;
            }

            data& dataItem{ output[hash] };
            if (col1 > 0) {
                dataItem.context = string.substr(col1 + 1, col2 - (1 + col1));
            }
            
            auto newSource = string.substr(col2 + 1);
            if (!newSource.empty()) {

                if (!dataItem.source.empty()) {
                    assert_true(newSource == dataItem.source, "Source string has changed? Investigate.");
                }
                
                dataItem.source = newSource;
            }
            
            if (dataItem.source.size() > 0) {
                assert_true(method(dataItem.source) == hash);
            }
        }

        bool loadHashFile(base::stream& stream, type& output, const delegate& method)
        {
            if (!stream.isOpen()) {
                return false;
            }

            std::string wholeFile;
            stream.readAll(wholeFile);

            size_t offset{ 0 };
            size_t length{ 0 };

            while (base::stringutils::readLine(wholeFile, offset, length)) {
                auto line{ wholeFile.substr(offset, length) };

                loadHashLine(line, output, method);
            }

            return true;
        }
    }
}
