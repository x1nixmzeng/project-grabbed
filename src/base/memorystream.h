#pragma once

#include "base/types.h"
#include "base/stream.h"

namespace grabbed
{
    namespace base
    {
        class memorystream : public stream
        {
        public:
            memorystream();
            memorystream(size_t initial_size);
            memorystream(const memorystream& other);

            virtual ~memorystream() {}

            virtual bool isOpen() const override { return true; }

            virtual size_t getSize() const override;
            virtual size_t getPosition() const override;

            virtual void seek(size_t position) override;

            void read(size_t length, memorystream& ms);
            virtual void readImpl(void* buffer, size_t size) override;

            void write(size_t length, void *buffer);

            void resize(size_t size);

            using stream::read;

            u8* getData();

        private:
            size_t m_position;
            buffer m_buffer;
        };
    }

}
