#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace compression::commons
{

using ByteArray = std::vector<std::byte>;

template <class Container>
ByteArray to_byte_array(const Container &c)
{
  static_assert(sizeof(typename Container::value_type) == 1, "Value type must fit in 1 byte.");

  ByteArray bytes;
  bytes.reserve(c.size());

  for (auto it = c.cbegin(); it != c.cend(); ++it)
  {
    bytes.push_back(std::byte{*it});
  }

  return bytes;
}

}  // namespace compression::commons
