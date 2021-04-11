#pragma once

#include "base/types.h"

#include <cstdint>
#include <type_traits>

namespace grabbed
{
    namespace base
    {
        namespace
        {
            //template<typename T> T byte_swap(T value);

            i8 byte_swap(i8 value) {
                return value;
            }

            u8 byte_swap(u8 value) {
                return value;
            }

            i16 byte_swap(i16 value) {
                return static_cast<i16>(_byteswap_ushort(static_cast<u16>(value)));
            }

            u16 byte_swap(u16 value) {
                return _byteswap_ushort(value);
            }

            wchar_t byte_swap(wchar_t value) {
                return static_cast<wchar_t>(_byteswap_ushort(static_cast<u16>(value)));
            }

            i32 byte_swap(i32 value) {
                return static_cast<i32>(_byteswap_ulong(static_cast<u32>(value)));
            }

            u32 byte_swap(u32 value) {
                return _byteswap_ulong(value);
            }

            f32 byte_swap(f32 value) {
                u32 temp = byte_swap(*reinterpret_cast<u32*>(&value));
                return *reinterpret_cast<f32*>(&temp);
            }

            template<typename T, std::enable_if_t<(std::is_enum<T>::value), int> = 0>
            T byte_swap(T value)
            {
                return static_cast<T>(byte_swap(static_cast<std::underlying_type_t<T>>(value)));
            }
        }

        template <typename T>
        struct endianBig {
            endianBig() = default;

            endianBig(const T& src)
                : value(byte_swap(src))
            { }

            endianBig(const endianBig& other)
                : value(other.value)
            { }

            operator T() const {
                return byte_swap(value);
            }

            T value{};
        };
        
        template <typename T>
        struct endianLittle {
            endianLittle() = default;

            endianLittle(const T& src)
                : value(src)
            { }

            endianLittle(const endianLittle& other)
                : value(other.value)
            { }

            operator T() const {
                return value;
            }

            T value{};
        };
    }
}
