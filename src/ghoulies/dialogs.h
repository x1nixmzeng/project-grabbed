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

            string name;
            std::vector<std::pair<TokenType, wstring>> tokens;
        };
        
        class Dialog
        {
        public:
            DialogData data;

            Dialog(const wstring& source);

            static bool resolveHead(const wstring& source, string& result);
            static bool resolveEvent(const wstring& source, string& result);
            static bool resolvePowerup(const wstring& source, string& result);

            static bool resolve(const wstring& source, string& result);
        };
    }
}
