#include <iterator>
#include <numeric>

namespace utils::numeric
{

template <class InputIt>
static auto gcd(InputIt first, InputIt last)
{
  decltype(*first) result = *first++;
  for (; first != last;)
  {
    result = std::gcd(*first++, result);
  }

  return result;
}

}  // namespace utils::numeric
