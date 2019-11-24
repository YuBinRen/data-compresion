#include "compression/bytes.h"
#include "compression/variants.h"

#include <iostream>
#include <string>

int main(std::int32_t argc, char **argv)
{
  using compression::variants::LZWCompressor;

  std::string str{"sir sid eastman easily teases sea sick seals"};
  auto raw = compression::commons::to_byte_array(str);

  auto encoded = LZWCompressor::compress(raw);

  std::cout << "bytes_count = " << encoded.size() << "\n";

  for (std::size_t i = 1; i < encoded.size(); i += 2)
  {
    std::printf(
        "[0x%02x, 0x%02x]\n",
        std::to_integer<int>(encoded[i - 1]),
        std::to_integer<int>(encoded[i]));
  }

  return 0;
}
