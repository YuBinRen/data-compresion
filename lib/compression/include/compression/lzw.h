#pragma once

#include "compression/seq.h"
#include "utils/bytes.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>  // FIXME Remove include
#include <iterator>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

namespace compression
{

static constexpr std::size_t ASCII_TABLE_SIZE = 256;

/**
 * \brief Dictionary implementation for the LZW compressor.
 *
 * Operations complexities:
 *  - put_sequence: O(m)
 *  - contains: O(1)
 *  - find: O(m)
 *  - at/operator[]: O(1)
 * where m := length(seq)
 */
class Dictionary
{
public:
  struct Node
  {
    Node *parent;
    std::size_t index;
    std::byte symbol;
    std::array<Node *, ASCII_TABLE_SIZE> children;

    Node(Node *parent, std::size_t index, std::byte symbol) :
        parent{parent}, index{index}, symbol{symbol}, children{}
    {
      children.fill(nullptr);
    }

    Node *add_child(std::size_t index, std::byte symbol)
    {
      auto n = std::make_unique<Node>(this, index, symbol);
      auto symbol_pos = std::to_integer<std::size_t>(symbol);

      if (children[symbol_pos])
      {
        return nullptr;
      }

      children[std::to_integer<std::size_t>(symbol)] = n.release();
      return children[symbol_pos];
    }

    Node *child_at(std::byte s) noexcept
    {
      return children[std::to_integer<std::size_t>(s)];
    }

    ~Node()
    {
      for (auto &node_ptr : children)
      {
        delete node_ptr;
      }
    }
  };

  Dictionary()
  {
    entries_.reserve(ASCII_TABLE_SIZE);
    for (std::size_t i = 0; i != ASCII_TABLE_SIZE; i++)
    {
      entries_.push_back(root_->add_child(size_++, std::byte{i}));
    }
  }

  /**
   * \brief Add a new sequence into the dictionary.
   * \param seq The byte sequence to be added
   * \returns The index of the last node that has been added.
   */
  std::size_t put_sequence(const utils::bytes::ByteSequence &seq) noexcept
  {
    Node *curr = root_.get();
    std::size_t front = 0;

    while (front < seq.size())
    {
      auto chr = seq[front++];
      if (!curr->child_at(chr))
      {
        entries_.push_back(curr->add_child(size_++, chr));
      }

      curr = curr->child_at(chr);
    }

    return curr->index;
  }

  Node *put_symbol(std::size_t parent_idx, std::byte symbol) noexcept
  {
    auto node_idx = size_++;
    Node *parent = entries_[parent_idx];
    Node *child = parent->add_child(node_idx, symbol);
    entries_.push_back(child);

    return child;
  }

  /**
   * \brief Checks whether a sequence with the given index exists in the dictionary.
   * \param index The index to be checked.
   */
  bool contains(std::size_t index) const noexcept
  {
    return size_ > index;
  }

  Node *find_node(const utils::bytes::ByteSequence &bytes) const noexcept
  {
    Node *curr = root_.get();
    std::size_t front = 0;

    while (front < bytes.size())
    {
      auto chr = bytes[front++];
      if (!curr->child_at(chr))
      {
        return nullptr;
      }

      curr = curr->child_at(chr);
    }

    return curr;
  }

  /**
   * \brief Find the index of the given sequence within the dictionary.
   * \param bytes The byte array to be searched for.
   * \return The index of the last node in the given sequence, or an empty optional otherwise.
   */
  std::optional<std::size_t> find(const utils::bytes::ByteSequence &bytes) const noexcept
  {
    auto node = find_node(bytes);
    if (node)
    {
      return node->index;
    }
    else
    {
      return std::nullopt;
    }
  }

  /**
   * \brief Get the bytes sequence resembled by the last node, found at \p index.
   *
   * Note that this function performs checked access which is slower than unchecked access. For
   * unchecked access, see the subscript operator.
   * \param index The index of the node representing the last symbol of the sequence.
   * \returns A byte sequence containing the found sequence, or an empty sequence if the sequence
   * does not exist.
   */
  utils::bytes::ByteSequence at(std::size_t index) const noexcept
  {
    if (!contains(index))
    {
      return {};
    }

    return operator[](index);
  }

  /**
   * \brief Performs unchecked access on the dictionary.
   * \see Dictionary::at
   */
  utils::bytes::ByteSequence operator[](std::size_t index) const noexcept
  {
    utils::bytes::ByteSequence bytes;  // TODO Find the average sequence length and reserve it here.
    std::stack<std::byte> rev;
    auto curr = entries_[index];

    while (curr->parent)
    {
      rev.push(curr->symbol);
      curr = curr->parent;
    }

    bytes.reserve(rev.size());
    while (!rev.empty())
    {
      bytes.push_back(rev.top());
      rev.pop();
    }

    return bytes;
  }

  const Node *entry(std::size_t idx) const noexcept
  {
    return entries_[idx];
  }

  std::size_t size() const noexcept
  {
    return size_;
  }

private:
  std::unique_ptr<Node> root_ = std::make_unique<Node>(nullptr, 0, std::byte{0});
  std::vector<Node *> entries_;
  std::size_t size_{0};
};

template <std::uint8_t compression_level = 1>
class LZW
{
protected:
  static utils::bytes::ByteSequence encode(const utils::bytes::ByteSequence &raw)
  {
    Dictionary dict;
    utils::bytes::ByteSequence temp;

    std::vector<std::size_t> dict_ptrs;
    dict_ptrs.reserve(raw.size());

    int pos_count = 0;

    Dictionary::Node *curr_node = nullptr;
    std::cout << "raw size = " << raw.size() << std::endl;

    for (std::size_t i = 0; i != raw.size(); i++)
    {
      std::byte x = raw[i];
      if (!curr_node)
      {
        curr_node = dict.find_node(temp);
      }

      utils::bytes::ByteSequence next_seq{seq::emplace_back_copy(temp, x)};

      if (!curr_node->child_at(x))  // We don't have the entry Ix in dictionary
      {
        dict.put_sequence(next_seq);
        auto p = *dict.find(temp);

        dict_ptrs.push_back(p);

        temp.clear();
        temp.emplace_back(x);
        curr_node = nullptr;

        continue;
      }

      temp.push_back(x);
      curr_node = curr_node->child_at(x);
    }

    /*
     * After the EOF condition is met, we need to make sure to create and emit the dictionary
     * pointer of the last sequence.
     */
    auto temp_ptr = dict.find(temp);
    if (!temp_ptr.has_value())
    {
      temp_ptr = dict.put_sequence(temp);
    }

    dict_ptrs.push_back(*temp_ptr);

    /*
     * We need to determine how many bits per dictionary pointer our archive requires. Currently,
     * this implementation will round this to the next byte. That is, dictionary pointers are not
     * going to be stored on split bytes.
     */
    auto ptr_bits_count = utils::bytes::count_bits(dict.size());
    auto ptr_size = ptr_bits_count / 8 + 1;

    std::cout << "ptr_size = " << ptr_size << "\n";

    utils::bytes::ByteSequence encoded{std::byte{ptr_size}};
    encoded.reserve(dict_ptrs.size() * sizeof(size_t));

    auto encoded_ptrs_count = utils::bytes::to_bytes(dict.size() - ASCII_TABLE_SIZE);
    std::copy_n(std::make_move_iterator(encoded_ptrs_count.cbegin()), ptr_size,
        std::back_inserter(encoded));

    std::cout << "ptrs_count = " << dict.size() - ASCII_TABLE_SIZE << "\n";

    for (std::size_t i = ASCII_TABLE_SIZE; i < dict.size(); i++)
    {
      auto node = dict.entry(i);
      auto encoded_parent_ptr = utils::bytes::to_bytes(node->parent->index);

      std::copy_n(std::make_move_iterator(encoded_parent_ptr.cbegin()), ptr_size,
          std::back_inserter(encoded));
      encoded.push_back(node->symbol);
    }

    for (auto p : dict_ptrs)
    {
      auto encoded_ptr = utils::bytes::to_bytes(p);
      std::copy_n(
          std::make_move_iterator(encoded_ptr.cbegin()), ptr_size, std::back_inserter(encoded));
    }

    return encoded;
  }

  static utils::bytes::ByteSequence decode(const utils::bytes::ByteSequence &encoded)
  {
    auto ptr_size = std::to_integer<std::size_t>(encoded.front());
    std::cout << "ptr_size = " << ptr_size << "\n";

    auto it = std::next(encoded.begin());

    std::vector<std::byte> b(it, std::next(it, ptr_size));
    auto ptrs_count = utils::bytes::from_bytes<std::size_t>(std::move(b));
    std::advance(it, ptr_size);

    std::cout << "found ptrs_count = " << ptrs_count << "\n";

    Dictionary dict;

    for (std::size_t i = 0; i < ptrs_count; i++)
    {
      std::vector<std::byte> encoded_parent_ptr(it, std::next(it, ptr_size));
      std::advance(it, ptr_size);

      auto parent_ptr = utils::bytes::from_bytes<std::size_t>(std::move(encoded_parent_ptr));
      auto symbol = *(it++);

      dict.put_symbol(parent_ptr, symbol);
    }

    std::cout << "dict.size() = " << dict.size() << "\n";

    utils::bytes::ByteSequence decompressed;
    // TODO How much to reserve?

    while (it != encoded.end())
    {
      std::vector<std::byte> encoded_ptr(it, std::next(it, ptr_size));
      std::advance(it, ptr_size);

      auto ptr = utils::bytes::from_bytes<std::size_t>(std::move(encoded_ptr));
      auto seq = dict[ptr];

      std::copy(std::make_move_iterator(seq.cbegin()), std::make_move_iterator(seq.cend()),
          std::back_inserter(decompressed));
    }

    return decompressed;
  }
};

}  // namespace compression
