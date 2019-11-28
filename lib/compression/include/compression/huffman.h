#pragma once

#include "utils/bytes.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>  // TODO Remove after completing.
#include <map>
#include <memory>
#include <queue>
#include <utility>

namespace compression
{

class Huffman
{
  using FreqByte = std::pair<std::byte, std::size_t>;
  using FreqMap = std::array<FreqByte, 256>;

  struct Node;
  using NodePtr = std::unique_ptr<Node>;

  struct Node
  {
    std::byte symbol;
    std::size_t freq;
    Node *parent;
    NodePtr left;
    NodePtr right;

    Node(std::byte symbol, std::size_t freq, Node *parent, NodePtr &&left, NodePtr &&right) :
        symbol{symbol}, freq{freq}, parent{parent}, left{std::move(left)}, right{std::move(right)}
    {
      left->parent = this;
      right->parent = this;
    }

    Node(std::byte symbol, std::size_t freq) : Node{symbol, freq, nullptr, nullptr, nullptr}
    {}
  };

  struct FreqTree
  {
    NodePtr root;
  };

protected:
  static utils::bytes::ByteSequence encode(const utils::bytes::ByteSequence &raw)
  {
    /*
     * This must be left intact to be written in the encoded output.
     */
    const auto freq = make_freq_map(raw);

    auto cmp = [](auto &&a, auto &&b) { return a->freq < b->freq; };  // TODO Sort by symbol if freq are equal.
    std::priority_queue<NodePtr, std::vector<NodePtr>, decltype(cmp)> pq(cmp);
    for (auto [b, f] : freq)
    {
      pq.push(std::make_unique<Node>(b, f));
    }

    while (pq.size() > 1)
    {
      pq.push(make_subtree(pq.pop(), pq.pop()));
    }

    auto symbols_tree = make_tree(pq.pop());

    return {};
  }

  static utils::bytes::ByteSequence decode(const utils::bytes::ByteSequence &encoded)
  {}

private:
  static FreqMap make_freq_map(const utils::bytes::ByteSequence &raw)
  {
    FreqMap freq;
    std::generate(
        freq.begin(), freq.end(), [b = 0]() mutable { return std::make_pair(std::byte{b++}, 0); });

    for (auto byte : raw)
    {
      freq[std::to_integer<std::size_t>(byte)].second++;
    }

    return freq;
  }

  static NodePtr make_subtree(NodePtr &&left, NodePtr &&right)
  {
    return std::make_unique<Node>(
        std::byte{0}, left->freq + right->freq, nullptr, std::move(left), std::move(right));
  }

  static FreqTree make_tree(NodePtr &&root)
  {
    return FreqTree{std::move(root)};
  }
};

}  // namespace compression
