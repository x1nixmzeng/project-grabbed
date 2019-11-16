#include "language.h"

namespace grabbed
{
    namespace ghoulies
    {
        LanguageProvider::LanguageProvider()
            : m_activeLanguage(Language::English)
        { }

        LanguageProvider::LanguageProvider(Language::Type defaultLanguage)
            : m_activeLanguage(defaultLanguage)
        { }

        Language::Type LanguageProvider::getActiveLanguage() const
        {
            return m_activeLanguage;
        }

        void LanguageProvider::setActiveLanguage(Language::Type language)
        {
            m_activeLanguage = language;
        }
    }
}
