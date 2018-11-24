// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include "common/file.h"

#include "stream/StdInputStream.h"
#include "stream/StdOutputStream.h"
#include "serialization/BinaryInputStreamSerializer.h"
#include "serialization/BinaryOutputStreamSerializer.h"
#include "cryptonote/core/currency.h"

using namespace cryptonote;
template<class T> class BlockAccessor {
public:
  typedef T value_type;

  class const_iterator {
  public:
    typedef ptrdiff_t difference_type;
    typedef std::random_access_iterator_tag iterator_category;
    typedef const T* pointer;
    typedef const T& reference;
    typedef T value_type;

    const_iterator() {
    }

    const_iterator(BlockAccessor* swappedVector, size_t index) : m_swappedVector(swappedVector), m_index(index) {
    }

    bool operator!=(const const_iterator& other) const {
      return m_index != other.m_index;
    }

    bool operator<(const const_iterator& other) const {
      return m_index < other.m_index;
    }

    bool operator<=(const const_iterator& other) const {
      return m_index <= other.m_index;
    }

    bool operator==(const const_iterator& other) const {
      return m_index == other.m_index;
    }

    bool operator>(const const_iterator& other) const {
      return m_index > other.m_index;
    }

    bool operator>=(const const_iterator& other) const {
      return m_index >= other.m_index;
    }

    const_iterator& operator++() {
      ++m_index;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator i = *this;
      ++m_index;
      return i;
    }

    const_iterator& operator--() {
      --m_index;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator i = *this;
      --m_index;
      return i;
    }

    const_iterator& operator+=(difference_type n) {
      m_index += n;
      return *this;
    }

    const_iterator& operator-=(difference_type n) {
      m_index -= n;
      return *this;
    }

    const_iterator operator+(difference_type n) const {
      return const_iterator(m_swappedVector, m_index + n);
    }

    friend const_iterator operator+(difference_type n, const const_iterator& i) {
      return const_iterator(i.m_swappedVector, n + i.m_index);
    }

    difference_type operator-(const const_iterator& other) const {
      return m_index - other.m_index;
    }

    const_iterator& operator-(difference_type n) const {
      return const_iterator(m_swappedVector, m_index - n);
    }

    const T& operator*() const {
      return (*m_swappedVector)[m_index];
    }

    const T* operator->() const {
      return &(*m_swappedVector)[m_index];
    }

    const T& operator[](difference_type offset) const {
      return (*m_swappedVector)[m_index + offset];
    }

    size_t index() const {
      return m_index;
    }

  private:
    BlockAccessor* m_swappedVector;
    size_t m_index;
  };

  BlockAccessor(const Currency &currency);
  //BlockAccessor(const BlockAccessor&) = delete;
  ~BlockAccessor();
  //BlockAccessor& operator=(const BlockAccessor&) = delete;

  bool init();
  bool initIndex();
  void close();

  bool empty() const;
  uint64_t size() const;
  const_iterator begin();
  const_iterator end();
  const T& operator[](uint64_t index);
  const T& front();
  const T& back();
  void clear();
  void pop_back();
  void push_back(const T& item);
  bool readItems(std::fstream &fs, uint64_t&count);
  bool readHeight(std::fstream &fs, uint64_t&count);
  bool writeHeight(std::fstream &fs, uint64_t&count);
  void writeIndex(const char *data, size_t size, size_t offset, std::string message);

private:
  struct item_entry_t;
  struct cache_entry_t;

  struct item_entry_t {
  public:
    T item;
    typename std::list<cache_entry_t>::iterator cacheIter;
  };

  struct cache_entry_t {
  public:
    typename std::map<uint64_t, item_entry_t>::iterator itemIter;
  };

  std::fstream m_itemsFile;
  size_t m_poolSize;
  std::vector<uint64_t> m_offsets;
  const Currency &m_currency;
  uint64_t m_itemsFileSize;
  std::map<uint64_t, item_entry_t> m_items;
  std::list<cache_entry_t> m_cache;
  uint64_t m_cacheHits;
  uint64_t m_cacheMisses;

  T* prepare(uint64_t index);
};

template <class T>
BlockAccessor<T>::BlockAccessor(const Currency &currency) : m_currency(currency)
{
  m_poolSize = currency.getPoolSize();
}

template<class T> BlockAccessor<T>::~BlockAccessor() {
  close();
}

template<class T>

void BlockAccessor<T>::writeIndex(const char *data, size_t size, size_t offset, std::string message) {
  std::string file = m_currency.blockIndexesFileName();
  if(!std::file::write(file, data, size, offset)) {
    throw std::runtime_error(message);
  }
}

template<class T>
bool BlockAccessor<T>::readHeight(std::fstream &fs, uint64_t&count) {
  std::string blockIndexesFilename = m_currency.blockIndexesFileName();  
  fs.read(reinterpret_cast<char*>(&count), sizeof count);
  if (!fs) {
    return false;
  }
  return true;
}

template<class T>
bool BlockAccessor<T>::writeHeight(std::fstream &fs, uint64_t&count) {
  std::string blockIndexesFilename = m_currency.blockIndexesFileName();  
  if (!fs.write(reinterpret_cast<char*>(&count), sizeof count)) {
    return false;
  }
  return true;
}


template<class T>
bool BlockAccessor<T>::readItems(std::fstream &fs, uint64_t&count) {
  std::vector<uint64_t> offsets;
  uint64_t itemsFileSize = 0;
  for (uint64_t i = 0; i < count; ++i) {
  uint32_t itemSize;
  fs.read(reinterpret_cast<char*>(&itemSize), sizeof itemSize);
  if (!fs) {
    return false;
  }

  offsets.emplace_back(itemsFileSize);
    itemsFileSize += itemSize;
  }
  m_offsets.swap(offsets);
  m_itemsFileSize = itemsFileSize;
  return true;
}

template<class T>
bool BlockAccessor<T>::initIndex() {
  std::string blockIndexesFilename = m_currency.blockIndexesFileName();
  std::fstream fs;

  // fs.open(blockIndexesFilename, std::ios::in | std::ios::out | std::ios::binary);

  fs = std::file::open(blockIndexesFilename);
  if (fs) {
    uint64_t count;
    if (!readHeight(fs, count)) {
      return false;
    }

    if (!readItems(fs, count)) {
      return false;
    }
  } else {
    fs.open(blockIndexesFilename, std::ios::out | std::ios::binary);
    uint64_t count = 0;
    if (!writeHeight(fs, count)) {
      return false;
    }

    fs.close();
    m_offsets.clear();
    m_itemsFileSize = 0;
  }
  return true;
}

template <class T>
bool BlockAccessor<T>::init()
{
  if (!initIndex()) {
    return false;
  }
  std::string blockFilename = m_currency.blocksFileName();
  m_itemsFile = std::file::open(blockFilename, true);

  m_items.clear();
  m_cache.clear();
  m_cacheHits = 0;
  m_cacheMisses = 0;
  return true;
}

template<class T> void BlockAccessor<T>::close() {
  std::cout << "BlockAccessor cache hits: " << m_cacheHits << ", misses: " << m_cacheMisses << " (" << std::fixed << std::setprecision(2) << static_cast<double>(m_cacheMisses) / (m_cacheHits + m_cacheMisses) * 100 << "%)" << std::endl;
}

template<class T> bool BlockAccessor<T>::empty() const {
  return m_offsets.empty();
}

template<class T> uint64_t BlockAccessor<T>::size() const {
  return m_offsets.size();
}

template<class T> typename BlockAccessor<T>::const_iterator BlockAccessor<T>::begin() {
  return const_iterator(this, 0);
}

template<class T> typename BlockAccessor<T>::const_iterator BlockAccessor<T>::end() {
  return const_iterator(this, m_offsets.size());
}

template<class T> const T& BlockAccessor<T>::operator[](uint64_t index) {
  auto itemIter = m_items.find(index);
  if (itemIter != m_items.end()) {
    if (itemIter->second.cacheIter != --m_cache.end()) {
      m_cache.splice(m_cache.end(), m_cache, itemIter->second.cacheIter);
    }

    ++m_cacheHits;
    return itemIter->second.item;
  }

  if (index >= m_offsets.size()) {
    throw std::runtime_error("BlockAccessor::operator[]");
  }

  if (!m_itemsFile) {
    throw std::runtime_error("BlockAccessor::operator[]");
  }

  m_itemsFile.seekg(m_offsets[index]);
  T tempItem;
  
  Common::StdInputStream stream(m_itemsFile);
  cryptonote::BinaryInputStreamSerializer archive(stream);
  serialize(tempItem, archive);

  T* item = prepare(index);
  std::swap(tempItem, *item);
  ++m_cacheMisses;
  return *item;
}

template<class T> const T& BlockAccessor<T>::front() {
  return operator[](0);
}

template<class T> const T& BlockAccessor<T>::back() {
  return operator[](m_offsets.size() - 1);
}

template<class T> void BlockAccessor<T>::clear() {
  uint64_t count = 0;
  writeIndex(reinterpret_cast<char*>(&count), sizeof count, 0, "BlockAccessor::clear");
  m_offsets.clear();
  m_itemsFileSize = 0;
  m_items.clear();
  m_cache.clear();
}

template<class T> void BlockAccessor<T>::pop_back() {
  uint64_t count = m_offsets.size() - 1;
  writeIndex(reinterpret_cast<char*>(&count), sizeof count, 0, "BlockAccessor::pop_back");

  m_itemsFileSize = m_offsets.back();
  m_offsets.pop_back();
  auto itemIter = m_items.find(m_offsets.size());
  if (itemIter != m_items.end()) {
    m_cache.erase(itemIter->second.cacheIter);
    m_items.erase(itemIter);
  }
}

template<class T> void BlockAccessor<T>::push_back(const T& item) {
  uint64_t itemsFileSize;

  {
    if (!m_itemsFile) {
      throw std::runtime_error("BlockAccessor::push_back");
    }

    m_itemsFile.seekp(m_itemsFileSize);

    Common::StdOutputStream stream(m_itemsFile);
    cryptonote::BinaryOutputStreamSerializer archive(stream);
    serialize(const_cast<T&>(item), archive);

    itemsFileSize = m_itemsFile.tellp();
  }

  {
    uint32_t itemSize = static_cast<uint32_t>(itemsFileSize - m_itemsFileSize);
    size_t offset = sizeof(uint64_t) + sizeof(uint32_t) * m_offsets.size();
    writeIndex(reinterpret_cast<char*>(&itemSize), sizeof itemSize, offset, "BlockAccessor::push_back");
    uint64_t count = m_offsets.size() + 1;
    writeIndex(reinterpret_cast<char*>(&count), sizeof count, 0, "BlockAccessor::push_back");
  }

  m_offsets.push_back(m_itemsFileSize);
  m_itemsFileSize = itemsFileSize;

  T* newItem = prepare(m_offsets.size() - 1);
  *newItem = item;
}

template<class T> T* BlockAccessor<T>::prepare(uint64_t index) {
  if (m_items.size() == m_poolSize) {
    auto cacheIter = m_cache.begin();
    m_items.erase(cacheIter->itemIter);
    m_cache.erase(cacheIter);
  }

  auto itemIter = m_items.insert(std::make_pair(index, item_entry_t()));
  cache_entry_t cacheEntry = { itemIter.first };
  auto cacheIter = m_cache.insert(m_cache.end(), cacheEntry);
  itemIter.first->second.cacheIter = cacheIter;
  return &itemIter.first->second.item;
}
