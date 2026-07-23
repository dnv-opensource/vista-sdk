/**
 * @file Builder.h
 * @brief JSON builder (streaming, no DOM)
 * @details Provides a streaming JSON writer with a fluent API. The Builder does not
 *          construct an intermediate DOM, it writes directly into an internal string buffer.
 *          Use toString() to extract the result. The Builder can be reset and reused.
 *          SIMD acceleration is enabled automatically when SSE2 is available.
 */

#pragma once

#include "dnv/vista/sdk/utils/StringBuilder.h"

#include "Document.h"

#include <charconv>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace dnv::vista::sdk::json
{
    /**
     * @brief Streaming JSON writer with a fluent API
     * @details All write* and write() methods return *this to allow method chaining.
     *          The Builder tracks a context stack to automatically insert commas
     *          and colons at the right positions.
     */
    class Builder final
    {
    public:
        /**
         * @brief Construction options for the Builder
         */
        struct Options
        {
            int indent = 0;              ///< Indentation width in spaces (0 = compact)
            bool escapeNonAscii = false; ///< Escape all non-ASCII characters as \uXXXX
        };

        /**
         * @brief Constructs a Builder writing into the given external buffer
         * @param buffer External buffer to write into. The caller owns it and may reuse it
         *               across multiple Builder instances/documents to retain its capacity.
         * @param options Construction options (indent, escape mode)
         */
        explicit Builder(StringBuilder& buffer, Options options = { 0, false });

        /**
         * @brief Writes the opening brace of a JSON object
         * @return Reference to this for chaining
         */
        Builder& writeStartObject();

        /**
         * @brief Writes the closing brace of a JSON object
         * @return Reference to this for chaining
         */
        Builder& writeEndObject();

        /**
         * @brief Writes the opening bracket of a JSON array
         * @return Reference to this for chaining
         */
        Builder& writeStartArray();

        /**
         * @brief Writes the closing bracket of a JSON array
         * @return Reference to this for chaining
         */
        Builder& writeEndArray();

        /**
         * @brief Writes an object key followed by a colon
         * @details Must be called inside an open object context, before the corresponding value
         * @param key The key string
         * @return Reference to this for chaining
         */
        Builder& writeKey(std::string_view key);

        /**
         * @brief Writes an object key followed by a colon, without JSON-escaping it
         * @details Must be called inside an open object context, before the corresponding value.
         *          The caller must guarantee @p key needs no escaping (e.g. a fixed schema field
         *          name known at compile time).
         * @note Never use this for keys sourced from external or user-controlled data.
         * @param key The key string. Must not require JSON escaping
         * @return Reference to this for chaining
         */
        Builder& writeTrustedKey(std::string_view key);

        Builder& write(std::string_view key, std::nullptr_t value);     ///< Write @p key + null
        Builder& write(std::string_view key, bool value);               ///< Write @p key + bool
        Builder& write(std::string_view key, int value);                ///< Write @p key + int
        Builder& write(std::string_view key, unsigned int value);       ///< Write @p key + unsigned int
        Builder& write(std::string_view key, int64_t value);            ///< Write @p key + int64
        Builder& write(std::string_view key, uint64_t value);           ///< Write @p key + uint64
        Builder& write(std::string_view key, float value);              ///< Write @p key + float
        Builder& write(std::string_view key, double value);             ///< Write @p key + double
        Builder& write(std::string_view key, std::string_view value);   ///< Write @p key + string
        Builder& write(std::string_view key, const char* value);        ///< Write @p key + C string
        Builder& write(std::string_view key, const std::string& value); ///< Write @p key + string

        /**
         * @brief Serializes a DOM Document as the value for key
         * @param key Object key
         * @param value The Document to serialize
         * @return Reference to this
         */
        Builder& write(std::string_view key, const Document& value);

        Builder& write(std::nullptr_t value);     ///< Write null
        Builder& write(bool value);               ///< Write bool
        Builder& write(int value);                ///< Write int
        Builder& write(unsigned int value);       ///< Write unsigned int
        Builder& write(int64_t value);            ///< Write int64
        Builder& write(uint64_t value);           ///< Write uint64
        Builder& write(float value);              ///< Write float
        Builder& write(double value);             ///< Write double
        Builder& write(std::string_view value);   ///< Write string
        Builder& write(const char* value);        ///< Write C string
        Builder& write(const std::string& value); ///< Write string

        /**
         * @brief Serializes a DOM Document value
         * @param value The Document to serialize
         * @return Reference to this
         */
        Builder& write(const Document& value);

        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>,
            Builder&>
        write(std::string_view key, T value)
        {
            return write(key, static_cast<int64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
                !std::is_same_v<unsigned long, uint64_t>,
            Builder&>
        write(std::string_view key, T value)
        {
            return write(key, static_cast<uint64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, Builder&> write(
            std::string_view key, T value)
        {
            return write(key, static_cast<int64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>,
            Builder&>
        write(std::string_view key, T value)
        {
            return write(key, static_cast<uint64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>,
            Builder&>
        write(T value)
        {
            return write(static_cast<int64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
                !std::is_same_v<unsigned long, uint64_t>,
            Builder&>
        write(T value)
        {
            return write(static_cast<uint64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, Builder&> write(
            T value)
        {
            return write(static_cast<int64_t>(value));
        }

        template <typename T>
        inline std::enable_if_t<
            std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>,
            Builder&>
        write(T value)
        {
            return write(static_cast<uint64_t>(value));
        }

        /**
         * @brief Writes key + a JSON array serialized from a container
         * @tparam Container A range whose elements are writable by write()
         * @param key Object key
         * @param values The container to serialize as a JSON array
         * @return Reference to this
         */
        template <typename Container>
        inline Builder& writeArray(std::string_view key, const Container& values)
        {
            writeKey(key);
            return writeArray(values);
        }
        /**
         * @brief Writes a JSON array serialized from a container (value-only, for array context)
         * @tparam Container A range whose elements are writable by write()
         * @param values The container to serialize as a JSON array
         * @return Reference to this
         */
        template <typename Container>
        inline Builder& writeArray(const Container& values)
        {
            writeStartArray();
            for (const auto& value : values)
            {
                write(value);
            }
            return writeEndArray();
        }

        /**
         * @brief Extracts the built JSON string and resets the builder
         * @return The complete JSON string
         */
        std::string toString();

        /**
         * @brief Resets the builder to its initial empty state, preserving options
         * @return Reference to this
         */
        Builder& reset();

        /**
         * @brief Returns the current byte length of the buffer
         * @return Number of bytes written so far
         */
        [[nodiscard]] size_t size() const noexcept;

        /**
         * @brief Checks whether the buffer is empty
         * @return true if no bytes have been written
         */
        [[nodiscard]] bool isEmpty() const noexcept;

        /**
         * @brief Reserves buffer capacity to avoid reallocations
         * @param capacity Minimum capacity to reserve
         * @return Reference to this
         */
        Builder& reserve(size_t capacity);

        /**
         * @brief Returns the current allocated buffer capacity
         * @return Capacity in bytes
         */
        [[nodiscard]] size_t capacity() const noexcept;

        /**
         * @brief Checks whether the output is a valid, fully closed JSON document
         * @return true if all opened objects/arrays have been closed and the buffer is non-empty
         */
        [[nodiscard]] bool isValid() const noexcept;

    private:
        struct ContextFrame
        {
            bool isObject;       ///< true if this frame is an object, false if array
            bool empty;          ///< true before the first element has been written
            bool expectingValue; ///< true after writeKey(), awaiting the value
        };

        void writeKeyImpl(std::string_view key);
        void writeTrustedKeyImpl(std::string_view key);
        void writeInt(int64_t value);
        void writeUInt(uint64_t value);
        void writeDouble(double value);
        void writeString(std::string_view str);
        void writeStringRaw(std::string_view str);
        void writeNewlineAndIndent();
        void writeCommaIfNeeded();
        void writeDocument(const Document& doc);
        void writeDocumentArray(const Array& arr);
        void writeDocumentObject(const Object& obj);

        StringBuilder& m_buffer;                  ///< Output buffer, owned by the caller
        int m_indent;                             ///< Configured indentation width
        int m_currentIndent;                      ///< Current indentation depth in spaces
        bool m_escapeNonAscii;                    ///< Whether to escape non-ASCII as \uXXXX
        std::vector<ContextFrame> m_contextStack; ///< Stack of open container frames
    };
} // namespace dnv::vista::sdk::json
