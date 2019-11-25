#pragma once

#include "utils/bytes.h"

namespace compression
{

template <class Algo>
class Compressor : protected Algo
{
public:
  static utils::bytes::ByteSequence compress(const utils::bytes::ByteSequence &raw)
  {
    return Algo::encode(raw);
  }

  static utils::bytes::ByteSequence decompress(const utils::bytes::ByteSequence &compressed)
  {
    return Algo::decode(compressed);
  }
};

}  // namespace compression
