/**
 * @file SerializableDocument.h
 * @brief Self-contained, JSON-library-agnostic document type for ISO 19848 extension points
 * @details ISO 19848 defines several `xs:any` extension points (CustomHeaders, CustomNameObjects,
 *          CustomElements, CustomProperties) that carry arbitrary, user-defined JSON content
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace dnv::vista::sdk::transport::serialization::json
{
    class SerializableDocument;

    /** @brief CPO tag: convert a user-provided JSON-like value into a SerializableDocument */
    struct ToSerializableDocumentTag final
    {
    };

    /** @brief CPO tag: convert a SerializableDocument into a user-provided JSON-like type */
    struct FromSerializableDocumentTag final
    {
    };

    /**
     * @brief Customization point object for adapting a user type to SerializableDocument
     * @details Users adapt their own JSON type `T` by providing an overload of
     *          `tag_invoke(dnv::vista::sdk::transport::serialization::ToSerializableDocumentTag, const T&)`
     *          discoverable via ADL in `T`'s namespace
     */
    inline constexpr struct ToSerializableDocumentFn final
    {
        template <typename T>
        [[nodiscard]] constexpr auto operator()(const T& value) const
            -> decltype(tag_invoke(ToSerializableDocumentTag{}, value))
        {
            return tag_invoke(ToSerializableDocumentTag{}, value);
        }
    } toSerializableDocument{};

    /**
     * @brief Customization point object for adapting SerializableDocument to a user type
     * @details Users adapt their own JSON type `T` by providing an overload of
     *          `tag_invoke(dnv::vista::sdk::transport::serialization::FromSerializableDocumentTag, const
     *          SerializableDocument&, T&)` discoverable via ADL in `T`'s namespace
     */
    inline constexpr struct FromSerializableDocumentFn final
    {
        template <typename T>
        constexpr auto operator()(const SerializableDocument& doc, T& out) const
            -> decltype(tag_invoke(FromSerializableDocumentTag{}, doc, out))
        {
            return tag_invoke(FromSerializableDocumentTag{}, doc, out);
        }
    } fromSerializableDocument{};

    /**
     * @brief Concept describing a type adaptable to/from SerializableDocument
     * @tparam T Candidate JSON-like type (e.g. a third-party JSON document type)
     */
    template <typename T>
    concept SerializableToDocument = requires(const T& value) { toSerializableDocument(value); };

    /**
     * @brief Concept describing a type that SerializableDocument content can be materialized into
     * @tparam T Candidate JSON-like type (e.g. a third-party JSON document type)
     * @details Requires T to be default-constructible in addition to being adaptable via
     *          `tag_invoke(FromSerializableDocumentTag, ...)`
     */
    template <typename T>
    concept SerializableFromDocument =
        std::is_default_constructible_v<T> &&
        requires(const SerializableDocument& doc, T& out) { fromSerializableDocument(doc, out); };

    /**
     * @brief Self-contained JSON value tree, independent of any specific JSON library
     * @details Represents arbitrary JSON content (null, boolean, integer, floating point, string,
     *          array, or object) for the ISO 19848 `xs:any` extension points. Insertion order of
     *          object keys is preserved to support lossless round-trip serialization
     */
    class SerializableDocument final
    {
    public:
        /** @brief Ordered key/value pairs of a JSON object */
        using Object = std::vector<std::pair<std::string, SerializableDocument>>;

        /** @brief Elements of a JSON array */
        using Array = std::vector<SerializableDocument>;

        /** @brief Discriminator for the value kind held by a SerializableDocument */
        enum class Kind : std::uint8_t
        {
            Null,
            Boolean,
            Integer,
            Double,
            String,
            Array,
            Object
        };

        /** @brief Constructs a JSON null */
        DNV_VISTA_SDK_CPP_API SerializableDocument() noexcept;

        /** @brief Constructs a JSON boolean
         *  @param value The boolean value */
        DNV_VISTA_SDK_CPP_API SerializableDocument(bool value) noexcept;

        /** @brief Constructs a JSON integer from any integral type
         *  @param value The integer value */
        DNV_VISTA_SDK_CPP_API SerializableDocument(std::int64_t value) noexcept;

        template <std::integral T>
            requires(!std::is_same_v<T, bool> && !std::is_same_v<T, std::int64_t>)
        SerializableDocument(T value) noexcept
            : SerializableDocument{ static_cast<std::int64_t>(value) }
        {}

        /** @brief Constructs a JSON floating point number
         *  @param value The double value */
        DNV_VISTA_SDK_CPP_API SerializableDocument(double value) noexcept;

        /** @brief Constructs a JSON string
         *  @param value The string value */
        DNV_VISTA_SDK_CPP_API SerializableDocument(std::string value);

        /** @brief Constructs a JSON string
         *  @param value The string value */
        DNV_VISTA_SDK_CPP_API SerializableDocument(std::string_view value);

        /** @brief Constructs a JSON string from any type with a toString() method (e.g. Decimal, DateTimeOffset) */
        template <typename T>
            requires requires(const T& v) {
                { v.toString() } -> std::convertible_to<std::string>;
            } && (!std::is_same_v<T, SerializableDocument>)
        SerializableDocument(const T& value)
            : SerializableDocument{ value.toString() }
        {}

        /** @brief Constructs a JSON array
         *  @param value The array elements */
        DNV_VISTA_SDK_CPP_API SerializableDocument(Array value);

        /** @brief Constructs a JSON object
         *  @param value The object key/value pairs, in insertion order */
        DNV_VISTA_SDK_CPP_API SerializableDocument(Object value);

        SerializableDocument(const SerializableDocument&) = default;
        SerializableDocument(SerializableDocument&&) noexcept = default;
        ~SerializableDocument() = default;

        SerializableDocument& operator=(const SerializableDocument&) = default;
        SerializableDocument& operator=(SerializableDocument&&) noexcept = default;

        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool operator==(const SerializableDocument& other) const noexcept;

        /** @brief Constructs an empty JSON object */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API SerializableDocument object() noexcept;

        /** @brief Constructs an empty JSON array */
        [[nodiscard]] static DNV_VISTA_SDK_CPP_API SerializableDocument array() noexcept;

        /**
         * @brief Constructs a SerializableDocument from a user-provided JSON-like value
         * @tparam T A type adapted via `tag_invoke(ToSerializableDocumentTag, const T&)`
         * @param value The value to convert
         * @return An equivalent SerializableDocument
         */
        template <SerializableToDocument T>
        [[nodiscard]] static SerializableDocument from(const T& value);

        /**
         * @brief Materializes this document into a user-provided JSON-like type
         * @tparam T A type adapted via `tag_invoke(FromSerializableDocumentTag, const
         * SerializableDocument&, T&)`
         * @return An equivalent instance of T
         */
        template <SerializableFromDocument T>
        [[nodiscard]] T to() const;

        /** @brief Gets the value kind held by this document */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Kind kind() const noexcept;

        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isNull() const noexcept;
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isBoolean() const noexcept;
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isInteger() const noexcept;
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isDouble() const noexcept;
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isString() const noexcept;
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isArray() const noexcept;
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool isObject() const noexcept;

        /** @brief Get boolean value. @throws std::bad_variant_access if not a boolean */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool asBoolean() const;

        /** @brief Get integer value. @throws std::bad_variant_access if not an integer */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API std::int64_t asInteger() const;

        /** @brief Get double value. @throws std::bad_variant_access if not a double */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API double asDouble() const;

        /** @brief Get string value. @throws std::bad_variant_access if not a string */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const std::string& asString() const;

        /** @brief Get array elements. @throws std::bad_variant_access if not an array */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Array& asArray() const;

        /** @brief Get object key/value pairs. @throws std::bad_variant_access if not an object */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const Object& asObject() const;

        /** @brief Get mutable array elements. @throws std::bad_variant_access if not an array */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Array& asArray();

        /** @brief Get mutable object key/value pairs. @throws std::bad_variant_access if not an object */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API Object& asObject();

        /**
         * @brief Looks up a key in this object
         * @param key The key to look up
         * @return Pointer to the value if this is an object and the key exists, nullptr otherwise
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API const SerializableDocument* find(std::string_view key) const noexcept;

        /**
         * @brief Looks up a key in this object, returning a mutable pointer
         * @param key The key to look up
         * @return Mutable pointer to the value if this is an object and the key exists, nullptr otherwise
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API SerializableDocument* find(std::string_view key) noexcept;

        /**
         * @brief Checks whether this object contains a key
         * @param key The key to look up
         * @return true if this is an object and contains key
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API bool contains(std::string_view key) const noexcept;

        /**
         * @brief Sets a key/value pair on this object, preserving insertion order
         * @details If this document is not yet an object, it is reset to an empty object first
         *          If key already exists, its value is replaced in place
         * @param key The key to set
         * @param value The value to associate with key
         */
        DNV_VISTA_SDK_CPP_API void set(std::string_view key, SerializableDocument value);

        /**
         * @brief Appends a value to this array
         * @details If this document is not yet an array, it is reset to an empty array first
         * @param value The value to append
         */
        DNV_VISTA_SDK_CPP_API void push_back(SerializableDocument value);

    private:
        std::variant<std::nullptr_t, bool, std::int64_t, double, std::string, Array, Object> m_value;
    };

    template <SerializableToDocument T>
    inline SerializableDocument SerializableDocument::from(const T& value)
    {
        return toSerializableDocument(value);
    }

    template <SerializableFromDocument T>
    inline T SerializableDocument::to() const
    {
        T out{};
        fromSerializableDocument(*this, out);
        return out;
    }
} // namespace dnv::vista::sdk::transport::serialization::json
