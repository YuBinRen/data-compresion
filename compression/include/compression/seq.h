#pragma once

#include <utility>

namespace compression::seq
{

template <typename Container, class... TArgs>
Container emplace_back_copy(Container c, TArgs &&... args)
{
  c.emplace_back(std::forward<TArgs>(args)...);
  return std::move(c);
}

}  // namespace compression::seq
