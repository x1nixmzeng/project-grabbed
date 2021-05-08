#include "kameoxmlblock.h"

#include "base/byte_order.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace kameo
    {
        namespace
        {
            constexpr const char sc_magic[] = "kameoxmlblock";
        
#pragma pack(push,1)
            struct Header
            {
                char signature[20];
                u32 size;
            };
#pragma pack(pop)

            struct PackedInfo
            {
                u8 un_1;
                u8 un_2;
                u8 un_3;
                u8 un_4;
            };

#pragma pack(push,1)
            struct XmlHeader
            {
                PackedInfo flags;
                base::endianBig<u16> nodeCount;
                base::endianBig<u16> un_1;
                base::endianBig<u16> un_2;
                base::endianBig<u16> un_3;
                base::endianBig<u16> un_4;
                base::endianBig<u16> un_5;
                base::endianBig<u16> un_6;
                base::endianBig<u16> un_7;
            };
#pragma pack(pop)
        }

        XmlBlock::XmlBlock()
        { }

        bool XmlBlock::Read(base::stream& stream)
        {
            auto header = stream.read<Header>();

            if (strcmp(header.signature, sc_magic) != 0) {
                // invalid header
                return false;
            }

            if (stream.getSize() != header.size) {
                // stream size is incorrect
                return false;
            }

            auto xmlHeader = stream.read<XmlHeader>();

            // packed flags?
            auto un_1 = stream.readArray<PackedInfo>(xmlHeader.nodeCount);

            // sizes 1?
            auto un_2 = stream.readArray<base::endianBig<u32>>(xmlHeader.nodeCount);

            // single int?
            auto un_3 = stream.read<base::endianBig<u32>>();

            // sizes 2?
            auto un_4 = stream.readArray<base::endianBig<u32>>(xmlHeader.nodeCount);

            // string table here - note the string specialisation to read c-strings!
            auto stringTable = stream.readArray<std::string>(xmlHeader.nodeCount);

            // padded to 4 bytes?!
            
            // todo
            return true;
        }
    }
}
