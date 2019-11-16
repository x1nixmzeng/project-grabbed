#include "fileutils.h"

#include <fstream>

namespace grabbed
{
    namespace base
    {
        namespace fileutils
        {
            void saveToDisk(stream& stream, size_t length, string filename)
            {
                auto file = std::ofstream(filename, std::ofstream::binary);
                if (file) {
                    buffer swap(1024);
                    while (length)
                    {
                        size_t count = (length > 1024) ? 1024 : length;
                        stream.read(swap, count);
                        file.write(reinterpret_cast<const char*>(swap.data()), count);
                        length -= count;
                    }
                }

                file.close();
            }
        }
    }
}
