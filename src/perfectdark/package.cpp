#include "package.h"

#include "base/zlibutils.h"

namespace grabbed
{
    namespace perfectdark
    {
        PackageReader::PackageReader(std::shared_ptr<PackageData> data)
            : m_data(data)
        {
        }

        bool PackageReader::read(base::stream& stream)
        {
            buffer data;
            data.resize(stream.getSize());
            stream.readAll(data);

            buffer out;
            if (!base::zlibutils::decompress(data, out)) {
                return false;
            }
            
            base::memorystream& package{ m_data->stream };

            package.resize(out.size());
            package.write(out.size(), out.data());
            package.seek(0);

            return true;
        }
    }
}
