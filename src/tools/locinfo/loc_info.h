#pragma once

#include "base/filestream.h"

#include "ghoulies/language.h"
#include "ghoulies/loctext.h"

#include <memory>

#include "args.h"

namespace grabbed
{
    class locinfo
    {
    public:
        locinfo();

        std::unique_ptr<base::filestream> openFile(const std::string& filename);

        void readLoc(const std::string& filename);

        void listText();

    protected:
        
        std::shared_ptr<ghoulies::LanguageProvider> m_languageProvider;
        std::shared_ptr<ghoulies::LocDb> m_loc;
    };

    namespace tool
    {
        auto main(args::splittype& args) -> int
        {
            locinfo locInfo;

            std::string path;
            if (args.read("locinfo", path))
            {
                locInfo.readLoc(path);
            }

            locInfo.listText();

            return 0;
        }
    }
}
