#pragma once

#include "base/types.h"
#include "base/stream.h"

#include "ghoulies/formats.h"
#include "ghoulies/language.h"

#include <map>
#include <memory>
#include <array>

namespace grabbed
{
    namespace ghoulies
    {
        class LanguageData
        {
        public:
            std::map<std::string, std::wstring> m_strings;
            std::vector<std::string> m_loadedFiles;
        };

        class LocDb
        {
            LocDb() = delete;

        public:
            LocDb(std::shared_ptr<LanguageProvider> languageProvider);

            std::string makeKey(const std::string& context, const std::string& name) const;

            const wchar_t* translate(const std::string& key) const;
            const wchar_t* translate(const std::string& context, const std::string& name) const;

            const LanguageData& getLanguageData() const;
            LanguageData& editLanguageData();

        private:
            
            std::shared_ptr<LanguageProvider> m_languageProvider;
            std::array<LanguageData, Language::Count> m_data;
        };

        class LocReader : public BaseFormat
        {
            struct LOCTEXT_OFFSET
            {
                u32 size;
                u32 offset;
            };

            class LOCTEXT_CHUNK1
            {
            public:
                u32 unknown_1;
                u32 count;

                std::vector<LOCTEXT_OFFSET> m_items;

                void read(base::stream& stream);
            };

#pragma pack(push,1)
            struct LOCTEXT_LSBL_HEADER
            {
                u32 a;
                u32 b;
                u32 c;
                u32 d;
                u32 e;
            };
#pragma pack(pop)

#pragma pack(push,1)
            struct LOCTEXT_CHUNK2_HEAD
            {
                u16 len;
                u32 id;
            };
#pragma pack(pop)

            class LOCTEXT_CHUNK2
            {
            public:
                std::vector<LOCTEXT_CHUNK2_HEAD> info;
                std::vector<std::wstring> wstrings;
                u32 a;
                void read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl);
            };

            struct LOCTEXT_CHUNK3_HEAD
            {
                u16 unknown_1;
                u16 index;
                u32 unknown_2;
            };

            class LOCTEXT_CHUNK3
            {
            public:
                std::vector<LOCTEXT_CHUNK3_HEAD> info;
                std::vector<std::string> strings;
                void read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl);
            };

            class LOCTEXT_CHUNK4 // comments (optional)
            {
            public:
                void read(base::stream& stream, const LOCTEXT_LSBL_HEADER& lsbl);
            };

            class LOCTEXT_CHUNK5 // (optional)
            {
            public:
                void read(base::stream& steam, const LOCTEXT_LSBL_HEADER& lsbl);
            };
            
            void Fixup(LOCTEXT_CHUNK2& strings, LOCTEXT_CHUNK3& names);

            LocReader() = delete;
            LocReader(const LocReader& other) = delete;

        public:
            LocReader(std::shared_ptr<LocDb> database);

            virtual bool read(base::stream& stream, Context& context) override;
            virtual bool canAdd(const std::string& name) const override;

        private:
            std::shared_ptr<LocDb> m_database;
        };
    }
}
