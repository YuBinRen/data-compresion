#include "utils/bytes.h"

#include "gtest/gtest.h"

#include <cstdint>
#include <array>

namespace utils::bytes
{

TEST(BytesInterop, FromBytes)
{
  // test one byte in an STL array
  std::array<unsigned char, 1> bytes8 = {(unsigned char)0xAA};

  EXPECT_EQ(0xAA, (from_bytes<std::uint8_t>(bytes8)));
  EXPECT_EQ(0xAA, (from_bytes<std::uint16_t>(bytes8)));
  EXPECT_EQ(0xAA, (from_bytes<std::uint32_t>(bytes8)));
  EXPECT_EQ(0xAA, (from_bytes<std::size_t>(bytes8)));

  // test one byte in a built-in array
  unsigned char bytes8_a[]{0xAA};
  EXPECT_EQ(0xAA, (from_bytes<std::uint8_t>(bytes8_a)));
  EXPECT_EQ(0xAA, (from_bytes<std::uint16_t>(bytes8_a)));
  EXPECT_EQ(0xAA, (from_bytes<std::uint32_t>(bytes8_a)));
  EXPECT_EQ(0xAA, (from_bytes<std::size_t>(bytes8_a)));

  // test two bytes in an STL array
  std::array<unsigned char, 2> bytes16 = {(unsigned char)0x34, (unsigned char)0x12};
  EXPECT_EQ(0x1234, (from_bytes<std::uint16_t>(bytes16)));
  EXPECT_EQ(0x1234, (from_bytes<std::uint32_t>(bytes16)));
  EXPECT_EQ(0x1234, (from_bytes<std::size_t>(bytes16)));

  // test two bytes in an STL vector
  std::vector<std::uint8_t> bytes16_v{0x34, 0x12};
  EXPECT_EQ(0x1234, (from_bytes<std::uint16_t>(bytes16_v)));
  EXPECT_EQ(0x1234, (from_bytes<std::uint32_t>(bytes16_v)));
  EXPECT_EQ(0x1234, (from_bytes<std::size_t>(bytes16_v)));

  // test three bytes to a 32 bit integer in a built-in array
  unsigned char bytes32_a[]{0x56, 0x34, 0x12};
  EXPECT_EQ(0x123456, (from_bytes<std::uint32_t>(bytes32_a)));
  EXPECT_EQ(0x123456, (from_bytes<std::size_t>(bytes32_a)));

  // test 5 bytes to a 64-bit integer
  unsigned char bytes64_a[]{0x9A, 0x78, 0x56, 0x34, 0x12};
  EXPECT_EQ(0x123456789A, (from_bytes<std::int32_t>(bytes64_a)));
  EXPECT_EQ(0x123456789A, (from_bytes<std::int64_t>(bytes64_a)));
  EXPECT_EQ(0x123456789A, (from_bytes<std::size_t>(bytes64_a)));

  // test with an inline STL array
  EXPECT_EQ(0x1234, from_bytes<std::int32_t>(std::array<std::uint8_t, 2>{0x34, 0x12}));
}

}
