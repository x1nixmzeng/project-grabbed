#include "memorystream.h"

#include "base/assert.h"

namespace grabbed
{
    namespace base
    {
        memorystream::memorystream()
            : m_position(0)
        { }

        memorystream::memorystream(size_t initial_size)
            : m_position(0)
            , m_buffer(initial_size)
        { }

        memorystream::memorystream(const memorystream& other)
            : m_position(other.m_position)
            , m_buffer(other.m_buffer)
        { }

        size_t memorystream::getSize() const
        {
            return m_buffer.size();
        }

        size_t memorystream::getPosition() const
        {
            return m_position;
        }

        void memorystream::seek(size_t position)
        {
            m_position = position;
        }

        void memorystream::read(size_t length, memorystream& ms)
        {
            assert_true(&ms != this, "cannot read self");
            if (&ms == this) {
                return;
            }

            assert_true(ms.getSize() >= length, "buffer is not preallocated");
            if (ms.getSize() < length) {
                return;
            }

            readImpl(ms.getData(), length);
        }

        void memorystream::readImpl(void* buffer, size_t size)
        {
            assert_true(canRead(size), "unable to read this size");

            if (!canRead(size)) {
                return;
            }
            
            memcpy(buffer, &m_buffer[m_position], size);
            m_stats.read(m_position, size);

            // Update local size
            m_position += size;
        }

        void memorystream::write(size_t length, void *buffer)
        {
            assert_true(m_buffer.size() >= calculatePosition(length));

            memcpy(&m_buffer[m_position], buffer, length);
            m_position += length;
        }

        void memorystream::resize(size_t size)
        {
            m_buffer.resize(size);
        }

        u8* memorystream::getData()
        {
            return m_buffer.data() + m_position;
        }
    }

}

