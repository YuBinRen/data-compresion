#pragma once

#include "compression/commons.h"

namespace compression
{

template <class Algo>
class Compressor : protected Algo
{
public:
  static commons::ByteArray compress(const commons::ByteArray &raw)
  {
    return Algo::encode(raw);
  }

  static commons::ByteArray decompress(const commons::ByteArray &compressed)
  {
    return Algo::decode(compressed);
  }
};

}  // namespace compression
