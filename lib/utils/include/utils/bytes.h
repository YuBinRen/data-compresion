#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace utils::bytes
{

namespace detail
{

  template <typename T>
  struct ByteEncoder
  {
    static constexpr std::size_t bytes_count = sizeof(T);
    using FixedByteSequence = std::array<std::byte, bytes_count>;

    FixedByteSequence operator()(T val)
    {
      FixedByteSequence bytes;

      for (std::size_t i = 0; i < bytes_count; i++)
      {
        bytes[i] = std::byte{(val >> (i << 3)) & 0xFF};
      }

      return bytes;
    }
  };

}  // namespace detail

template <typename T>
static typename detail::ByteEncoder<T>::FixedByteSequence to_bytes(T val)
{
  static_assert(std::is_integral_v<T>, "Integral value required.");

  return detail::ByteEncoder<T>{}(val);
}

using ByteSequence = std::vector<std::byte>;

template <class Container>
ByteSequence to_byte_array(const Container &c)
{
  static_assert(sizeof(typename Container::value_type) == 1, "Value type must fit in 1 byte.");

  ByteSequence bytes;
  bytes.reserve(c.size());

  for (auto it = c.cbegin(); it != c.cend(); ++it)
  {
    bytes.push_back(std::byte{*it});
  }

  return bytes;
}

}  // namespace utils::bytes
