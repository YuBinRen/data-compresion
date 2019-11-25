#pragma once

#include "compression/variants.h"

namespace compression
{

class Huffman
{
protected:
  static commons::ByteArray encode(const commons::ByteArray &raw)
  {}

  static commons::ByteArray decode(const commons::ByteArray &encoded)
  {}
};

}  // namespace compression
