#pragma once

#include "utils/unaligned_storage.h"
#include "utils/bytes.h"
#include "utils/numeric.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>  // TODO Remove after completing.
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <bitset>
#include <stack>
#include <cmath>

namespace compression
{

class Huffman
{
  using FreqMap = std::map<std::byte, std::size_t>;
  using SymbolEncoding = std::pair<std::bitset<256>, std::uint8_t>;

  struct Node
  {
    std::byte symbol;
    std::size_t freq;
    Node *parent;
    Node *left;
    Node *right;

    Node(std::byte symbol, std::size_t freq, Node *left, Node *right) :
        symbol{symbol}, freq{freq}, left{left}, right{right}
    {
      if (left)
      {
        left->parent = this;
      }

      if (right)
      {
        right->parent = this;
      }
    }

    Node(std::byte symbol, std::size_t freq) : Node{symbol, freq, nullptr, nullptr}
    {}

    ~Node()
    {
      delete left;
      delete right;
    }

    bool is_leaf() const noexcept
    {
      return !left && !right;
    }

    SymbolEncoding encoding() const noexcept
    {
      std::stack<bool> path;
      auto curr = this;
      while (curr->parent)
      {
        /*
         * If this is the left child of its parent, then the path from the parent to the current
         * node is "1". If it is the right child, the path is "0". In other words, the edges of this
         * binary tree are each given a number: the left edge is always 1, and the right edge is
         * always 0.
         */
        path.push(curr == curr->parent->left);

        curr = curr->parent;
      }

      std::bitset<256> binary_path;
      std::uint8_t binary_path_size = 0;

      while (!path.empty())
      {
        binary_path.set(binary_path_size++, path.top());
        path.pop();
      }

      return std::make_pair(binary_path, binary_path_size);
    }
  };

  struct FreqTree
  {
    Node *root;

    std::map<std::byte, SymbolEncoding> to_table()
    {
      std::map<std::byte, SymbolEncoding> table;
      std::stack<Node *> stack;

      stack.push(root);

      while (!stack.empty())
      {
        auto curr = stack.top();
        stack.pop();

        if (curr->is_leaf())
        {
          table[curr->symbol] = curr->encoding();
        }
        else
        {
          stack.push(curr->right);
          stack.push(curr->left);
        }
      }

      return table;
    }

    ~FreqTree()
    {
      delete root;
    }
  };

protected:
  static utils::bytes::ByteSequence encode(const utils::bytes::ByteSequence &raw)
  {
    /*
     * This must be left intact to be written in the encoded output.
     */
    const auto freq_map = make_freq_map(raw);
    auto symbols_tree = make_tree(freq_map);
    auto symbol_codes = symbols_tree.to_table();

    for (const auto &[byte, freq] : freq_map)
    {
      auto chr = std::to_integer<std::size_t>(byte);
      auto [code, codelen] = symbol_codes[byte];
      auto codestr = code.to_string();
    }

    utils::bytes::ByteSequence output;
    output.reserve(raw.size());

    auto max_freq = std::max_element(freq_map.cbegin(), freq_map.cend(), [](auto &&a, auto &&b) {
      return a.second < b.second;
    });
    auto freq_bits_count = utils::bytes::count_bits(max_freq->second);
    std::uint8_t freq_bytes_count = std::ceil(freq_bits_count / 8.);
    // how many bytes the number of elements in the archive takes
    std::uint8_t elems_count_size = std::ceil(utils::bytes::count_bits(raw.size()) / 8.);

    /*
     * Control byte structure:
     * Mask 11110000 must be applied to obtain the size of archive counter.
     * Mask 00001111 must be applied to obtain the size of frequency values.
     */
    output.push_back(std::byte{freq_bytes_count | (elems_count_size << 4)});
    output.push_back(std::byte{symbol_codes.size()});  // how many symbols have been emitted.

    for (const auto &[byte, freq] : freq_map)
    {
      output.push_back(byte);

      auto fb = utils::bytes::to_bytes(freq);
      std::copy_n(std::make_move_iterator(fb.begin()), freq_bytes_count, std::back_inserter(output));
    }

    auto elems_count_bytes = utils::bytes::to_bytes(raw.size());
    std::copy_n(std::make_move_iterator(elems_count_bytes.begin()), elems_count_size, std::back_inserter(output));

    utils::unaligned_storage::Writer write_unaligned{output};

    for (auto byte : raw)
    {
      const auto &[coding, coding_size] = symbol_codes[byte];
      write_unaligned(coding, coding_size);
    }

    return output;
  }

  static utils::bytes::ByteSequence decode(const utils::bytes::ByteSequence &encoded)
  {
    auto front = encoded.begin();

    auto control = std::to_integer<std::uint8_t>(*front++);
    auto freq_size = control & 0xF;
    auto elems_count_size = (control & 0xF0) >> 4;

    auto table_size = std::to_integer<std::size_t>(*front++);

    FreqMap freq_map;
    for (std::size_t i = 0; i < table_size; i++)
    {
      std::byte symbol = *front++;
      utils::bytes::ByteSequence bf(front, std::next(front, freq_size));
      std::advance(front, freq_size);

      auto freq = utils::bytes::from_bytes<std::size_t>(std::move(bf));

      freq_map.emplace(symbol, freq);
    }

    utils::bytes::ByteSequence decompressed;

    auto symbols_tree = make_tree(freq_map);
    auto symbol_codes = symbols_tree.to_table();

    for (const auto &[byte, freq] : freq_map)
    {
      auto chr = std::to_integer<std::size_t>(byte);
      auto [code, codelen] = symbol_codes[byte];
      auto codestr = code.to_string();
    }

    auto elems_count_bytes = utils::bytes::ByteSequence(front, std::next(front, elems_count_size));
    auto elems_count = utils::bytes::from_bytes<std::size_t>(std::move(elems_count_bytes));
    std::advance(front, elems_count_size);

    auto curr = symbols_tree.root;

    utils::unaligned_storage::Reader reader{front};

    while (elems_count)
    {
      if (curr->is_leaf())
      {
        decompressed.push_back(curr->symbol);
        curr = symbols_tree.root;
        elems_count--;
      }

      bool bit = reader.read();
      if (bit == 1)  // the left edge must be picked
      {
        curr = curr->left;
      }
      else  // if 0, the right edge must be picked
      {
        curr = curr->right;
      }
    }

    return decompressed;
  }

private:
  static FreqMap make_freq_map(const utils::bytes::ByteSequence &raw)
  {
    FreqMap freq_map;

    for (auto byte : raw)
    {
      if (freq_map.find(byte) == freq_map.end())
      {
        freq_map.emplace(byte, 0);
      }

      freq_map[byte]++;
    }

    std::vector<std::size_t> frequencies(freq_map.size(), 0);
    std::transform(freq_map.begin(), freq_map.end(), frequencies.begin(), [](auto &&p) {
      return p.second;
    });

    std::size_t gcd = utils::numeric::gcd(frequencies.begin(), frequencies.end());

    for (auto &[byte, freq] : freq_map)
    {
      freq /= gcd;
    }

    return freq_map;
  }

  static Node *make_subtree(Node *left, Node *right)
  {
    return new Node(std::byte{0}, left->freq + right->freq, left, right);
  }

  static FreqTree make_tree(const FreqMap &freq_map)
  {
    auto cmp = [](auto &&a, auto &&b) {
      if (a->freq == b->freq)
      {
        return a->symbol < b->symbol;
      }

      return b->freq < a->freq;
    };

    std::priority_queue<Node *, std::vector<Node *>, decltype(cmp)> pq(cmp);
    for (auto [b, f] : freq_map)
    {
      pq.push(new Node(b, f));
    }

    while (pq.size() > 1)
    {
      auto left = pq.top();
      pq.pop();
      auto right = pq.top();
      pq.pop();

      pq.push(make_subtree(left, right));
    }

    auto root = pq.top();
    pq.pop();

    return FreqTree{root};
  }
};

}  // namespace compression
