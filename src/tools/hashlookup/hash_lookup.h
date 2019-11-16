#pragma once

#include "base/types.h"
#include "base/filestream.h"
#include "base/stringutils.h"

#include "hashutils.h"

#include "args.h"

namespace grabbed
{
    class hashlookup
    {
    public:
        void loadLooseGhoulies(const string& path);
        void loadLooseKameo(const string& path);
        void loadLoosePinata(const string& path);

        void outputStats();
        bool lookupString(const string& filename);

    protected:
        static bool lookupHash(const char* origin, u32 hash, const string& source, const hashutils::type& hashes);

        hashutils::type m_ghouliesHashes;
        hashutils::type m_kameoHashes;
        hashutils::type m_pinataHashes;
    };

    namespace tool
    {
        auto main(args::splittype& args) -> int
        {
            hashlookup hashInfo;
            
            {
                string path;

                if (args.read("ghoulies", path)) {
                    hashInfo.loadLooseGhoulies(path);
                }

                if (args.read("kameo", path)) {
                    hashInfo.loadLooseKameo(path);
                }

                if (args.read("pinata", path)) {
                    hashInfo.loadLoosePinata(path);
                }
            }
            
            hashInfo.outputStats();

            {
                string path;
                if (args.read("hash_file", path)) {

                    base::filestream fs(path);

                    if (fs.isOpen()) {
                        string wholeFile;
                        fs.readAll(wholeFile);

                        size_t offset{ 0 };
                        size_t length{ 0 };

                        while (base::stringutils::readLine(wholeFile, offset, length)) {
                            auto line{ wholeFile.substr(offset, length) };

                            hashInfo.lookupString(line);
                        }
                    }
                }
            }
            
            for (const auto& arg : args.loose) {
                hashInfo.lookupString(arg);
            }

            hashInfo.lookupString("aid_loctext_banjox_actor_hitdialogs");
            
            return 0;
        }
    }
}
