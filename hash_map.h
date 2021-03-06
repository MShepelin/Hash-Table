#pragma once
#include <algorithm>
#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>
#include <utility>

template<
  class KeyType,
  class ValueType,
  class Hash = std::hash<KeyType>>
  class HashMap {
  private:
    using size_type = size_t;
    using list_type = std::list<std::pair<const KeyType, ValueType>>;
    using table_type = std::vector<std::list<typename list_type::iterator>>;

    Hash hasher_;

    list_type list_of_pairs_;
    table_type table_;

    const size_type kMinSize = 16;
    const double kAlpha = 0.5;
    size_type elements_stored_;
    size_type slots_available_;

  public:
    using iterator = typename list_type::iterator;
    using const_iterator = typename list_type::const_iterator;

    iterator end() {
      return list_of_pairs_.end();
    }

    const_iterator end() const {
      return list_of_pairs_.end();
    }

    iterator begin() {
      return list_of_pairs_.begin();
    }

    const_iterator begin() const {
      return list_of_pairs_.begin();
    }

    iterator insert(std::pair<KeyType, ValueType> key_and_value) {
      iterator found_pair_iterator = find(key_and_value.first);
      if (found_pair_iterator != end()) {
        return found_pair_iterator;
      }

      list_of_pairs_.push_front(key_and_value);
      iterator list_iterator = begin();
      size_type key_hash = hasher_(key_and_value.first);

      table_[key_hash % slots_available_].push_back(list_iterator);
      elements_stored_++;

      if (elements_stored_ * kAlpha > slots_available_) {
        rehash();
      }

      return list_iterator;
    }

  private:
    void rehash() {
      slots_available_ *= 2;
      table_ = table_type(slots_available_);

      for (iterator iterator_to_key_and_value = begin();
        iterator_to_key_and_value != end();
        iterator_to_key_and_value++) {
        size_type key_hash = hasher_(iterator_to_key_and_value->first);
        table_[key_hash % slots_available_].push_back(
          iterator_to_key_and_value);
      }
    }

  public:
    explicit HashMap(Hash hasher = Hash()) : hasher_(hasher),
      list_of_pairs_(list_type(0)), elements_stored_(0),
      slots_available_(kMinSize) {
      table_ = table_type(slots_available_);
    }

    template<class ForwardIt>
    HashMap(ForwardIt first, ForwardIt last, Hash hasher = Hash()) :
      HashMap(hasher) {
      for (; first != last; first++) {
        insert(*first);
      }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> pairs_list,
      Hash hasher = Hash()) :
      HashMap(pairs_list.begin(), pairs_list.end(), hasher) {}

    iterator find(KeyType key) {
      size_type key_hash = hasher_(key);
      for (auto iterator_to_key_and_value : table_[key_hash % slots_available_]) {
        if (iterator_to_key_and_value != end() &&
          iterator_to_key_and_value->first == key) {
          return iterator_to_key_and_value;
        }
      }

      return list_of_pairs_.end();
    }

    const_iterator find(KeyType key) const {
      return const_cast<HashMap*>(this)->find(key);
    }

    void erase(KeyType key) {
      iterator list_iterator = find(key);

      if (list_iterator == end()) {
        return;
      }

      elements_stored_--;
      size_type key_hash = hasher_(key);

      auto& list_of_iters = table_[key_hash % slots_available_];
      for (auto table_list_it = list_of_iters.begin();
        table_list_it != list_of_iters.end();
        table_list_it++) {
        if ((*table_list_it)->first == key) {
          list_of_iters.erase(table_list_it);
          list_of_pairs_.erase(list_iterator);
          return;
        }
      }
    }

    size_type size() const {
      return elements_stored_;
    }

    bool empty() const {
      return elements_stored_ == 0;
    }

    Hash hash_function() const {
      return hasher_;
    }

    ValueType& operator[] (KeyType key) {
      iterator inserted_pair_iterator = insert(
        std::make_pair(key, ValueType()));
      return inserted_pair_iterator->second;
    }

    const ValueType& at(KeyType key) const {
      const_iterator list_iterator = find(key);
      if (list_iterator == end()) {
        throw std::out_of_range("");
      }

      return list_iterator->second;
    }

    void clear() {
      slots_available_ = kMinSize;
      elements_stored_ = 0;
      list_of_pairs_ = list_type(0);
      table_ = table_type(slots_available_);
    }

    HashMap& operator= (HashMap hash_map) {
      clear();
      for (auto key_and_value : hash_map) {
        insert(key_and_value);
      }
      return *this;
    }
};