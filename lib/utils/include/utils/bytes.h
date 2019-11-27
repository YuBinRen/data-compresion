  #pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <iterator>

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

  template <typename T>
  struct ByteDecoder
  {
    static constexpr std::size_t bytes_count = sizeof(T);

    template <class Container>
    T operator()(Container c) noexcept
    {
      static_assert(sizeof(typename Container::value_type) == 1);
      return impl(std::rbegin(c), std::rend(c));
    }

    template <typename U, std::size_t N>
    T operator()(const U (&c)[N]) noexcept
    {
      static_assert(sizeof(U) == 1);
      static_assert(std::size(c) <= bytes_count, "Bytes do not fit in the desired integral type.");

      return impl(std::rbegin(c), std::rend(c));
    }

  private:
    template <typename InputIt>
    T impl(InputIt rbegin, InputIt rend) noexcept
    {
      static_assert(sizeof(decltype(*rbegin)) == 1);
      static_assert(sizeof(decltype(*rend)) == 1);

      T res = 0;

      for (auto it = rbegin; it != rend; it++)
      {
        (res <<= 8) |= static_cast<unsigned char>(*it);
      }

      return res;
    }
  };

}  // namespace detail

template <typename T>
static typename detail::ByteEncoder<T>::FixedByteSequence to_bytes(T val)
{
  static_assert(std::is_integral_v<T>, "Integral value required.");

  return detail::ByteEncoder<T>{}(val);
}

template <typename T, class Container>
static T from_bytes(Container c)
{
  return detail::ByteDecoder<T>{}(c);
}

template <typename T, class U, std::size_t N>
static T from_bytes(const U (&array)[N])
{
  return detail::ByteDecoder<T>{}(array);
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

template <typename Integer>
std::size_t count_bits(Integer n)
{
  static_assert(std::is_integral_v<Integer>);

  std::size_t bits_count = 0;
  do
  {
    bits_count++;
  } while (n >>= 1);

  return bits_count;
}

}  // namespace utils::bytes
