#pragma once

#include <bitset>
#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>

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

  template <std::size_t N>
  void operator()(std::bitset<N> bitset, std::size_t bits_count = N)
  {
    for (std::size_t i = 0; i < bits_count; i++)
    {
      operator()(bitset.test(i));
    }
  }

private:
  Container *container_;
  PointerType val_;
  std::uint8_t bit_idx_;
};

template <class InputIt>
class Reader
{
public:
  Reader(InputIt it) : it_{it}, bit_idx_{0}
  {
    static_assert(sizeof(decltype(*it)) == 1);
  }

  bool read()
  {
    bool on = static_cast<std::uint8_t>(*it_) & (1 << bit_idx_++);

    if (bit_idx_ == 8)
    {
      std::advance(it_, 1);
      bit_idx_ = 0;
    }

    return on;
  }

  template <typename T>
  T read(std::size_t bits_count)
  {
    static_assert(std::is_integral_v<T>);

    T val = T(0);
    for (std::size_t i = 0; i < bits_count; i++)
    {
      val |= (read() << i);
    }

    return val;
  }

  InputIt begin() const
  {
    return it_;
  }

private:
  InputIt it_;
  std::uint8_t bit_idx_;
};

}  // namespace utils::unaligned_storage
