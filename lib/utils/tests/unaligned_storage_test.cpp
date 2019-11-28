#include "utils/unaligned_storage.h"

#include "gtest/gtest.h"
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

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

TEST(Reader, OneByte)
{
  std::vector<std::byte> stream{std::byte{0b00100101}};
  Reader reader{stream.begin()};

  EXPECT_EQ(reader.read<std::uint8_t>(3), 0b00000101);
  EXPECT_EQ(reader.read<std::uint8_t>(4), 0b00000100);
}

TEST(Reader, MoreBytes)
{
  std::vector<std::byte> a{std::byte{0b00100101}, std::byte{0b10001101}};
  Reader reader1{a.begin()};

  EXPECT_EQ(reader1.read<std::uint16_t>(12), (0b00001101'00100101));
  EXPECT_EQ(reader1.read<std::uint8_t>(4), 0b00001000);

  std::vector<std::uint8_t> b{0b10101101, 0b11001101};
  Reader reader2{b.cbegin()};

  EXPECT_EQ(reader2.read<std::uint8_t>(3), 0b00000101);
  EXPECT_EQ(reader2.read<std::uint16_t>(4), 0b0000101);
  EXPECT_EQ(reader2.read<std::size_t>(4), 0b00001011);
}

}  // namespace utils::unaligned_storage
