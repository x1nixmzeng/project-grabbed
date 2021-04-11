#pragma once

#include "base/types.h"
#include "base/stream.h"
#include "base/memorystream.h"

namespace grabbed
{
    namespace base
    {
        class streamview : public stream
        {
            streamview() = delete;
        public:
            // Maps "size" onto the current base position
            streamview(stream& base, size_t size);
            streamview(streamview& other);

            bool isOpen() const override { return true; }

            size_t getSize() const override;
            size_t getPosition() const override;

            void seek(size_t position) override;
            void skip(size_t count) override;

            void read(size_t length, memorystream& ms);
            virtual void readImpl(void* buffer, size_t size) override;

            using stream::read;

        private:
            stream& m_src;
            size_t m_pos;
            size_t m_cursor;
            size_t m_size;
        };
    }
}
