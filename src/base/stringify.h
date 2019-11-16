#pragma once

#include "base/types.h"
#include "base/assert.h"

namespace grabbed
{
    namespace base
    {
        class stringify
        {
        public:
            stringify& append(u32 value) {
                return append(std::to_string(value));
            }

            stringify& append(i32 value) {
                return append(std::to_string(value));
            }

            stringify& append(u16 value) {
                return append(std::to_string(value));
            }

            stringify& append(i16 value) {
                return append(std::to_string(value));
            }

            stringify& append(u8 value) {
                return append(std::to_string(value));
            }

            stringify& append(i8 value) {
                return append(std::to_string(value));
            }

            stringify& append(f32 value) {
                return append(std::to_string(value));
            }

            stringify& append(const char* value) {
                m_buffer.append(value);
                return *this;
            }

            stringify& append(const string& value) {
                return append(value.c_str());
            }

            stringify& append(const wstring& value) {
                assert_always_once("append(wstring) is not implemented");
                return *this;
            }

            stringify& eol() {
                m_buffer.append("\r\n");
                return *this;
            }

            const size_t size() const {
                return m_buffer.size();
            }

            const string& operator()() const {
                return m_buffer;
            }

            const char* c_str() const {
                return m_buffer.c_str();
            }

        private:
            string m_buffer;
        };
    }
}
