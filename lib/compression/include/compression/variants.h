#pragma once

#include "compression/compressor.h"
#include "compression/huffman.h"
#include "compression/lzw.h"

namespace compression::variants
{

using LZWCompressor = compression::Compressor<compression::LZW<1>>;
using AdvancedLZWCompressor = compression::Compressor<compression::LZW<2>>;
using HuffmanCoding = compression::Compressor<compression::Huffman>;

}  // namespace compression::variants
