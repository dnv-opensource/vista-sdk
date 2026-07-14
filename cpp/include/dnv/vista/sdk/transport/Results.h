/**
 * @file Results.h
 * @brief Common result types for validation operations
 */

#pragma once

#if defined(_MSC_VER)
    #define DNV_VISTA_SDK_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define DNV_VISTA_SDK_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace dnv::vista::sdk::transport
{
    /**
     * @brief Result of validation operations
     * @details Contains validation status and error messages if validation failed
     *          Template parameter T is optional value type for parsing operations
     * @tparam T Optional value type (void for simple validation, concrete type for parsing)
     */
    template <typename T = void>
    class ValidateResult final
    {
        using ValueType = std::conditional_t<std::is_void_v<T>, std::monostate, std::optional<T>>;

    private:
        /**
         * @brief Construct a validation result
         * @param isValid Validation status
         * @param errors Error messages if validation failed
         * @param value Parsed value if validation succeeded
         */
        ValidateResult(bool isValid, std::vector<std::string> errors, ValueType value)
            : m_valid{ isValid },
              m_errors{ std::move(errors) },
              m_value{ std::move(value) }
        {}

    public:
        /**
         * @brief Contextual conversion to bool, true if validation succeeded
         */
        [[nodiscard]] explicit operator bool() const noexcept { return m_valid; }

        /**
         * @brief Get error messages
         * @return Vector of error messages describing validation failures
         */
        [[nodiscard]] const std::vector<std::string>& errors() const noexcept { return m_errors; }

        /**
         * @brief Get parsed value
         * @return Optional containing the parsed value if validation succeeded
         */
        [[nodiscard]] const ValueType& value() const noexcept
            requires(!std::is_void_v<T>)
        {
            return m_value;
        }

        /**
         * @brief Create a successful validation result (simple validation)
         * @return ValidateResult indicating success
         */
        [[nodiscard]] static ValidateResult ok()
            requires std::is_void_v<T>
        {
            return ValidateResult{ true, {}, std::monostate{} };
        }

        /**
         * @brief Create a successful validation result with parsed value
         * @param val Parsed value
         * @return ValidateResult indicating success with value
         */
        template <typename U = T>
        [[nodiscard]] static ValidateResult ok(U&& val)
            requires(!std::is_void_v<T>)
        {
            return ValidateResult{ true, {}, std::optional<T>{ std::forward<U>(val) } };
        }

        /**
         * @brief Create a failed validation result with a single error message
         * @param error Error message describing the validation failure
         * @return ValidateResult indicating failure
         */
        [[nodiscard]] static ValidateResult invalid(std::string error)
        {
            return invalid(std::vector<std::string>{ std::move(error) });
        }

        /**
         * @brief Create a failed validation result
         * @param errors Vector of error messages describing validation failures
         * @return ValidateResult indicating failure
         */
        [[nodiscard]] static ValidateResult invalid(std::vector<std::string> errors)
        {
            if constexpr (std::is_void_v<T>)
            {
                return ValidateResult{ false, std::move(errors), std::monostate{} };
            }
            else
            {
                return ValidateResult{ false, std::move(errors), std::nullopt };
            }
        }

    private:
        bool m_valid;                                      ///< Validation status flag
        std::vector<std::string> m_errors;                 ///< Error messages if validation failed
        DNV_VISTA_SDK_NO_UNIQUE_ADDRESS ValueType m_value; ///< Parsed value if validation succeeded
    };

    /**
     * @brief Result of parsing operations
     * @details Contains parse status and optional value if parsing succeeded
     * @tparam T Type of parsed value
     */
    template <typename T>
    class ParseResult final
    {
    private:
        /**
         * @brief Construct a parse result
         * @param isValid Parse status
         * @param value Parsed value if parsing succeeded
         */
        ParseResult(bool isValid, std::optional<T> value)
            : m_valid{ isValid },
              m_value{ std::move(value) }
        {}

    public:
        /**
         * @brief Contextual conversion to bool, true if parsing succeeded
         */
        [[nodiscard]] explicit operator bool() const noexcept { return m_valid; }

        /**
         * @brief Get parsed value
         * @return Optional containing the parsed value if parsing succeeded
         */
        [[nodiscard]] const std::optional<T>& value() const noexcept { return m_value; }

        /**
         * @brief Create a successful parse result
         * @param val The parsed value
         * @return ParseResult indicating success
         */
        template <typename U = T>
        [[nodiscard]] static ParseResult ok(U&& val)
        {
            return ParseResult{ true, std::optional<T>{ std::forward<U>(val) } };
        }

        /**
         * @brief Create a failed parse result
         * @return ParseResult indicating failure
         */
        [[nodiscard]] static ParseResult invalid() { return ParseResult{ false, std::nullopt }; }

    private:
        bool m_valid;             ///< Parse status flag
        std::optional<T> m_value; ///< Parsed value if parsing succeeded
    };
} // namespace dnv::vista::sdk::transport
