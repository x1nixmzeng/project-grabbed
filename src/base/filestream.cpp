#include "filestream.h"

#include "base/assert.h"

#include <filesystem>

namespace grabbed
{
    namespace base
    {
        namespace
        {
            string extractFileName(const string& pathname)
            {
                std::filesystem::path path{ pathname };
                return path.filename().generic_string();
            }
        }

        filestream::filestream(const string &filename)
            : m_size(0)
            , m_position(0)
        {
            m_openPathName = filename;
            m_openFileName = extractFileName(filename);
            
            m_file = std::ifstream(m_openPathName, std::ios::in | std::ios::binary);
            if (m_file) {
                m_file.seekg(0, m_file.end);
                m_size = static_cast<size_t>(m_file.tellg());
                m_file.seekg(0, m_file.beg);
            }
#if _DEBUG
            else {
                auto currentPath = std::filesystem::current_path().generic_string();
                assert_always("Failed to open file. Check current path is valid: %s", currentPath.c_str());
            }
#endif
        }

        filestream::~filestream()
        {
            if (m_file) {
                close();
            }
        }

        void filestream::close()
        {
            if (m_file) {
                m_file.close();
            }
            m_position = 0;
            m_size = 0;
        }

        bool filestream::isOpen() const
        {
            return m_file.is_open();
        }

        void filestream::seek(size_t position)
        {
            if (!isOpen()) {
                assert_always("error: file is not open");
                return;
            }

            if (position > getSize()) {
                assert_always("error: seek position out of bounds");
                return;
            }

            m_file.seekg(position, m_file.beg);
            m_position = position;
        }

        void filestream::readImpl(void* buffer, size_t size)
        {
            if (!isOpen()) {
                assert_always("error: file is not open");
                return;
            }

            if (!canRead(size)) {
                assert_always("error: unable to read this length");
                return;
            }
            
            m_file.read(reinterpret_cast<std::ifstream::char_type*>(buffer), size);
            m_stats.read(m_position, size);

            m_position += size;
        }

        void filestream::readAll(memorystream& ms)
        {
            if (!isOpen()) {
                assert_always("error: file is not open");
                return;
            }

            if (!ms.canRead(getSize())) {
                ms.resize(getSize());
            }
            
            readImpl(ms.getData(), getSize());
        }

        const string& filestream::getPathName() const
        {
            return m_openPathName;
        }

        const string& filestream::getFileName() const
        {
            return m_openFileName;
        }
    }

}

