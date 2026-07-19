/**
 * @file StringSet.h
 * @brief Mutable Robin Hood hash set with string keys and heterogeneous lookup
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Hashing.h"

namespace dnv::vista::sdk
{
    /**
     * @brief Mutable string set using Robin Hood hashing with heterogeneous lookup
     * @details Buckets are stored contiguously (std::vector), tracking each entry's
     *          displacement distance from its ideal slot to bound probe length
     *          Lookup accepts std::string, std::string_view, or const char* without
     *          allocating a temporary std::string
     */
    class StringSet final
    {
    private:
        struct Bucket
        {
            std::optional<std::string> key;
            uint32_t hash{};
            uint32_t distance{};
            bool occupied{};
        };

    public:
        using key_type = std::string;
        using value_type = std::string;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        /**
         * @brief Construct an empty set with default initial capacity
         */
        inline StringSet();

        /**
         * @brief Construct a set from an initializer list of keys
         * @param init Keys to insert
         */
        inline StringSet(std::initializer_list<std::string> init);

        StringSet(const StringSet&) = default;
        StringSet(StringSet&&) noexcept = default;
        ~StringSet() = default;

        StringSet& operator=(const StringSet&) = default;
        StringSet& operator=(StringSet&&) noexcept = default;

        /**
         * @brief Insert a key if not already present
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to insert
         * @return True if inserted, false if key already existed
         */
        template <typename K>
        inline bool insert(const K& key);

        /**
         * @brief Insert an owned string key, avoiding a copy
         * @param key Key to move in
         * @return True if inserted, false if key already existed
         */
        inline bool insert(std::string&& key);

        /**
         * @brief Check if the set contains the given key
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to search for
         * @return True if the key is present
         */
        template <typename K>
        [[nodiscard]] inline bool contains(const K& key) const noexcept;

        /**
         * @brief Find a key in the set
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to search for
         * @return Pointer to the stored string if found, nullptr otherwise
         */
        template <typename K>
        [[nodiscard]] inline const std::string* find(const K& key) const noexcept;

        /**
         * @brief Remove a key from the set if present
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to remove
         * @return True if the key was present and removed, false otherwise
         * @details Uses Robin Hood backward-shift deletion: after clearing the found slot,
         *          subsequent entries are shifted back one slot at a time (decrementing each
         *          shifted entry's distance) until an empty slot or an entry already at its
         *          ideal position (distance 0) is reached, preserving the invariant that no
         *          slot's distance ever exceeds what a fresh insert would compute
         */
        template <typename K>
        inline bool erase(const K& key);

        /**
         * @brief Reserve capacity for at least minCapacity entries
         * @param minCapacity Minimum number of entries to accommodate without rehashing
         */
        inline void reserve(size_t minCapacity);

        /**
         * @brief Return the number of entries
         * @return Current entry count
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Check if the set contains no entries
         * @return True if size is zero
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        class Iterator;
        using iterator = Iterator;
        using const_iterator = Iterator;

        /**
         * @brief Return an iterator to the first entry
         * @details `iterator`/`const_iterator` are both aliases of the same `Iterator` type,
         *          since StringSet exposes read-only iteration. Bucket order is unspecified
         *          (hash set), not insertion order
         * @return Iterator to the first non-empty bucket
         */
        [[nodiscard]] inline Iterator begin() const noexcept;

        /**
         * @brief Return an iterator past the last entry
         * @return Iterator to one past the last bucket
         */
        [[nodiscard]] inline Iterator end() const noexcept;

        /** @brief Same as begin(), provided for explicit-const-iteration style */
        [[nodiscard]] inline Iterator cbegin() const noexcept;

        /** @brief Same as end(), provided for explicit-const-iteration style */
        [[nodiscard]] inline Iterator cend() const noexcept;

        class Iterator final
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::string;
            using difference_type = std::ptrdiff_t;
            using pointer = const std::string*;
            using reference = const std::string&;

            inline Iterator(const std::vector<Bucket>* buckets, size_t index);

            inline reference operator*() const;
            inline pointer operator->() const;
            inline Iterator& operator++();
            inline Iterator operator++(int);
            inline bool operator==(const Iterator& other) const;

        private:
            inline void skipEmpty();

            const std::vector<Bucket>* m_buckets;
            size_t m_index;
        };

    private:
        static constexpr size_t INITIAL_CAPACITY = 16;
        static constexpr size_t MAX_LOAD_FACTOR_PERCENT = 75;

        std::vector<Bucket> m_buckets;
        size_t m_size = 0;
        size_t m_mask = INITIAL_CAPACITY - 1;

        inline bool shouldResize() const noexcept;
        inline void resize();
        inline bool insertHashed(std::string&& key, uint32_t hash);

        template <typename K>
        inline uint32_t hashOf(const K& key) const noexcept;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/containers/StringSet.inl"
