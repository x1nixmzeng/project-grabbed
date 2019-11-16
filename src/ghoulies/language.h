#pragma once

#include "base/types.h"

namespace grabbed
{
    namespace ghoulies
    {
        namespace Language
        {
            enum Type
            {
                English,
                French,
                German,
                Spanish,
                Italian,
                Korean,
                Chinese,
                Japanese,
                Default = English,
            };
            
            constexpr size_t Count = 8;

            constexpr const char* Names[Count] =
            {
                "english",
                "french",
                "german",
                "spanish",
                "italian",
                "korean",
                "chinese",
                "japanese",
            };
        }

        class LanguageProvider
        {
        public:
            LanguageProvider();
            LanguageProvider(Language::Type defaultLanguage);
            
            Language::Type getActiveLanguage() const;
            void setActiveLanguage(Language::Type language);

        protected:
            Language::Type m_activeLanguage;
        };
    }
}
