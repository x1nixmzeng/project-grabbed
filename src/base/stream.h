#pragma once

#include "base/types.h"
#include "base/streamstats.h"

namespace grabbed
{
    namespace base
    {
        class stream
        {
        private:
            stream(const stream&) = delete;
            stream &operator=(const stream&) = delete;

        protected:
            stream() = default;

        public:
            virtual ~stream() = default;

            // Check the stream can be read
            virtual bool isOpen() const = 0;

            // Return the full stream size
            virtual size_t getSize() const = 0;

            // Return the current stream position
            virtual size_t getPosition() const = 0;

            // Go to a specific position in the stream
            virtual void seek(size_t position) = 0;

            // Skip a number of bytes from the current stream position
            virtual void skip(size_t count);

            // Align the current offset with a power-of-2 alignment
            virtual void align(size_t alignment);

            // Check a number of bytes can be read from the current stream position
            bool canRead(size_t length) const;

            // Return the offset from the current stream position
            size_t calculatePosition(size_t offset) const;

            // Reads a fixed-length string from the current position
            string readString(size_t length);

            // Reads a fixed-length wide string from the current position
            wstring readWString(size_t length);

            // Reads a null-terminated string from the current position
            string readCString();

            // Reads a null-terminated wide string from the current position
            wstring readWCString();

            // Read a number of bytes to a preallocated buffer from the current position
            void read(buffer& buffer, size_t length);

            // Read a number of bytes to buffer pointer from the current position
            virtual void readImpl(void* buffer, size_t size) = 0;

            // Read the remaining stream into a preallocated buffer from the current position
            void readAll(buffer& buffer);

            // Read the entire stream as a string
            void readAll(string& string);

            // Read an instance of a specific type from the current position
            template<typename T>
            __forceinline size_t read(T& result)
            {
                constexpr size_t length = sizeof(T);
                readImpl(&result, length);
                return length;
            }

            // Catch invalid usage when passing in a vector
            template<typename T, typename A>
            __forceinline size_t read(std::vector<T, A>& result)
            {
                static_assert(false, "Invalid use of read. Use readAll for vector types.");
            }

            // Return an instance of a specific type from the current position
            template<typename T>
            __forceinline T read()
            {
                typename T result;
                read<T>(result);
                return result;
            }

            // Catch invalid usage when passing in a vector
            template<typename T, typename A>
            __forceinline std::vector<T, A> read()
            {
                static_assert(false, "Invalid use of read. Use readAll for vector types.");
            }

            // Return an known-length vector of instances of a specific type from the current position
            template<typename T>
            __forceinline void readAll(typename std::vector<T>& items)
            {
                if (items.size() > 0) {
                    readImpl(items.data(), items.size() * sizeof(T));
                }
            }

            // Return an known-length vector of instances of a specific type from the current position
            template<typename T>
            __forceinline typename std::vector<T> readArray(size_t count)
            {
                typename std::vector<T> result(count);
                readAll<T>(result);
                return result;
            }

            template<>
            __forceinline typename std::vector<string> readArray(size_t count)
            {
                std::vector<string> result(count);
                
                for (string& item : result) {
                    item = readCString();
                }

                return result;
            }

            // Check the current position of the stream is at the end of all available
            void ensureFullyRead();

        protected:
            streamstats m_stats;
        };
    }
}
