/**
 * @file LocalIdParsingErrorBuilder.h
 * @brief Error builder for LocalId parsing validation
 * @details Accumulates validation errors during LocalId string parsing with specific error types
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
        /**
         * @enum LocalIdParsingState
         * @brief State codes for LocalId string parsing
         * @details Indicates the parsing state or validation error encountered during LocalId parsing
         */
        enum class LocalIdParsingState : std::uint8_t
        {
            NamingRule = 0,
            VisVersion,
            PrimaryItem,
            SecondaryItem,
            ItemDescription,
            MetaQuantity,
            MetaContent,
            MetaCalculation,
            MetaState,
            MetaCommand,
            MetaType,
            MetaPosition,
            MetaDetail,

            EmptyState = 100,
            Formatting = 101,
            Completeness = 102,

            NamingEntity = 200,
            IMONumber = 201
        };

        /**
         * @class LocalIdParsingErrorBuilder
         * @brief Builder for accumulating LocalId parsing errors
         * @details Collects validation errors with specific error types during LocalId string parsing.
         *          Use default constructor to instantiate, then addError() to accumulate errors,
         *          and build() to produce final ParsingErrors object.
         */
        class LocalIdParsingErrorBuilder final
        {
        public:
            LocalIdParsingErrorBuilder() = default;
            LocalIdParsingErrorBuilder(const LocalIdParsingErrorBuilder&) = default;
            LocalIdParsingErrorBuilder(LocalIdParsingErrorBuilder&&) noexcept = default;
            ~LocalIdParsingErrorBuilder() = default;

            LocalIdParsingErrorBuilder& operator=(const LocalIdParsingErrorBuilder&) = default;
            LocalIdParsingErrorBuilder& operator=(LocalIdParsingErrorBuilder&&) noexcept = default;

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
            LocalIdParsingErrorBuilder& addError(LocalIdParsingState state, std::string_view message);

        private:
            std::vector<std::pair<LocalIdParsingState, std::string>> m_errors;
        };
    } // namespace internal
} // namespace dnv::vista::sdk
