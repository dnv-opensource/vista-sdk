/**
 * @file LocationParsingErrorBuilder.h
 * @brief Error builder for Location parsing validation
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk
{
    class ParsingErrors;

    namespace internal
    {
        /** @brief Result codes for Location string validation */
        enum class LocationValidationResult : std::uint8_t
        {
            Invalid = 0,      ///< Invalid format or structure
            InvalidCode,      ///< Invalid location code character
            InvalidOrder,     ///< Location codes not alphabetically sorted
            NullOrWhiteSpace, ///< Empty or whitespace-only string
            Valid             ///< Valid location string
        };

        /** @brief Accumulates Location parsing errors by validation result type */
        class LocationParsingErrorBuilder final
        {
        public:
            LocationParsingErrorBuilder() = default;
            LocationParsingErrorBuilder(const LocationParsingErrorBuilder&) = default;
            LocationParsingErrorBuilder(LocationParsingErrorBuilder&&) noexcept = default;
            ~LocationParsingErrorBuilder() = default;

            LocationParsingErrorBuilder& operator=(const LocationParsingErrorBuilder&) = default;
            LocationParsingErrorBuilder& operator=(LocationParsingErrorBuilder&&) noexcept = default;

            /**
             * @brief Check if any errors have been accumulated
             * @return True if errors exist, false otherwise
             */
            [[nodiscard]] bool hasError() const noexcept;

            /**
             * @brief Build the final ParsingErrors object
             * @return ParsingErrors containing all accumulated errors
             */
            [[nodiscard]] ParsingErrors build() const;

            /**
             * @brief Add a validation error
             * @param validationResult Type of validation error
             * @param message Error message description
             * @return Reference to this builder for chaining
             */
            LocationParsingErrorBuilder& addError(LocationValidationResult validationResult, std::string_view message);

        private:
            std::vector<std::pair<LocationValidationResult, std::string>> m_errors;
        };
    } // namespace internal
} // namespace dnv::vista::sdk
