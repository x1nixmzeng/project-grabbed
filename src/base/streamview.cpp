#include "streamview.h"

#include "base/assert.h"

namespace grabbed
{
    namespace base
    {
        streamview::streamview(stream& base, size_t size)
            : m_src(base)
            , m_pos(base.getPosition())
            , m_cursor(0)
            , m_size(size)
        {
            // "use up these bytes"
            m_src.skip(size);
        }

        streamview::streamview(streamview& other)
            : m_src(other.m_src)
            , m_pos(other.m_pos)
            , m_cursor(other.m_cursor)
            , m_size(other.m_size)
        { }

        size_t streamview::getSize() const
        {
            return m_size;
        }

        size_t streamview::getPosition() const
        {
            return m_cursor;
        }

        void streamview::seek(size_t position)
        {
            if (position < m_size)
            {
                m_cursor = position;
            }
        }

        void streamview::skip(size_t count)
        {
            if (m_cursor + count < m_size)
            {
                m_stats.read(m_cursor, count);

                m_cursor += count;
            }
        }

        void streamview::read(size_t length, memorystream& ms)
        {
            assert_true(ms.getSize() >= length, "buffer is not preallocated");
            if (ms.getSize() < length) {
                return;
            }

            readImpl(ms.getData(), length);
        }

        void streamview::readImpl(void* buffer, size_t size)
        {
            assert_true(canRead(size), "unable to read this size");

            if (!canRead(size)) {
                return;
            }

            auto oldPos = m_src.getPosition();

            m_stats.read(m_cursor, size);

            m_src.seek(m_pos + m_cursor);
            m_src.readImpl(buffer, size);

            skip(size);

            m_src.seek(oldPos);
        }
    }
}
