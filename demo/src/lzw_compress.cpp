#include "compression/variants.h"
#include "utils/bytes.h"

#include <iostream>
#include <string>
#include <fstream>
#include <ios>
#include <filesystem>
#include <chrono>

int main(std::int32_t argc, char **argv)
{
  using compression::variants::LZWCompressor;

  if (argc < 3)
  {
    std::cerr << "usage: lzw_compress <source_file> <output_file>\n";
    return 1;
  }

  auto raw_file_size = std::filesystem::file_size(argv[1]);
  std::ifstream raw_file{argv[1], std::ios_base::in | std::ios_base::binary};

  utils::bytes::ByteSequence raw(raw_file_size);
  raw_file.read(static_cast<char *>((void *)raw.data()), raw_file_size);
  raw_file.close();

  auto t1 = std::chrono::high_resolution_clock::now();

  auto encoded = LZWCompressor::compress(raw);

  double compression_ratio = 1. * encoded.size() / raw_file_size * 100;

  std::cout << "Encoded size: " << encoded.size() << " bytes.\n";
  std::cout << "Compression ratio: " << compression_ratio << "%.\n";

  auto t2 = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> dur_s{t2 - t1};
  std::cout << "Took " << dur_s.count() << "s.\n";

  std::ofstream archived_file{argv[2], std::ios_base::out | std::ios_base::binary};
  archived_file.write((char *) encoded.data(), encoded.size());

  return 0;
}
