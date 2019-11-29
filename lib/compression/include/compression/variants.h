#pragma once

#include "compression/compressor.h"
#include "compression/huffman.h"
#include "compression/lzw.h"

namespace compression::variants
{

using LZWCompressor = compression::Compressor<compression::LZW>;
using HuffmanCoding = compression::Compressor<compression::Huffman>;

}  // namespace compression::variants
