#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace compression::bytes
{

namespace detail
{

  template <typename T>
  struct ByteEncoder
  {
    static constexpr std::size_t bytes_count = sizeof(T);
    using ByteSequence = std::array<std::byte, bytes_count>;

    ByteSequence operator()(T val)
    {
      ByteSequence bytes;

      for (std::size_t i = 0; i < bytes_count; i++)
      {
        bytes[i] = std::byte{(val >> (i << 3)) & 0xFF};
      }

      return bytes;
    }
  };

}  // namespace detail

template <typename T>
static typename detail::ByteEncoder<T>::ByteSequence to_bytes(T val)
{
  static_assert(std::is_integral_v<T>, "Integral value required.");

  return detail::ByteEncoder<T>{}(val);
}

}  // namespace compression::bytes
