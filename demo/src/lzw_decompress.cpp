#include "compression/variants.h"
#include "utils/bytes.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

int main(std::int32_t argc, char **argv)
{
  using compression::variants::LZWCompressor;

  if (argc < 3)
  {
    std::cerr << "usage: lzw_decompress <archived_file> <output_file>\n";
    return 1;
  }

  auto archived_file_size = std::filesystem::file_size(argv[1]);
  std::ifstream archived_file{argv[1], std::ios_base::in | std::ios_base::binary};

  utils::bytes::ByteSequence archived(archived_file_size);
  archived_file.read(static_cast<char *>((void *)archived.data()), archived_file_size);
  archived_file.close();

  auto t1 = std::chrono::high_resolution_clock::now();

  auto decoded = LZWCompressor::decompress(archived);

  auto t2 = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> dur_s{t2 - t1};
  std::cout << "Took " << dur_s.count() << "s.\n";

  std::ofstream output_file{argv[2], std::ios_base::out | std::ios_base::binary};
  output_file.write((char *)decoded.data(), decoded.size());

  return 0;
}
