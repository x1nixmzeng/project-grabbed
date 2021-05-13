#pragma once

#include "base/types.h"
#include "base/stream.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace grabbed::xzp
{
    class XZPFile
    {
    public:
        struct Item
        {
            std::string path;
            std::string name;
            buffer data;
        };

        std::vector<Item> m_items;
    };

    class XZPDB
    {
    public:
        std::map<std::string, XZPFile> m_files;
    };

    class XZPReader
    {
    public:
        XZPReader(std::shared_ptr<XZPDB> database);

        bool read(base::stream& stream);

    protected:
        std::shared_ptr<XZPDB> m_database;
    };
}
