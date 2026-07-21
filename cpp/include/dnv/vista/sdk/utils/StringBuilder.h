/**
 * @file StringBuilder.h
 * @brief Growable character buffer for efficient string construction
 */

#pragma once

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    /**
     * @brief Growable character buffer with small buffer optimization
     * @details Uses a fixed-size stack buffer for small content, transparently
     *          transitioning to a heap allocation once that capacity is exceeded
     */
    class StringBuilder final
    {
    public:
        /**
         * @brief Construct an empty builder with the default stack buffer
         */
        inline StringBuilder() noexcept;

        /**
         * @brief Construct a builder with at least the given capacity reserved
         * @param initialCapacity Minimum capacity to reserve up front
         */
        inline explicit StringBuilder(size_t initialCapacity);

        inline StringBuilder(const StringBuilder& other);
        inline StringBuilder(StringBuilder&& other) noexcept;
        ~StringBuilder() = default;

        inline StringBuilder& operator=(const StringBuilder& other);
        inline StringBuilder& operator=(StringBuilder&& other) noexcept;

        /**
         * @brief Append a string
         * @param str String to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& operator+=(std::string_view str);

        /**
         * @brief Append a single character
         * @param c Character to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& operator+=(char c);

        /**
         * @brief Return the current content size
         * @return Current content size in bytes
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Return the current allocated capacity
         * @return Current allocated capacity in bytes
         */
        [[nodiscard]] inline size_t capacity() const noexcept;

        /**
         * @brief Check if the builder contains no content
         * @return True if no content has been appended
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Get the last character in the buffer
         * @return Reference to the last character
         * @warning Undefined behavior if the buffer is empty
         */
        [[nodiscard]] inline char& back() noexcept;

        /**
         * @brief Get the last character in the buffer
         * @return The last character
         * @warning Undefined behavior if the buffer is empty
         */
        [[nodiscard]] inline char back() const noexcept;

        /**
         * @brief Remove the last character from the buffer
         * @warning Undefined behavior if the buffer is empty
         */
        inline void pop_back() noexcept;

        /**
         * @brief Get a pointer to the buffer's content
         * @return Pointer to the first byte of buffer content, not null-terminated
         */
        [[nodiscard]] inline const char* data() const noexcept;

        /**
         * @brief View the buffer's current content
         * @return A string_view over the buffer's content
         */
        [[nodiscard]] inline std::string_view view() const noexcept;

        /**
         * @brief Materialize the buffer's content as an owning string
         * @return A copy of the buffer's content as std::string
         */
        [[nodiscard]] inline std::string toString() const;

        /**
         * @brief Reserve capacity for at least minCapacity bytes
         * @param minCapacity Minimum capacity to accommodate without reallocating
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& reserve(size_t minCapacity);

        /**
         * @brief Append a string
         * @param str String to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& append(std::string_view str);

        /**
         * @brief Append a single character
         * @param c Character to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& append(char c);

        /**
         * @brief Append the decimal representation of a signed integer
         * @param value Integer value to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& append(std::int64_t value);

        /**
         * @brief Append the decimal representation of an unsigned integer
         * @param value Integer value to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& append(std::uint64_t value);

        /**
         * @brief Append the shortest round-trippable decimal representation of a double
         * @param value Floating-point value to append
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& append(double value);

        /**
         * @brief Append the given character, repeated count times
         * @param c Character to repeat
         * @param count Number of times to append the character
         * @return Reference to this builder, for chaining
         */
        inline StringBuilder& appendRepeated(char c, size_t count);

        /**
         * @brief Discard all content
         * @param toStack If true, also releases any heap buffer and returns to the stack buffer
         *                if false (default), keeps the current buffer allocated
         */
        inline void clear(bool toStack = false) noexcept;

    private:
        static constexpr size_t STACK_BUFFER_SIZE = 256;
        // max_digits10 (17) + '-' + '.' + 'e+' + 3-digit exponent (e.g. "308") = 24, rounded to 25
        static constexpr size_t MAX_NUMERIC_CHARS = std::numeric_limits<double>::max_digits10 + 8;

        inline void ensureCapacity(size_t needed);

        std::unique_ptr<char[]> m_heapBuffer;
        char* m_buffer;
        char m_stackBuffer[STACK_BUFFER_SIZE];
        size_t m_size;
        size_t m_capacity;
    };

    inline std::ostream& operator<<(std::ostream& os, const StringBuilder& sb)
    {
        return os << sb.view();
    }
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/utils/StringBuilder.inl"
