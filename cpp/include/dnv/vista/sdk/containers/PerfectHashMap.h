/**
 * @file PerfectHashMap.h
 * @brief Immutable perfect hash map with string keys and heterogeneous lookup
 */

#pragma once

#include "Hashing.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace dnv::vista::sdk
{
    /**
     * @brief Immutable perfect hash map with string keys and heterogeneous lookup
     * @details Two-level perfect hashing: keys are bucketed by a first hash, then a per-bucket
     *          seed resolves collisions to unique positions. All keys must be known at construction
     *          time. O(1) worst-case lookup, no collisions, no rehashing. Lookup accepts
     *          @c std::string, @c std::string_view, or @c const @c char* without allocation
     * @tparam TValue Mapped value type
     */
    template <typename TValue>
    class PerfectHashMap final
    {
    public:
        using key_type = std::string;
        using mapped_type = TValue;
        using value_type = std::pair<std::string, TValue>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        class Iterator;
        using iterator = Iterator;
        using const_iterator = Iterator;

        /**
         * @brief Construct the map from a list of key/value pairs
         * @param items Key/value pairs to build the perfect hash table from
         * @throws std::invalid_argument if duplicate keys are detected
         */
        inline explicit PerfectHashMap(std::vector<std::pair<std::string, TValue>>&& items);

        PerfectHashMap() = default;
        PerfectHashMap(const PerfectHashMap&) = default;
        PerfectHashMap(PerfectHashMap&&) noexcept = default;
        ~PerfectHashMap() = default;

        PerfectHashMap& operator=(const PerfectHashMap&) = default;
        PerfectHashMap& operator=(PerfectHashMap&&) noexcept = default;

        /**
         * @brief Compare two maps for equality
         * @param other Map to compare against
         * @return True if both maps contain the same key/value pairs
         */
        [[nodiscard]] inline bool operator==(const PerfectHashMap& other) const noexcept;

        /**
         * @brief Access a value by key with bounds checking
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to search for
         * @return Const reference to the value
         * @throws std::out_of_range if key is not found
         */
        template <typename K>
        [[nodiscard]] inline const TValue& at(const K& key) const;

        /**
         * @brief Check if the map contains the given key
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to search for
         * @return True if the key is present
         */
        template <typename K>
        [[nodiscard]] inline bool contains(const K& key) const noexcept;

        /**
         * @brief Find a value by key
         * @tparam K std::string, std::string_view, or const char*
         * @param key Key to search for
         * @return Pointer to the value if found, nullptr otherwise
         */
        template <typename K>
        [[nodiscard]] inline const TValue* find(const K& key) const noexcept;

        /**
         * @brief Return the table capacity (>= count())
         * @return Number of slots in the internal table
         */
        [[nodiscard]] inline size_type size() const noexcept;

        /**
         * @brief Return the number of key/value pairs
         * @return Number of entries stored in the map
         */
        [[nodiscard]] inline size_type count() const noexcept;

        /**
         * @brief Check if the map contains no entries
         * @return True if count is zero
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Return an iterator to the first (key, value) pair
         * @details `iterator`/`const_iterator` are both aliases of the same `Iterator` type,
         *          since PerfectHashMap exposes read-only iteration. Slot order is fixed by
         *          the perfect hash, not insertion order
         * @return Iterator over (key, value) pairs, starting at the first occupied slot
         */
        [[nodiscard]] inline Iterator begin() const noexcept;

        /**
         * @brief Return an iterator past the last entry
         * @return Iterator to one past the last table slot
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
            using value_type = std::pair<std::string, TValue>;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_type*;
            using reference = const value_type&;

            inline Iterator(const std::vector<std::optional<std::pair<std::string, TValue>>>* table, size_t index);

            inline reference operator*() const;
            inline pointer operator->() const;
            inline Iterator& operator++();
            inline Iterator operator++(int);
            inline bool operator==(const Iterator& other) const;

        private:
            inline void skipEmpty();

            const std::vector<std::optional<std::pair<std::string, TValue>>>* m_table;
            size_t m_index;
        };

    private:
        size_t m_itemCount = 0;
        std::vector<std::optional<std::pair<std::string, TValue>>> m_table;
        std::vector<std::int32_t> m_seeds;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/containers/PerfectHashMap.inl"
