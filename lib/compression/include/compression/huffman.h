#pragma once

#include "utils/bytes.h"

namespace compression
{

class Huffman
{
protected:
  static utils::bytes::ByteSequence encode(const utils::bytes::ByteSequence &raw)
  {}

  static utils::bytes::ByteSequence decode(const utils::bytes::ByteSequence &encoded)
  {}
};

}  // namespace compression
