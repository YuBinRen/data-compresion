#include "compression/compressor.h"
#include "compression/lzw.h"
#include "utils/bytes.h"

#include "gtest/gtest.h"
#include <cstdint>
#include <iterator>
#include <string>

namespace compression
{

TEST(Dictionary, Construct)
{
  Dictionary dict;

  ASSERT_EQ(dict.size(), ASCII_TABLE_SIZE);

  for (std::size_t i = 0; i < dict.size(); i++)
  {
    SCOPED_TRACE(std::string{"i = "} + std::to_string(i));

    EXPECT_TRUE(dict.contains(i));
  }
}

TEST(Dictionary, PutSequence)
{
  Dictionary dict;

  std::string sequences[]{"foo", "bar", "foobaz", "foobar", "barbar"};

  for (auto it = std::begin(sequences); it != std::end(sequences); it++)
  {
    dict.put_sequence(utils::bytes::to_byte_array(*it));
  }

  for (auto it = std::begin(sequences); it != std::end(sequences); it++)
  {
    EXPECT_TRUE(dict.find(utils::bytes::to_byte_array(*it)));
  }
}

TEST(Dictionary, Find)
{
  Dictionary dict;

  std::string sequences[]{"foo", "bar", "foobaz", "foobar", "barbar"};

  for (auto it = std::begin(sequences); it != std::end(sequences); it++)
  {
    dict.put_sequence(utils::bytes::to_byte_array(*it));
  }

  for (auto it = std::begin(sequences); it != std::end(sequences); it++)
  {
    for (std::size_t i = 1; i < it->size(); i++)
    {
      EXPECT_TRUE(dict.find(utils::bytes::to_byte_array(it->substr(0, i))));
    }
  }
}

TEST(Dictionary, Subscript)
{
  Dictionary dict;

  std::string sequences[]{"foo", "bar", "foobaz", "foobar", "barbar"};

  std::map<std::size_t, std::string> positions;

  for (auto it = std::begin(sequences); it != std::end(sequences); it++)
  {
    auto pos = dict.put_sequence(utils::bytes::to_byte_array(*it));

    positions.emplace(pos, *it);
  }

  for (const auto &entry : positions)
  {
    EXPECT_EQ(dict[entry.first], utils::bytes::to_byte_array(entry.second));
  }
}

TEST(Dictionary, At)
{
  Dictionary dict;

  EXPECT_TRUE(dict.at(512).empty());
}

}  // namespace compression
