/**
 * @file Concepts.h
 * @brief C++20 concepts for JSON type constraints
 * @details Defines the type traits and concepts used to constrain template parameters
 *          throughout the JSON components (Document, Builder, Parser).
 */

#pragma once

#include <concepts>
#include <string>
#include <type_traits>

namespace dnv::vista::sdk::json
{
    class Document;

    /**
     * @brief Trait to identify JSON container types (Object, Array, Document)
     * @details Specialized for Object, Array, and Document. Use is_json_container_v<T>
     *          for a convenient bool constant.
     */
    template <typename T>
    struct is_json_container : std::false_type
    {
    };

    template <>
    struct is_json_container<Document> : std::true_type
    {
    };

    /**
     * @brief Convenience variable template for is_json_container
     */
    template <typename T>
    inline constexpr bool is_json_container_v = is_json_container<std::decay_t<T>>::value;

    /**
     * @brief Concept matching primitive JSON value types
     * @details Matches std::string, char, bool, all integral types (except bool/char), and floating-point types.
     */
    template <typename T>
    concept Primitive = std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, char> ||
                        std::is_same_v<std::decay_t<T>, bool> ||
                        (std::is_integral_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool> &&
                         !std::is_same_v<std::decay_t<T>, char>) ||
                        std::is_floating_point_v<std::decay_t<T>>;

    /**
     * @brief Concept matching any JSON value (primitive or container)
     */
    template <typename T>
    concept Value = Primitive<T> || is_json_container_v<T>;

    /**
     * @brief Concept matching JSON container types
     */
    template <typename T>
    concept Container = is_json_container_v<T>;

    /**
     * @brief Concept for types that can be checked via isRoot<T>()
     * @details Matches primitives and non-Document containers (Object, Array).
     */
    template <typename T>
    concept Checkable = Primitive<T> || (is_json_container_v<T> && !std::is_same_v<std::decay_t<T>, Document>);
} // namespace dnv::vista::sdk::json
