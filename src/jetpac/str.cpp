#include "str.h"

#include "base/assert.h"
#include "base/memorystream.h"
#include "base/byte_order.h"
#include "base/stringutils.h"

#include <vector>
#include <string>

namespace grabbed
{
    namespace jetpac
    {
        namespace
        {
            using u32b = base::endianBig<u32>;
            using i16b = base::endianBig<i16>;
            using u16b = base::endianBig<u16>;

            constexpr std::string_view static const sc_magic = "LSB2";

            struct ChunkHeader
            {
                u32b size;
                u32b count;
            };

#pragma pack(push,1)
            struct Chunk1
            {
                i16b id;
                u32b offset;
            };
#pragma pack(pop)

            constexpr static const auto sc_Chunk1Size{ sizeof(Chunk1) };
            static_assert(sc_Chunk1Size == 6, "Chunk1 be 6 bytes");

            struct Chunk2
            {
                i16b id;
                u16b un1; // ?
                u32b offset;
            };

            struct Chunk3
            {
                u16b id; // there can be dupes of these
                u16b lineNumber; // or part
                u32b offset;
            };

            struct Header
            {
                u32 magic;
                u32 un1; // 1
                u32 un2; // 2
                u32b dataStart; // 36
                u32b chunks; // 4
                u32b textOffset; // 36
                u32b keyOffset;
                u32b metadataOffset; // optional
                u32b otherOffset; // optional
            };

            constexpr size_t sc_headerSize{ sizeof(Header) };
            static_assert(sc_headerSize == 36, "Header be 36 bytes");

            struct LookupData
            {
                std::map<short, std::string> keys;
                std::map<short, std::string> texts;
                //std::map<short, short> extra; // not sure we get a lot of info from this
                std::map<short, std::vector<std::pair<short, std::string>>> metadata;
            };
        }

        StrReader::StrReader(std::shared_ptr<StrDB> data)
            : m_data(data)
        {
        }

        bool StrReader::read(base::stream& stream)
        {
            auto caffHeader{ stream.readString(4) };
            if (caffHeader != sc_magic) {
                return false;
            }

            stream.seek(0);

            auto header { stream.read<Header>() };

            LookupData lookup;

            {
                stream.seek(header.textOffset);

                auto c1Offset{ stream.getPosition() };
                auto c1{ stream.read<ChunkHeader>() };

                auto size{ static_cast<size_t>(c1.count) + 1 };
                auto c1Headers{ stream.readArray<Chunk1>(size) };

                auto remSize{ c1.size - (stream.getPosition() - c1Offset) };
                auto buf{ stream.readArray<u8>(remSize) };

                auto* stringPool{ reinterpret_cast<wchar_t*>(buf.data()) };

                for (const auto& h : c1Headers)
                {
                    if (h.id == -1) 
                        break;

                    auto* wstr = stringPool + h.offset;
                    auto str = base::stringutils::stringFromPtrBe(wstr);
                    lookup.texts.emplace(h.id, str);
                }
            }

            if (header.keyOffset != 0) {
                stream.seek(header.keyOffset);

                auto c2Offset{ stream.getPosition() };
                auto c2{ stream.read<ChunkHeader>() };

                auto c2Headers{ stream.readArray<Chunk2>(c2.count) };

                auto remSize{ c2.size - (stream.getPosition() - c2Offset) };
                auto buf{ stream.readArray<u8>(remSize) };

                auto* stringPool{ reinterpret_cast<char*>(buf.data()) };

                for (const auto& h : c2Headers)
                {
                    auto* str = stringPool + h.offset;
                    lookup.keys.emplace(h.id, str);

                    //if (h.un1 != 0) {
                    //    lookup.extra.emplace(h.un1, h.id);
                    //}
                }
            }

            if (header.metadataOffset != 0) {
                stream.seek(header.metadataOffset);

                auto c3Offset{ stream.getPosition() };
                auto c3{ stream.read<ChunkHeader>() };

                auto c3Headers{ stream.readArray<Chunk3>(c3.count) };

                auto remSize{ c3.size - (stream.getPosition() - c3Offset) };
                auto buf{ stream.readArray<u8>(remSize) };

                auto* stringPool{ reinterpret_cast<char*>(buf.data()) };

                for (const auto& h : c3Headers)
                {
                    auto* str = stringPool + h.offset;
                    auto& info = lookup.metadata[h.id];
                    info.push_back(std::make_pair(h.lineNumber, str));
                }
            }

            for (const auto& [key, value] : lookup.keys)
            {
                auto text = lookup.texts.find(key);
                if (text != lookup.texts.end()) {
                    m_data->data.emplace(value, text->second);
                }
            }

            return true;
        }
    }
}
