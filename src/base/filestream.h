#pragma once

#include "base/stream.h"
#include "base/memorystream.h"

#include <string_view>
#include <fstream>

namespace grabbed
{
    namespace base
    {
        class filestream : public stream
        {
            filestream() = delete;
            filestream &operator=(const filestream&) = delete;
            
        public:
            filestream(const std::string_view& filename);

            virtual ~filestream();
            void close();

            virtual bool isOpen() const override;
            virtual size_t getSize() const override { return m_size; }
            virtual size_t getPosition() const override { return m_position; }
            virtual void seek(size_t position) override;

            virtual void readImpl(void* buffer, size_t size) override;

            using stream::readAll;

            void readAll(memorystream& ms);

            const string& getPathName() const;
            const string& getFileName() const;

        private:
            string m_openPathName;
            string m_openFileName;
            size_t m_size{ 0 };
            size_t m_position{ 0 };
            std::ifstream m_file;
        };
    }
}
