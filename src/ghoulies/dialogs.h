#pragma once

#include "base/types.h"

namespace grabbed
{
    namespace ghoulies
    {
        struct DialogData
        {
            enum class TokenType
            {
                Option,
                Text,
            };

            std::string name;
            std::vector<std::pair<TokenType, std::wstring>> tokens;
        };
        
        class Dialog
        {
        public:
            DialogData data;

            Dialog(const std::wstring& source);

            static bool resolveHead(const std::wstring& source, std::string& result);
            static bool resolveEvent(const std::wstring& source, std::string& result);
            static bool resolvePowerup(const std::wstring& source, std::string& result);

            static bool resolve(const std::wstring& source, std::string& result);
        };
    }
}
