#include "dialogs.h"

#include "base/stringutils.h"

#include <map>

namespace grabbed
{
    namespace ghoulies
    {
        Dialog::Dialog(const wstring& source)
        {
            DialogData::TokenType current = DialogData::TokenType::Text;
            size_t stringStart = 0;
            size_t currentPosition = 0;

            while (currentPosition < source.size())
            {
                switch (current)
                {
                case DialogData::TokenType::Option:
                {
                    if (source[currentPosition] == '}')
                    {
                        wstring payload = source.substr(stringStart, currentPosition - stringStart);
                        data.tokens.push_back(std::make_pair(DialogData::TokenType::Option, payload));

                        stringStart = currentPosition + 1;
                        current = DialogData::TokenType::Text;
                    }
                }
                break;

                case DialogData::TokenType::Text:
                {
                    if (source[currentPosition] == '{')
                    {
                        if (stringStart < currentPosition)
                        {
                            wstring payload = source.substr(stringStart, currentPosition - stringStart);
                            data.tokens.push_back(std::make_pair(DialogData::TokenType::Text, payload));
                        }

                        stringStart = currentPosition + 1;
                        current = DialogData::TokenType::Option;
                    }
                }
                break;
                }

                ++currentPosition;
            }

            // Fixup missing end tags if this required
            if (stringStart < currentPosition)
            {
                wstring payload = source.substr(stringStart, currentPosition - stringStart);
                data.tokens.push_back(std::make_pair(DialogData::TokenType::Text, payload));

                data.tokens.push_back(std::make_pair(DialogData::TokenType::Option, L"end"));
            }
        }
    
        namespace
        {
            constexpr const char* sc_dialogheads[] =
            {
                "baron",
                // "binklogo", // is this used for the boot flow icon?!
                "boy",
                "butler",
                "cook",
                "crone",
                "girl",
                "groundskeeper",
                "housekeeper",
                "hunchback",
                "krackpot",
                "mrribs",
                "mummy",
                "pirate",
                "piratecaptain",
                "skeletonbad",
                "zombiestandard",
            };

            constexpr const char* sc_eventicons[] =
            {
                "cross",
                "dontgethit",
                "findkey",
                "findkeyghouly",
                "ghouliegirl",
                "hauntedchair",
                "hauntedcoat",
                "haunteddoor",
                "hauntedtv",
                "hunchback",
                "imp",
                "impflying",
                "impninja",
                "jessieclyde",
                "killall",
                "killany",
                "limitedattacks",
                "lockintimer",
                "medusa",
                "minimumdamage",
                "mummy",
                "mummycursed",
                "nodamage",
                "onetype",
                "pirate",
                "piratecaptain",
                "skeleton",
                "spider",
                "survive",
                "tick",
                "timelimit",
                "unarmedonly",
                "vampirechicken",
                "vampirecoffin",
                "warlock",
                "weapononly",
                "zombie",
            };

            constexpr const char* sc_powerups[] =
            {
                "dizzyreaper",
                "ghoulyfreeze",
                "instantdeath",
                "invisibility",
                "invunerability",
                "knockdownmania",
                "minicooper",
                "nastyskull",
                "permanentweaponboost",
                "playerspeedup",
                "shockerblocker",
                "traitorfever",
            };
            
            template<size_t Size>
            bool doesStringExist(const string& source, const char* const(&stringList)[Size])
            {
                for (size_t i(0); i < Size; ++i) {
                    if (source.compare(stringList[i]) == 0) {
                        return true;
                    }
                }

                return false;
            }
        }

        bool Dialog::resolveHead(const wstring& source, string& result)
        {
            string headSource = base::stringutils::stringFromWide(source);

            if (doesStringExist(headSource, sc_dialogheads)) {
                result.clear();
                result.append("aid_texture_ghoulies_scoreboard_dialogheads_");
                result.append(headSource);

                return true;
            }

            return false;
        }

        bool Dialog::resolveEvent(const wstring& source, string& result)
        {
            string eventSource = base::stringutils::stringFromWide(source);

            if (doesStringExist(eventSource, sc_eventicons)) {
                result.clear();
                result.append("aid_texture_ghoulies_scoreboard_eventicons_");
                result.append(eventSource);

                return true;
            }

            return false;
        }

        bool Dialog::resolvePowerup(const wstring& source, string& result)
        {
            string powerupSource = base::stringutils::stringFromWide(source);

            if (doesStringExist(powerupSource, sc_powerups)) {
                result.clear();
                result.append("aid_texture_ghoulies_powerups_");
                result.append(powerupSource);

                return true;
            }

            return false;
        }

        bool Dialog::resolve(const wstring& source, string& result)
        {
            if (resolveHead(source, result)) {
                return true;
            }

            if (resolveEvent(source, result)) {
                return true;
            }

            if (resolvePowerup(source, result)) {
                return true;
            }

            return false;
        }
    }
}
