#include "stream.h"

#include "base/assert.h"

namespace grabbed
{
    namespace base
    {
        void stream::read(buffer& buffer, size_t length)
        {
            if (buffer.size() < length) {
                assert_always("error: buffer does not have enough space to read this length");
                return;
            }

            readImpl(&buffer[0], length);
        }

        void stream::readAll(buffer& buffer)
        {
            if (buffer.size() == 0) {
                assert_always("error: buffer has no size");
                return;
            }

            if (!canRead(buffer.size())) {
                assert_always("error: unable to read this length");
                return;
            }

            readImpl(&buffer[0], buffer.size());
        }

        void stream::readAll(string& string)
        {
            auto size{ getSize() - getPosition() };

            if (size == 0) {
                return;
            }

            string.resize(size);
            readImpl(const_cast<char*>(string.data()), string.size());
        }

        void stream::skip(size_t count)
        {
            if (!canRead(count)) {
                assert_always("error: unable to skip this length");
                return;
            }

            if (count > 0) {
                m_stats.read(getPosition(), count);
                seek(calculatePosition(count));
            }
        }

        void stream::align(size_t alignment)
        {
            auto p = getPosition();
            auto rem = p & (alignment - 1);
            if (rem != 0)
            {
                skip(alignment - rem);
            }
        }

        bool stream::canRead(size_t length) const
        {
            return calculatePosition(length) <= getSize();
        }

        size_t stream::calculatePosition(size_t offset) const
        {
            return getPosition() + offset;
        }

        string stream::readString(size_t length)
        {
            string result;

            if (!canRead(length * sizeof(u8))) {
                assert_always("error: unable to read this many characters");
                return result;
            }

            result.resize(length);
            readImpl(&result[0], length);

            // Strip away any string terminators
            auto invalid = result.find_first_of('\0', 0);
            if (invalid != result.npos) {
                return result.substr(0, invalid);
            }

            return result;
        }

        wstring stream::readWString(size_t length)
        {
            wstring result;

            if (!canRead(length * sizeof(u16))) {
                assert_always("error: unable to read this many characters");
                return result;
            }

            result.resize(length);
            readImpl(&result[0], length * 2);
            
            // Strip away any string terminators
            auto invalid = result.find_first_of(L'\0', 0);
            if (invalid != result.npos) {
                return result.substr(0, invalid);
            }

            return result;
        }

        string stream::readCString()
        {
            string result;
            result.reserve(128);

            u8 data(0);

            while (canRead(sizeof(data)))
            {
                data = read<u8>();
                if (data == 0)
                    break;
                result.push_back(static_cast<char>(data));
            }

            return result;
        }

        wstring stream::readWCString()
        {
            wstring result;
            result.reserve(128);

            u16 data(0);

            while (canRead(sizeof(data)))
            {
                data = read<u16>();
                if (data == 0)
                    break;

                result.push_back(static_cast<wchar_t>(data));
            }

            return result;
        }

        void stream::ensureFullyRead()
        {
            auto size = getSize();
            auto total = m_stats.getTotalRead();

            auto perc = (100.0f / static_cast<float>(total)) * static_cast<float>(size);
            assert_true(size == total, "entire file was not read. %u read of %u bytes (%.2f%%)", size, total, perc);
        }
    }
}
