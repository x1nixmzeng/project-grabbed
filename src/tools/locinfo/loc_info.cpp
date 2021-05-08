#include "loc_info.h"

#include "ghoulies/loctext.h"
#include "ghoulies/dialogs.h"

#include "base/output.h"

namespace grabbed
{
    locinfo::locinfo()
        : m_languageProvider(std::make_shared<ghoulies::LanguageProvider>(ghoulies::Language::English))
        , m_loc(std::make_shared<ghoulies::LocDb>(m_languageProvider))
    {
    }

    std::unique_ptr<base::filestream> locinfo::openFile(const std::string& filename)
    {
        return std::make_unique<base::filestream>(filename);
    }

    void locinfo::readLoc(const std::string& filename)
    {
        auto file = openFile(filename);
        if (file->isOpen())
        {
            ghoulies::LocReader reader(m_loc);

            auto name{ file->getFileName() };
            reader.readLoose(*file, name);
        }
    }

    void locinfo::listText()
    {
        const auto& data = m_loc->getLanguageData();

        for (const auto& item : data.m_strings)
        {
            output("Key:    %s\n", item.first.c_str());

            if (item.first.find("dialogs__") == 0)
            {
                ghoulies::Dialog dialog(item.second);

                for (const auto& token : dialog.data.tokens)
                {
#if 0
                    switch (token.first)
                    {
                    case ghoulies::DialogData::TokenType::Text:
                        output("Text:   %s\n", token.second.c_str());
                        break;
                    case ghoulies::DialogData::TokenType::Option:
                        output("Option: %s\n", token.second.c_str());
                        break;
                    }
#else
                    switch (token.first)
                    {
                    case ghoulies::DialogData::TokenType::Text:
                        output("%s", token.second.c_str());
                        break;
                    case ghoulies::DialogData::TokenType::Option:
                        if (token.second == L"line"
                            || token.second == L"linek"
                            || token.second == L"pagek"
                            || token.second == L"page") {
                            output("\n");
                        }
                        break;
                    }
#endif
                }

                output("\n");
            }
            else
            {
                output("Value:  %s\n", item.second.c_str());
            }
        }
    }
}
