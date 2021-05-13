#include "xzp.h"

#include "base/memorystream.h"
#include "base/streamview.h"

#include "base/assert.h"
#include "base/byte_order.h"

#include "base/stringutils.h"

#include <string_view>
#include <filesystem>

namespace grabbed::xzp
{
    namespace
    {
        constexpr static const std::string_view sc_magic = "XUIZ";

#pragma pack(push,1)
        struct Header
        {
            char magic[4];
            base::endianBig<u32> version;
            base::endianBig<u32> fileSize;
            base::endianBig<u32> un1; // zeor
            base::endianBig<u32> infoSize;
            base::endianBig<u16> files;
        };
#pragma pack(pop)

        static constexpr auto sc_HeaderSize = sizeof(Header);

        struct ItemHeader
        {
            base::endianBig<u32> size;
            base::endianBig<u32> offset;
            std::string path;
        };
    }

    XZPReader::XZPReader(std::shared_ptr<XZPDB> database)
        : m_database(database)
    { }

    bool XZPReader::read(base::stream& stream)
    {
        auto header{ stream.read<Header>() };

        if (strncmp(header.magic, sc_magic.data(), sc_magic.size() - 1) != 0) {
            return false;
        }

        if (header.fileSize != stream.getSize()) {
            return false;
        }

        XZPFile xzpFile;

        base::streamview dataView(stream, header.infoSize);
        const auto baseOffset{ stream.getPosition() };

        for (size_t i = 0; i < header.files; ++i)
        {
            XZPFile::Item xzpItem;

            auto itemSize = dataView.read<base::endianBig<u32>>();
            auto itemOffset = dataView.read<base::endianBig<u32>>();

            auto itemNameSize = dataView.read<u8>();
            auto name = dataView.readArray<base::endianBig<wchar_t>>(itemNameSize);

            // Ugly workaround for fixed-length wide string in big endian
            std::transform(name.begin(), name.end(), std::back_inserter(xzpItem.path), [](const auto& value) { return value; });

            xzpItem.name = std::filesystem::path(xzpItem.path).filename().generic_string();

            xzpItem.data.resize(itemSize);

            stream.seek(baseOffset + itemOffset);
            stream.readAll(xzpItem.data);

            xzpFile.m_items.push_back(xzpItem);
        }

        m_database->m_files.emplace("xzp", xzpFile);

        return true;
    }
}
