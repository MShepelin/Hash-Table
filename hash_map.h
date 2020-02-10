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

    list_type list_of_pairs_;
    table_type table_;

    const size_type kMinSize = 16;
    const double alpha_ = 2;
    size_type elements_stored_;
    size_type size_;

    void rehash() {
        size_ *= 2;
        table_type temp_table(size_, { list_of_pairs_.end() });

        for (iterator list_iterator = list_of_pairs_.begin(); list_iterator != list_of_pairs_.end(); list_iterator++) {
            size_type key_hash = hasher_(list_iterator->first);
            temp_table[key_hash % size_].push_back(list_iterator);
        }

        table_ = temp_table;
    }

public:
    using iterator = typename list_type::iterator;
    using const_iterator = typename list_type::const_iterator;

    HashMap(Hash hasher = Hash()) : hasher_(hasher) {
        size_ = kMinSize;
        elements_stored_ = 0;
        list_of_pairs_= list_type(0);
        table_ = table_type(size_, { list_of_pairs_.end() });
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
        for (auto list_iterator : table_[key_hash % size_]) {
            if (list_iterator != list_of_pairs_.end() && list_iterator->first == key) {
                return list_iterator;
            }
        }
         
        return list_of_pairs_.end();
    }

    const_iterator find(KeyType key) const {
        size_type key_hash = hasher_(key);
        for (auto list_iterator : table_[key_hash % size_]) {
            if (list_iterator != list_of_pairs_.end() && list_iterator->first == key) {
                return list_iterator;
            }
        }

        return list_of_pairs_.end();
    }

    void insert(std::pair<KeyType, ValueType> key_and_value) {
        if (find(key_and_value.first) != list_of_pairs_.end()) {
            return;
        }

        list_of_pairs_.push_front(key_and_value);
        iterator list_iterator = list_of_pairs_.begin();
        size_type key_hash = hasher_(key_and_value.first);

        table_[key_hash % size_].push_back(list_iterator);
        elements_stored_++;

        if (elements_stored_ > alpha_* size_) {
            rehash();
        }
    }

    void erase(KeyType key) {
        iterator list_iterator = find(key);

        if (list_iterator == list_of_pairs_.end()) {
            return;
        }

        elements_stored_--;
        size_type key_hash = hasher_(key);

        auto& list_of_iters = table_[key_hash % size_];
        for (auto table_list_it = list_of_iters.begin(); table_list_it != list_of_iters.end(); table_list_it++)
            if ((*table_list_it)->first == key) {
                list_of_iters.erase(table_list_it);
                list_of_pairs_.erase(list_iterator);
                return;
            }
    }

    size_type size() const {
        return elements_stored_;
    }

    iterator begin() {
        return list_of_pairs_.begin();
    }

    const_iterator begin() const {
        return list_of_pairs_.begin();
    }

    iterator end() {
        return list_of_pairs_.end();
    }

    const_iterator end() const {
        return list_of_pairs_.end();
    }

    bool empty() const {
        return elements_stored_ == 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    ValueType& operator[] (KeyType key) {
        iterator found_pair = find(key);

        if (found_pair != list_of_pairs_.end()) {
            return found_pair->second;
        }

        list_of_pairs_.push_front(std::make_pair(key, ValueType()));
        iterator list_iterator = list_of_pairs_.begin();
        size_type key_hash = hasher_(key);

        table_[key_hash % size_].push_back(list_iterator);
        elements_stored_++;

        if (elements_stored_ > alpha_* size_) {
            rehash();
        }

        return list_iterator->second;
    }

    const ValueType& at(KeyType key) const {
        const_iterator list_iterator = find(key);
        if (list_iterator == list_of_pairs_.end())
            throw std::out_of_range("");

        return list_iterator->second;
    }

    void clear() {
        size_ = kMinSize;
        elements_stored_ = 0;
        list_of_pairs_= list_type(0);
        table_ = table_type(size_, { list_of_pairs_.end() });
    }

    HashMap& operator= (HashMap hash_map) {
        clear();
        for (auto key_and_value : hash_map)
            insert(key_and_value);
        return *this;
    }
};