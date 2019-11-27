#include "utils/unaligned_storage.h"

#include "gtest/gtest.h"

#include <vector>
#include <cstddef>
#include <cstdint>
#include <iterator>

namespace utils::unaligned_storage
{

TEST(Writer, UsingBool)
{
  std::vector<std::byte> stream;
  Writer unaligned_write{stream};

  /* preferred not to use std::vector<bool> specialization, so these uint8_t will be cast to bool */
  std::vector<std::uint8_t> bits{
      1, 0, 1, 0, 0, 1, 0, 0,  // first byte, in reverse order
      1, 1, 1, 0  // only the high part of the second byte, in reverse order
  };

  for (auto bit : bits)
  {
    unaligned_write(static_cast<bool>(bit));
  }

  EXPECT_EQ(stream.size(), 2);
  EXPECT_EQ(stream[0], std::byte{0b00100101});
  EXPECT_EQ(stream[1], std::byte{0b00000111});
}

}
