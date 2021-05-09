#include "fileutils.h"

#include <fstream>

namespace grabbed::base::fileutils
{    
    void saveToDisk(stream& stream, size_t length, const std::string& filename)
    {
        auto file = std::ofstream(filename, std::ofstream::binary);
        if (file) {
            constexpr static const size_t sc_swapSize{ 1024 * 4 };

            buffer swap(sc_swapSize);
            while (length) {
                size_t count = (length > sc_swapSize) ? sc_swapSize : length;
                stream.read(swap, count);
                file.write(reinterpret_cast<const char*>(swap.data()), count);
                length -= count;
            }

            file.close();
        }
    }

    void saveToDisk(const buffer& buffer, const std::string& filename)
    {
        auto file = std::ofstream(filename, std::ofstream::binary);
        if (file) {
            file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
            file.close();
        }
    }
}
