/**
 * @file ParsingErrors.h
 * @brief Container for parsing validation errors
 * @details Holds a collection of typed error messages from parsing operations
 *          Provides STL-compatible iteration and error querying
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk
{
    /**
     * @brief Container for parsing validation errors
     * @details Immutable collection of typed error messages from parsing operations
     *          Supports STL-compatible iteration and convenience methods for error checking
     */
    class ParsingErrors final
    {
    public:
        /**
         * @brief Single error entry with type and message
         */
        struct ErrorEntry
        {
            std::string type;    ///< Error type identifier
            std::string message; ///< Human-readable error message

            /**
             * @brief Equality comparison
             * @param other ErrorEntry to compare with
             * @return True if both type and message are equal
             */
            [[nodiscard]] bool operator==(const ErrorEntry& other) const noexcept
            {
                return type == other.type && message == other.message;
            }
        };

        /** @brief Const iterator over error entries */
        using Iterator = std::vector<ErrorEntry>::const_iterator;

        /**
         * @brief Constructs from a vector of error entries
         * @param errors Error entries to store (moved in)
         */
        inline explicit ParsingErrors(std::vector<ErrorEntry>&& errors) noexcept;

        ParsingErrors() = default;
        ParsingErrors(const ParsingErrors&) = default;
        ParsingErrors(ParsingErrors&&) noexcept = default;
        ~ParsingErrors() = default;

        ParsingErrors& operator=(const ParsingErrors&) = default;
        ParsingErrors& operator=(ParsingErrors&&) noexcept = default;

        /**
         * @brief Equality comparison
         * @param other ParsingErrors to compare with
         * @return True if both contain the same errors in the same order
         */
        [[nodiscard]] inline bool operator==(const ParsingErrors& other) const noexcept;

        /**
         * @brief Get number of errors
         * @return Error count
         */
        [[nodiscard]] inline size_t count() const noexcept;

        /**
         * @brief Check if any errors exist
         * @return True if the collection is non-empty
         */
        [[nodiscard]] inline bool hasErrors() const noexcept;

        /**
         * @brief Check if a specific error type exists
         * @param type Error type identifier to search for
         * @return True if at least one entry with that type is present
         */
        [[nodiscard]] inline bool hasErrorType(std::string_view type) const noexcept;

        /**
         * @brief Format errors as a human-readable string
         * @return Tab-indented error list, or "Success" if empty
         */
        [[nodiscard]] std::string toString() const;

        /**
         * @brief Get iterator to the first error entry
         * @details Iterates ErrorEntry values in the order errors were added
         * @return Const iterator to beginning
         */
        [[nodiscard]] inline Iterator begin() const noexcept;

        /** @brief Get iterator past the last error entry */
        [[nodiscard]] inline Iterator end() const noexcept;

    private:
        std::vector<ErrorEntry> m_errors;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/ParsingErrors.inl"
