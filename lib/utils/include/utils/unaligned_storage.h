#pragma once

#include <cstdint>
#include <iterator>
#include <memory>

namespace utils::unaligned_storage
{

template <class Container>
class Writer
{
  using ValueType = typename Container::value_type;
  using PointerType = typename Container::pointer;

public:
  Writer(Container &c) : container_{std::addressof(c)}, val_{nullptr}, bit_idx_{0}
  {
    static_assert(sizeof(ValueType) == 1);
  }

  void operator()(bool bit)
  {
    if (bit_idx_ == 8 || !val_)
    {
      container_->push_back(ValueType(0));
      val_ = std::addressof(container_->back());
      bit_idx_ = 0;
    }

    *val_ |= ValueType(static_cast<std::uint8_t>(bit) << (bit_idx_++));
  }

private:
  Container *container_;
  PointerType val_;
  std::uint8_t bit_idx_;
};


class Reader
{

};


}
