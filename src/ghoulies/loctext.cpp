#include "loctext.h"

#include "base/assert.h"

#include <algorithm>

namespace grabbed
{
    namespace ghoulies
    {
        LocDb::LocDb(std::shared_ptr<LanguageProvider> languageProvider)
            : m_languageProvider(languageProvider)
        { }

        string LocDb::makeKey(const string& context, const string& name) const
        {
            string key;
            key.append(context);
            key.append("__");
            key.append(name);
            return key;
        }

        const wchar_t* LocDb::translate(const string& key) const
        {
            auto& data = getLanguageData();

            auto result = data.m_strings.find(key);
            if (result != data.m_strings.end())
            {
                return result->second.c_str();
            }

            assert_always("failed to find key %s", key.c_str());
            return L"";
        }

        const wchar_t* LocDb::translate(const string& context, const string& name) const
        {
            return translate(makeKey(context, name));
        }

        const LanguageData& LocDb::getLanguageData() const
        {
            return m_data[m_languageProvider->getActiveLanguage()];
        }

        LanguageData& LocDb::editLanguageData()
        {
            return m_data[m_languageProvider->getActiveLanguage()];
        }

        // ---------------------------------------------------------------------------------------

        LocReader::LocReader(std::shared_ptr<LocDb> data)
            : BaseFormat(ResourceType::eResLoctext)
            , m_database(data)
        {
        }

        void LocReader::LOCTEXT_CHUNK1::read(base::stream& stream)
        {
            u32 chunk_size;
            stream.read(chunk_size);

            stream.read(unknown_1); // used for reference
            stream.read(count);

            struct INFO
            {
                u32 len;
                u16 unknown_1;
                u16 index;
            };

            // todo: this may be useful

            if (count > 0)
            {
                INFO info;
                for (u32 i = 0; i < count; ++i)
                {
                    stream.read(info);
                }

                for (u32 i = 0; i < count; ++i)
                {
                    stream.readCString();
                }
            }
        }

        void LocReader::LOCTEXT_CHUNK2::read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl)
        {
            stream.read(a); // xxx what is this?

            u32 chunk_size;
            stream.read(chunk_size);

            u32 count;
            stream.read(count);

            info.resize(count);
            wstrings.resize(count);

            // --- string hints

            LOCTEXT_CHUNK2_HEAD hd;

            for (u32 i = 0; i < count; ++i)
            {
                stream.read(hd);
                info[i] = hd;
            }

            // extra one (ignored)
            stream.read(hd);

            // --- string data

            for (u32 i = 0; i < count; ++i)
            {
                auto wstr{ stream.readWCString() };

                wstrings[i] = wstr;
            }

            // extra one (ignored)
            stream.readWCString();
        }

        void LocReader::LOCTEXT_CHUNK3::read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl)
        {
            u32 chunk_size;
            stream.read(chunk_size);

            assert_true(chunk_size > 4);

            u32 count;
            stream.read(count);

            info.resize(count);
            strings.resize(count);

            // --- string hints

            LOCTEXT_CHUNK3_HEAD hd;

            for (u32 i = 0; i < count; ++i)
            {
                stream.read(hd);
                info[i] = hd;
            }

            // --- string data

            for (u32 i = 0; i < count; ++i)
            {
                strings[i] = stream.readCString();
            }
        }

        void LocReader::LOCTEXT_CHUNK4::read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl)
        {
            if (lsbl.e == 0)
            {
                return;
            }

            u32 tmp = stream.read<u32>();
            assert_true(tmp - 4 != 0);

            tmp = stream.read<u32>();

            LOCTEXT_CHUNK3_HEAD hd;

            for (u32 i = 0; i < tmp; ++i)
            {
                stream.read(hd);
            }

            for (u32 i = 0; i < tmp; ++i)
            {
                stream.readCString();
            }

        }

        void LocReader::LOCTEXT_CHUNK5::read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl)
        {
            u32 tmp;
            stream.read(tmp); // size
            u32 count;
            stream.read(count);

            if (count > 0)
            {
                for (u32 i = 0; i < count; ++i)
                {
                    u16 tmp_short;
                    stream.read(tmp_short);
                }
            }
        }
        
        bool LocReader::read(base::stream& stream, Context& context)
        {
            // Setup the stream to point to the correct data
            stream.seek(context.getFileOffset());

            LOCTEXT_CHUNK1 c1;
            c1.read(stream);

            const string hint(stream.readString(4));
            assert_true(hint == "LSBL");

            // --- second important header

            LOCTEXT_LSBL_HEADER lsbl;
            stream.read(lsbl);

            assert_true(lsbl.d != 0); // assuming d is c2 offset

            LOCTEXT_CHUNK2 localisedStrings;
            localisedStrings.read(stream, lsbl);

            LOCTEXT_CHUNK3 nameValues;
            nameValues.read(stream, lsbl);

            // are there comments?!
            if (lsbl.e != 0)
            {
                LOCTEXT_CHUNK4 c4;
                c4.read(stream, lsbl);
            }

            if (localisedStrings.a != 0)
            {
                // we have another chunk here (C5)

                LOCTEXT_CHUNK5 c5;
                c5.read(stream, lsbl);
            }

            assert_true(stream.getPosition() == stream.getSize());
            
            Fixup(localisedStrings, nameValues);

            // Mark the file loaded for this language
            m_database->editLanguageData().m_loadedFiles.push_back(context.name);

            stream.ensureFullyRead();
            return true;
        }

        bool LocReader::canAdd(string& name) const
        {
            const auto& loadedData = m_database->getLanguageData().m_loadedFiles;

            return std::find(loadedData.begin(), loadedData.end(), name) == loadedData.end();
        }

        void LocReader::Fixup(LOCTEXT_CHUNK2& strings, LOCTEXT_CHUNK3& names)
        {
            assert_true(strings.wstrings.size() == names.strings.size());

            size_t count = strings.info.size();

            std::vector<size_t> idxs;
            idxs.resize(count);

            for (size_t i = 0; i < count; ++i)
            {
                idxs[names.info[i].index - 1] = i;
            }

            auto& data = m_database->editLanguageData();

            for (size_t i = 0; i < count; ++i)
            {
                auto& value = strings.wstrings[idxs[i]];
                auto& name = names.strings[i];

                data.m_strings[name] = value;
            }
        }
    }
}

