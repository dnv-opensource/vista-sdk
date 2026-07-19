/**
 * @file StackVector.h
 * @brief Small-buffer-optimized vector with stack storage and heap fallback
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace dnv::vista::sdk
{
    /**
     * @brief Vector with small buffer optimization for any copyable or movable type
     * @details Stores up to N elements inline on the stack (zero heap allocations)
     *          Once size exceeds N, storage transitions transparently to a heap
     *          std::vector<T>. Works with any type T, using if constexpr to select
     *          trivial memcpy paths when available
     * @tparam T Element type
     * @tparam N Maximum stack capacity before transitioning to heap storage
     */
    template <typename T, size_t N = 8>
    class StackVector final
    {
    public:
        using value_type = T;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        inline StackVector() noexcept;

        /** @brief Construct from initializer list, copying elements into the vector */
        inline StackVector(std::initializer_list<T> init);

        inline StackVector(const StackVector& other);
        inline StackVector(StackVector&& other) noexcept;

        inline ~StackVector() noexcept;

        inline StackVector& operator=(const StackVector& other);
        inline StackVector& operator=(StackVector&& other) noexcept;

        /**
         * @brief Equality comparison operator
         * @param other The StackVector to compare with
         * @return True if equal
         */
        inline bool operator==(const StackVector& other) const noexcept;

        /**
         * @brief Append an element to the end (copy)
         * @param value Element to append
         */
        inline void push_back(const T& value);

        /**
         * @brief Append an element to the end (move)
         * @param value Element to append
         */
        inline void push_back(T&& value);

        /**
         * @brief Construct element in-place at end
         * @tparam Args Types of arguments to forward to constructor
         * @param args Arguments to forward to element constructor
         */
        template <typename... Args>
        inline void emplace_back(Args&&... args);

        /**
         * @brief Remove the last element
         * @warning Undefined behavior if the vector is empty
         */
        inline void pop_back() noexcept;

        /**
         * @brief Remove all elements, retaining heap capacity if already on heap
         */
        inline void clear() noexcept;

        /**
         * @brief Resize the vector to count elements
         * @param count Target number of elements
         */
        inline void resize(size_type count);

        /**
         * @brief Resize the vector to count elements, initializing new elements with value
         * @param count Target number of elements
         * @param value Value to assign to new elements
         */
        inline void resize(size_type count, const T& value);

        /**
         * @brief Reserve capacity for at least newCapacity elements
         * @param newCapacity Minimum desired capacity
         */
        inline void reserve(size_type newCapacity);

        /**
         * @brief Access element at position pos without bounds checking
         * @param pos Zero-based index
         * @return Reference to the element
         */
        [[nodiscard]] inline reference operator[](size_type pos) noexcept;

        /**
         * @brief Access element at position pos without bounds checking (const)
         * @param pos Zero-based index
         * @return Const reference to the element
         */
        [[nodiscard]] inline const_reference operator[](size_type pos) const noexcept;

        /**
         * @brief Access element at position pos with bounds checking
         * @param pos Zero-based index
         * @return Reference to the element
         * @throws std::out_of_range if pos >= size()
         */
        [[nodiscard]] inline reference at(size_type pos);

        /**
         * @brief Access element at position pos with bounds checking (const)
         * @param pos Zero-based index
         * @return Const reference to the element
         * @throws std::out_of_range if pos >= size()
         */
        [[nodiscard]] inline const_reference at(size_type pos) const;

        /**
         * @brief Access the last element
         * @return Reference to the last element
         * @warning Undefined behavior if the vector is empty
         */
        [[nodiscard]] inline reference back() noexcept;

        /**
         * @brief Access the last element (const)
         * @return Const reference to the last element
         * @warning Undefined behavior if the vector is empty
         */
        [[nodiscard]] inline const_reference back() const noexcept;

        /**
         * @brief Access the first element
         * @return Reference to the first element
         * @warning Undefined behavior if the vector is empty
         */
        [[nodiscard]] inline reference front() noexcept;

        /**
         * @brief Access the first element (const)
         * @return Const reference to the first element
         * @warning Undefined behavior if the vector is empty
         */
        [[nodiscard]] inline const_reference front() const noexcept;

        /**
         * @brief Return the number of elements
         * @return Current element count
         */
        [[nodiscard]] inline size_type size() const noexcept;

        /**
         * @brief Check if the vector contains no elements
         * @return True if size is zero
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Return the current capacity
         * @return Number of elements that can be held without reallocation
         */
        [[nodiscard]] inline size_type capacity() const noexcept;

        /**
         * @brief Return pointer to the underlying data
         * @return Pointer to the first element
         */
        [[nodiscard]] inline pointer data() noexcept;

        /**
         * @brief Return const pointer to the underlying data
         * @return Const pointer to the first element
         */
        [[nodiscard]] inline const_pointer data() const noexcept;

        /**
         * @brief Return pointer to the first element
         * @details The returned pointer is a raw pointer into either the inline stack buffer or
         *          the heap-allocated storage. It is invalidated by any operation that grows the
         *          vector past its inline capacity (e.g. push_back triggering the stack->heap
         *          transition), same as std::vector's own iterator invalidation rules
         * @return Pointer to the first element
         */
        [[nodiscard]] inline pointer begin() noexcept;

        /** @brief Const overload of begin() */
        [[nodiscard]] inline const_pointer begin() const noexcept;

        /**
         * @brief Return pointer past the last element
         * @details Same invalidation rules as begin() apply
         * @return Pointer one past the last element
         */
        [[nodiscard]] inline pointer end() noexcept;

        /** @brief Const overload of end() */
        [[nodiscard]] inline const_pointer end() const noexcept;

        /** @brief Same as the const begin() overload, provided for explicit-const-iteration style */
        [[nodiscard]] inline const_pointer cbegin() const noexcept;

        /** @brief Same as the const end() overload, provided for explicit-const-iteration style */
        [[nodiscard]] inline const_pointer cend() const noexcept;

    private:
        union alignas(alignof(T) > alignof(std::vector<T>) ? alignof(T) : alignof(std::vector<T>))
        {
            unsigned char m_stackStorage[N * sizeof(T)];
            unsigned char m_heapStorage[sizeof(std::vector<T>)];
        };

        size_type m_size;
        bool m_usingStack;

        inline T* stackData() noexcept;
        inline const T* stackData() const noexcept;
        inline std::vector<T>& heapData() noexcept;
        inline const std::vector<T>& heapData() const noexcept;
        inline void transitionToHeap();
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/containers/StackVector.inl"
