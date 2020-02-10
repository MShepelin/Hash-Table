#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <stdexcept>

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

    list_type pairs_list_;
    table_type pairs_iterators_;

    const size_type kMinSize = 16;
    const double alpha_ = 2;
    size_type elements_stored_;
    size_type size_;

    void rehash() {
        size_ *= 2;
        table_type temp_pairs_pointers(size_, { pairs_list_.end() });

        for (iterator pair_iterator = pairs_list_.begin(); pair_iterator != pairs_list_.end(); pair_iterator++) {
            size_type key_hash = hasher_(pair_iterator->first);
            temp_pairs_pointers[key_hash % size_].push_back(pair_iterator);
        }

        pairs_iterators_ = temp_pairs_pointers;
    }

public:
    using iterator = typename list_type::iterator;
    using const_iterator = typename list_type::const_iterator;

    HashMap(Hash hasher = Hash()) : hasher_(hasher) {
        size_ = kMinSize;
        elements_stored_ = 0;
        pairs_list_ = list_type(0);
        pairs_iterators_ = table_type(size_, { pairs_list_.end() });
    }

    template<class ForwardIt>
    HashMap(ForwardIt first, ForwardIt last, Hash hasher = Hash()) : HashMap(hasher) {
        while (first != last) {
            insert(*first);
            first++;
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> init_list, Hash hasher = Hash()) : HashMap(hasher) {
        for (auto key_and_value : init_list) {
            insert(key_and_value);
        }
    }

    iterator find(KeyType key) {
        size_type key_hash = hasher_(key);
        for (auto pair_iterator : pairs_iterators_[key_hash % size_]) {
            if (pair_iterator != pairs_list_.end() && pair_iterator->first == key) {
                return pair_iterator;
            }
        }
         
        return pairs_list_.end();
    }

    const_iterator find(KeyType key) const {
        size_type key_hash = hasher_(key);
        for (auto pair_iterator : pairs_iterators_[key_hash % size_]) {
            if (pair_iterator != pairs_list_.end() && pair_iterator->first == key) {
                return pair_iterator;
            }
        }

        return pairs_list_.end();
    }

    void insert(std::pair<KeyType, ValueType> key_and_value) {
        if (find(key_and_value.first) != pairs_list_.end()) {
            return;
        }

        pairs_list_.push_front(key_and_value);
        iterator pair_iterator = pairs_list_.begin();
        size_type key_hash = hasher_(key_and_value.first);

        pairs_iterators_[key_hash % size_].push_back(pair_iterator);
        elements_stored_++;

        if (elements_stored_ > alpha_* size_) {
            rehash();
        }
    }

    void erase(KeyType key) {
        iterator pair_iterator = find(key);

        if (pair_iterator == pairs_list_.end()) {
            return;
        }

        elements_stored_--;
        size_type key_hash = hasher_(key);

        auto& list_of_iters = pairs_iterators_[key_hash % size_];
        for (auto iter_to_pair = list_of_iters.begin(); iter_to_pair != list_of_iters.end(); iter_to_pair++)
            if ((*iter_to_pair)->first == key) {
                list_of_iters.erase(iter_to_pair);
                pairs_list_.erase(pair_iterator);
                return;
            }
    }

    size_type size() const {
        return elements_stored_;
    }

    iterator begin() {
        return pairs_list_.begin();
    }

    const_iterator begin() const {
        return pairs_list_.begin();
    }

    iterator end() {
        return pairs_list_.end();
    }

    const_iterator end() const {
        return pairs_list_.end();
    }

    bool empty() const {
        return elements_stored_ == 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    ValueType& operator[] (KeyType key) {
        // insert function will insert new element only if it's key isn't used in
        // hash table for this momnet
        insert(std::make_pair(key, ValueType()));
        return find(key)->second;
    }

    const ValueType& at(KeyType key) const {
        const_iterator pair_iterator = find(key);
        if (pair_iterator == pairs_list_.end())
            throw std::out_of_range("");

        return pair_iterator->second;
    }

    void clear() {
        size_ = kMinSize;
        elements_stored_ = 0;
        pairs_list_ = list_type(0);
        pairs_iterators_ = table_type(size_, { pairs_list_.end() });
    }

    HashMap& operator= (HashMap hash_map) {
        clear();
        for (auto key_and_value : hash_map)
            insert(key_and_value);
        return *this;
    }
};