/**
 * @file Document.h
 * @brief JSON Document DOM node
 * @details Provides the central DOM value type for the JSON components. A Document can hold
 *          any JSON value (null, boolean, integer, unsigned integer, double, string, array,
 *          or object) and exposes a unified API for construction, access, mutation, and
 *          serialization. The default-constructed Document is an empty JSON object.
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "Concepts.h"

#include <algorithm>
#include <compare>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace dnv::vista::sdk::json
{
    class Document;

    /**
     * @brief Discriminator for the type of value stored in a Document
     */
    enum class Type : uint8_t
    {
        Null,
        Boolean,
        Integer,
        UnsignedInteger,
        Double,
        String,
        Array,
        Object
    };

    /**
     * @brief JSON object type, ordered key/value pairs
     * @details Insertion order is preserved. Keys are @c std::string, values are Documents.
     */
    using Object = std::vector<std::pair<std::string, Document>>;

    /**
     * @brief JSON array type, ordered sequence of Documents
     */
    using Array = std::vector<Document>;

    template <>
    struct is_json_container<Object> : std::true_type
    {
    };

    template <>
    struct is_json_container<Array> : std::true_type
    {
    };

    /**
     * @brief Central JSON DOM node
     * @details Holds any JSON value. The default constructor produces an empty object.
     *          Use Document::object() / Document::array() as explicit factory alternatives.
     *          All mutating operations that require a specific type (e.g. set(), push_back())
     *          operate silently on the contained container. Behaviour is undefined if the
     *          Document holds a type that does not match the operation.
     */
    class Document final
    {
    public:
        /** @brief Constructs an empty JSON object (default) */
        DNV_VISTA_SDK_CPP_API Document();

        DNV_VISTA_SDK_CPP_API explicit Document(bool value) noexcept;         ///< Construct JSON boolean
        DNV_VISTA_SDK_CPP_API explicit Document(int value) noexcept;          ///< Construct JSON integer
        DNV_VISTA_SDK_CPP_API explicit Document(unsigned int value) noexcept; ///< Construct JSON unsigned integer
        DNV_VISTA_SDK_CPP_API explicit Document(int64_t value) noexcept;      ///< Construct JSON integer
        DNV_VISTA_SDK_CPP_API explicit Document(uint64_t value) noexcept;     ///< Construct JSON unsigned integer
        DNV_VISTA_SDK_CPP_API explicit Document(double value) noexcept;       ///< Construct JSON double
        DNV_VISTA_SDK_CPP_API explicit Document(std::string value);           ///< Construct JSON string
        DNV_VISTA_SDK_CPP_API explicit Document(std::string_view value);      ///< Construct JSON string
        DNV_VISTA_SDK_CPP_API explicit Document(const char* value);           ///< Construct JSON string

        template <size_t N>
        inline Document(const char (&value)[N])
            : m_data{ std::string{ value } }
        {}

        DNV_VISTA_SDK_CPP_API Document(std::nullptr_t) noexcept; ///< Construct JSON null
        DNV_VISTA_SDK_CPP_API explicit Document(Array value);    ///< Construct JSON array
        DNV_VISTA_SDK_CPP_API explicit Document(Object value);   ///< Construct JSON object

        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<
                std::is_same_v<T, long> && !std::is_same_v<long, int> && !std::is_same_v<long, int64_t>,
                int> = 0) noexcept
            : m_data{ static_cast<int64_t>(value) }
        {}

        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<
                std::is_same_v<T, unsigned long> && !std::is_same_v<unsigned long, unsigned int> &&
                    !std::is_same_v<unsigned long, uint64_t>,
                int> = 0) noexcept
            : m_data{ static_cast<uint64_t>(value) }
        {}

        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<std::is_same_v<T, long long> && !std::is_same_v<long long, int64_t>, int> = 0) noexcept
            : m_data{ static_cast<int64_t>(value) }
        {}

        template <typename T>
        inline Document(
            T value,
            std::enable_if_t<
                std::is_same_v<T, unsigned long long> && !std::is_same_v<unsigned long long, uint64_t>,
                int> = 0) noexcept
            : m_data{ static_cast<uint64_t>(value) }
        {}

        Document(const Document& other) = default;

        Document(Document&& other) noexcept = default;

        ~Document() = default;

        Document& operator=(const Document& other) = default;
        Document& operator=(Document&& other) noexcept = default;

        DNV_VISTA_SDK_CPP_API Document& operator=(std::string value);       ///< Assign string
        DNV_VISTA_SDK_CPP_API Document& operator=(bool value) noexcept;     ///< Assign bool
        DNV_VISTA_SDK_CPP_API Document& operator=(int64_t value) noexcept;  ///< Assign int64
        DNV_VISTA_SDK_CPP_API Document& operator=(uint64_t value) noexcept; ///< Assign uint64
        DNV_VISTA_SDK_CPP_API Document& operator=(double value) noexcept;   ///< Assign double
        DNV_VISTA_SDK_CPP_API Document& operator=(std::nullptr_t) noexcept; ///< Assign null

        DNV_VISTA_SDK_CPP_API bool operator==(const Document& other) const noexcept;
        DNV_VISTA_SDK_CPP_API std::strong_ordering operator<=>(const Document& other) const noexcept;

        /**
         * @brief Creates an empty JSON object
         * @return A Document holding an empty Object
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API Document object();

        /**
         * @brief Creates an empty JSON array
         * @return A Document holding an empty Array
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API Document array();

        /**
         * @brief Returns the type of the stored value
         * @return The Type enum value corresponding to the held alternative
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Type type() const noexcept;

        /**
         * @brief Returns the stored value as type T, if the type matches
         * @tparam T The expected value type
         * @return The value wrapped in std::optional, or std::nullopt if type mismatch
         */
        template <Value T>
        [[nodiscard]] inline std::optional<T> root() const
        {
            if (std::holds_alternative<T>(m_data))
            {
                return std::get<T>(m_data);
            }
            return std::nullopt;
        }

        /**
         * @brief Assigns the stored value to out if the type matches
         * @tparam T The expected value type
         * @param out Output parameter
         * @return true if the assignment succeeded, false on type mismatch
         */
        template <Value T>
        [[nodiscard]] inline bool root(T& out) const
        {
            auto val = root<T>();
            if (val)
            {
                out = std::move(*val);
                return true;
            }
            return false;
        }

        /**
         * @brief Returns a const reference wrapper to the stored value if the type matches
         * @tparam T The expected value type
         * @return Reference wrapper in std::optional, or std::nullopt if type mismatch
         */
        template <Value T>
        [[nodiscard]] inline std::optional<std::reference_wrapper<const T>> rootRef() const
        {
            if (std::holds_alternative<T>(m_data))
            {
                return std::cref(std::get<T>(m_data));
            }
            return std::nullopt;
        }

        /**
         * @brief Returns a mutable reference wrapper to the stored value if the type matches
         * @tparam T The expected value type
         * @return Reference wrapper in std::optional, or std::nullopt if type mismatch
         */
        template <Value T>
        [[nodiscard]] inline std::optional<std::reference_wrapper<T>> rootRef()
        {
            if (std::holds_alternative<T>(m_data))
            {
                return std::ref(std::get<T>(m_data));
            }
            return std::nullopt;
        }

        /**
         * @brief Checks whether the Document holds exactly type T
         * @tparam T The type to check
         * @return true if the stored alternative matches T
         */
        template <Checkable T>
        [[nodiscard]] inline bool isRoot() const
        {
            return std::holds_alternative<T>(m_data);
        }

        /**
         * @brief Applies a visitor to the stored value (const overload)
         * @tparam Visitor Callable compatible with all variant alternatives
         * @param visitor The visitor to invoke
         * @return The return value of the visitor call
         */
        template <typename Visitor>
        inline decltype(auto) visit(Visitor&& visitor) const
        {
            return std::visit(std::forward<Visitor>(visitor), m_data);
        }

        /**
         * @brief Applies a visitor to the stored value (mutable overload)
         * @tparam Visitor Callable compatible with all variant alternatives
         * @param visitor The visitor to invoke
         * @return The return value of the visitor call
         */
        template <typename Visitor>
        inline decltype(auto) visit(Visitor&& visitor)
        {
            return std::visit(std::forward<Visitor>(visitor), m_data);
        }

        /**
         * @brief Finds a value by key in a JSON object
         * @param key The key to look up
         * @return Pointer to the found Document, or nullptr if not found or not an object
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Document* find(std::string_view key) noexcept;

        /**
         * @brief Finds a value by key in a JSON object (const overload)
         * @param key The key to look up
         * @return Const pointer to the found Document, or nullptr if not found or not an object
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Document* find(std::string_view key) const noexcept;

        /**
         * @brief Inserts or replaces a key/value pair in a JSON object
         * @param key The key to insert or update
         * @param value The value to associate with the key
         */
        DNV_VISTA_SDK_CPP_API void set(std::string key, Document value);

        /**
         * @brief Checks whether a key exists in a JSON object
         * @param key The key to look up
         * @return true if the key exists, false otherwise
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool contains(std::string_view key) const noexcept;

        /**
         * @brief Returns a reference to the value for key, throws if missing
         * @param key The key to look up
         * @return Reference to the found Document
         * @throws std::out_of_range if the key does not exist or this is not an object
         */
        DNV_VISTA_SDK_CPP_API Document& at(std::string_view key);

        /**
         * @brief Returns a const reference to the value for key, throws if missing
         * @param key The key to look up
         * @return Const reference to the found Document
         * @throws std::out_of_range if the key does not exist or this is not an object
         */
        DNV_VISTA_SDK_CPP_API const Document& at(std::string_view key) const;

        /**
         * @brief Returns a reference to the element at index, throws if out of range
         * @param index Zero-based index
         * @return Reference to the element
         * @throws std::out_of_range if index is out of bounds or this is not an array
         */
        DNV_VISTA_SDK_CPP_API Document& at(size_t index);

        /**
         * @brief Returns a const reference to the element at index, throws if out of range
         * @param index Zero-based index
         * @return Const reference to the element
         * @throws std::out_of_range if index is out of bounds or this is not an array
         */
        DNV_VISTA_SDK_CPP_API const Document& at(size_t index) const;

        /**
         * @brief Returns a reference to the value for key, inserts null if missing
         * @param key The key to look up or insert
         * @return Reference to the existing or newly inserted Document
         */
        DNV_VISTA_SDK_CPP_API Document& operator[](std::string_view key);

        /**
         * @brief Returns a const reference to the value for key, returns a static null Document if missing
         * @param key The key to look up
         * @return Const reference to the found Document, or a static null Document
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Document& operator[](std::string_view key) const;

        /**
         * @brief Returns a reference to the element at index (no bounds check)
         * @param index Zero-based index
         * @return Reference to the element
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Document& operator[](size_t index);

        /**
         * @brief Returns a const reference to the element at index (no bounds check)
         * @param index Zero-based index
         * @return Const reference to the element
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Document& operator[](size_t index) const;

        /**
         * @brief Removes all elements from an object or array
         */
        DNV_VISTA_SDK_CPP_API void clear();

        /**
         * @brief Removes the key/value pair with the given key from an object
         * @param key The key to remove
         * @return 1 if the key was found and removed, 0 otherwise
         */
        DNV_VISTA_SDK_CPP_API size_t erase(std::string_view key);

        /** @brief Removes by C string key, delegates to erase(std::string_view)
         *  @param key The key to remove
         *  @return 1 if removed, 0 otherwise */
        DNV_VISTA_SDK_CPP_API size_t erase(const char* key);

        /** @brief Removes by std::string key, delegates to erase(std::string_view)
         *  @param key The key to remove
         *  @return 1 if removed, 0 otherwise */
        DNV_VISTA_SDK_CPP_API size_t erase(const std::string& key);

        /**
         * @brief Removes the element pointed to by an iterator
         * @tparam iterator Iterator type (must not be convertible to std::string_view)
         * @param it Iterator to the element to remove
         * @return Iterator to the element following the erased one
         */
        template <typename iterator, typename = std::enable_if_t<!std::is_convertible_v<iterator, std::string_view>>>
        inline iterator erase(iterator it)
        {
            if (type() == Type::Array)
            {
                return std::get<Array>(m_data).erase(it);
            }
            return it;
        }

        /**
         * @brief Inserts a Document before the given iterator position
         * @tparam iterator Iterator type
         * @param pos Position before which the element is inserted
         * @param value The Document to insert
         * @return Iterator pointing to the inserted element
         */
        template <typename iterator>
        inline iterator insert(iterator pos, Document value)
        {
            if (type() != Type::Array)
            {
                m_data = Array{};
            }
            return std::get<Array>(m_data).insert(pos, std::move(value));
        }

        /**
         * @brief Returns the number of elements in an array or object
         * @return Element count, or 0 if not a container
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API size_t size() const noexcept;

        /**
         * @brief Checks whether the array or object contains no elements
         * @return true if size() == 0
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isEmpty() const noexcept;

        /**
         * @brief Appends an element to a JSON array
         * @param value The Document to append
         */
        DNV_VISTA_SDK_CPP_API void push_back(Document value);

        /**
         * @brief Reserves storage in the underlying array or object vector
         * @param capacity The minimum capacity to reserve
         */
        DNV_VISTA_SDK_CPP_API void reserve(size_t capacity);

        /**
         * @brief Returns the current reserved capacity
         * @return Capacity of the underlying vector
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API size_t capacity() const noexcept;

        /**
         * @brief Returns a reference to the first element of a JSON array
         * @return Reference to the first element
         * @throws std::out_of_range if the array is empty
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Document& front();

        /**
         * @brief Const overload of front()
         * @return Const reference to the first element
         * @throws std::out_of_range if the array is empty
         * */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Document& front() const;

        /**
         * @brief Returns a reference to the last element of a JSON array
         * @return Reference to the last element
         * @throws std::out_of_range if the array is empty
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Document& back();

        /**
         *  @brief Const overload of back()
         *  @return Const reference to the last element
         *  @throws std::out_of_range if the array is empty
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Document& back() const;

        [[nodiscard]] auto begin();
        [[nodiscard]] auto end();
        [[nodiscard]] auto begin() const;
        [[nodiscard]] auto end() const;

        /**
         * @brief Forward iterator over the key/value pairs of a JSON object
         * @details Provides key() and value() accessors in addition to standard iterator operators.
         */
        class ObjectIterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::pair<const std::string, Document>;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_type*;
            using reference = const value_type&;
            using MapIterator = Object::const_iterator;

            MapIterator it; ///< Underlying iterator into the Object vector

            DNV_VISTA_SDK_CPP_API explicit ObjectIterator(MapIterator iter);

            ObjectIterator(const ObjectIterator&) = default;
            ObjectIterator(ObjectIterator&&) noexcept = default;
            ObjectIterator& operator=(const ObjectIterator&) = default;
            ObjectIterator& operator=(ObjectIterator&&) noexcept = default;

            DNV_VISTA_SDK_CPP_API const std::string& key() const; ///< Key of the current pair
            DNV_VISTA_SDK_CPP_API const Document& value() const;  ///< Value of the current pair (const)
            DNV_VISTA_SDK_CPP_API Document& value();              ///< Value of the current pair (mutable)

            DNV_VISTA_SDK_CPP_API const Document& operator*() const;
            DNV_VISTA_SDK_CPP_API Document& operator*();
            DNV_VISTA_SDK_CPP_API ObjectIterator& operator++();
            DNV_VISTA_SDK_CPP_API bool operator==(const ObjectIterator& other) const;
        };

        /**
         * @brief Returns an iterator to the first key/value pair of the object
         * @return ObjectIterator pointing to the first pair, or end() if not an object
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API ObjectIterator objectBegin() const;

        /**
         * @brief Returns an iterator past the last key/value pair of the object
         * @return ObjectIterator past the last pair
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API ObjectIterator objectEnd() const;

        /**
         * @brief Serializes the Document to a JSON string
         * @param indent Indentation width in spaces (0 = compact output)
         * @param bufferSize Initial buffer size hint (0 = use default)
         * @return The JSON string representation
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API std::string toString(int indent = 0, size_t bufferSize = 0) const;

        /**
         * @brief Parses a JSON string and returns a Document
         * @param jsonStr The JSON string to parse
         * @return The parsed Document, or std::nullopt on parse failure
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API std::optional<Document> fromString(std::string_view jsonStr);

        /**
         * @brief Parses a JSON string into an existing Document
         * @param jsonStr The JSON string to parse
         * @param value Output Document to populate
         * @return true on success, false on parse failure
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API bool fromString(std::string_view jsonStr, Document& value);

        /**
         * @brief Parses a UTF-8 byte vector as JSON and returns a Document
         * @param bytes The byte vector to parse
         * @return The parsed Document, or std::nullopt on parse failure
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API std::optional<Document> fromBytes(const std::vector<uint8_t>& bytes);

        /**
         * @brief Parses a UTF-8 byte vector into an existing Document
         * @param bytes The byte vector to parse
         * @param value Output Document to populate
         * @return true on success, false on parse failure
         */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API bool fromBytes(const std::vector<uint8_t>& bytes, Document& value);

    private:
        template <typename T>
        const T& rootInternal() const
        {
            return std::get<T>(m_data);
        }

        template <typename T>
        T& rootInternal()
        {
            return std::get<T>(m_data);
        }

        std::variant<std::nullptr_t, bool, int64_t, uint64_t, double, std::string, Array, Object> m_data;
    };
} // namespace dnv::vista::sdk::json
