/**
 * @file UniversalIdBuilder.h
 * @brief Builder pattern for constructing VIS UniversalId instances
 * @details Provides a fluent API for building UniversalId objects combining IMO numbers with LocalIds
 *          UniversalId represents a globally unique sensor identifier across all vessels
 *          Format:
 * data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet
 */

#pragma once

#include "ImoNumber.h"
#include "LocalIdBuilder.h"

#include <optional>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    class UniversalId;
    class ParsingErrors;

    /**
     * @class UniversalIdBuilder
     * @brief Builder for constructing VIS UniversalId instances with fluent API
     * @details Provides immutable builder pattern for creating UniversalId objects with validation
     *          Each with* method returns a new builder instance with the modified value,
     *          preserving immutability. A valid UniversalId requires both an IMO number and a valid LocalIdBuilder
     *          Use build() to construct the final UniversalId object from the builder state
     *
     * @note UniversalId combines vessel identification (IMO number) with sensor identification (LocalId)
     *       to create globally unique identifiers in the maritime domain
     */
    class UniversalIdBuilder final
    {
    private:
        UniversalIdBuilder() = default;

    public:
        UniversalIdBuilder(const UniversalIdBuilder&) = default;
        UniversalIdBuilder(UniversalIdBuilder&&) noexcept = default;
        ~UniversalIdBuilder() = default;

        UniversalIdBuilder& operator=(const UniversalIdBuilder&) = default;
        UniversalIdBuilder& operator=(UniversalIdBuilder&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const UniversalIdBuilder& other) const noexcept;

        /**
         * @brief Get ISO 19848 Annex C naming entity
         * @return String view containing the naming entity ("data.dnv.com")
         */
        [[nodiscard]] static constexpr std::string_view namingEntity() noexcept;

        /** @brief Get the IMO number (nullopt if not set) */
        [[nodiscard]] inline const std::optional<ImoNumber>& imoNumber() const noexcept;

        /** @brief Get the LocalIdBuilder (nullopt if not set) */
        [[nodiscard]] inline const std::optional<LocalIdBuilder>& localId() const noexcept;

        /**
         * @brief Check if builder state is valid for building a UniversalId
         * @return True if both IMO number and valid LocalIdBuilder are set
         */
        [[nodiscard]] inline bool isValid() const noexcept;

        /**
         * @brief Create a new UniversalIdBuilder for the given VIS version
         * @param visVersion VIS version for the LocalId component
         * @return New UniversalIdBuilder instance with LocalIdBuilder initialized
         */
        [[nodiscard]] static inline UniversalIdBuilder create(VisVersion visVersion) noexcept;

        /**
         * @brief Set the IMO number
         * @param imoNumber IMO number for vessel identification
         * @return New UniversalIdBuilder with IMO number set
         */
        [[nodiscard]] inline UniversalIdBuilder withImoNumber(const ImoNumber& imoNumber) const&;

        /**
         * @brief Set the IMO number
         * @param imoNumber IMO number for vessel identification
         * @return New UniversalIdBuilder with IMO number set
         */
        [[nodiscard]] inline UniversalIdBuilder withImoNumber(const ImoNumber& imoNumber) &&;

        /**
         * @brief Set the IMO number from optional
         * @param imoNumber Optional IMO number
         * @return New UniversalIdBuilder with IMO number set
         * @throws std::invalid_argument if imoNumber is nullopt
         */
        [[nodiscard]] inline UniversalIdBuilder withImoNumber(const std::optional<ImoNumber>& imoNumber) const&;

        /**
         * @brief Set the IMO number from optional
         * @param imoNumber Optional IMO number
         * @return New UniversalIdBuilder with IMO number set
         * @throws std::invalid_argument if imoNumber is nullopt
         */
        [[nodiscard]] inline UniversalIdBuilder withImoNumber(const std::optional<ImoNumber>& imoNumber) &&;

        /**
         * @brief Remove the IMO number
         * @return New UniversalIdBuilder with IMO number cleared
         */
        [[nodiscard]] inline UniversalIdBuilder withoutImoNumber() const&;

        /**
         * @brief Remove the IMO number
         * @return New UniversalIdBuilder with IMO number cleared
         */
        [[nodiscard]] inline UniversalIdBuilder withoutImoNumber() && noexcept;

        /**
         * @brief Set the LocalIdBuilder
         * @param localIdBuilder LocalIdBuilder containing sensor identification
         * @return New UniversalIdBuilder with LocalIdBuilder set
         */
        [[nodiscard]] inline UniversalIdBuilder withLocalId(const LocalIdBuilder& localIdBuilder) const&;

        /**
         * @brief Set the LocalIdBuilder
         * @param localIdBuilder LocalIdBuilder containing sensor identification
         * @return New UniversalIdBuilder with LocalIdBuilder set
         */
        [[nodiscard]] inline UniversalIdBuilder withLocalId(const LocalIdBuilder& localIdBuilder) &&;

        /**
         * @brief Set the LocalIdBuilder from optional
         * @param localIdBuilder Optional LocalIdBuilder
         * @return New UniversalIdBuilder with LocalIdBuilder set
         * @throws std::invalid_argument if localIdBuilder is nullopt
         */
        [[nodiscard]] inline UniversalIdBuilder withLocalId(const std::optional<LocalIdBuilder>& localIdBuilder) const&;

        /**
         * @brief Set the LocalIdBuilder from optional
         * @param localIdBuilder Optional LocalIdBuilder
         * @return New UniversalIdBuilder with LocalIdBuilder set
         * @throws std::invalid_argument if localIdBuilder is nullopt
         */
        [[nodiscard]] inline UniversalIdBuilder withLocalId(const std::optional<LocalIdBuilder>& localIdBuilder) &&;

        /**
         * @brief Remove the LocalIdBuilder
         * @return New UniversalIdBuilder with LocalIdBuilder cleared
         */
        [[nodiscard]] inline UniversalIdBuilder withoutLocalId() const&;

        /**
         * @brief Remove the LocalIdBuilder
         * @return New UniversalIdBuilder with LocalIdBuilder cleared
         */
        [[nodiscard]] inline UniversalIdBuilder withoutLocalId() && noexcept;

        /**
         * @brief Build the final UniversalId object from builder state
         * @return UniversalId instance constructed from current builder state
         * @throws std::invalid_argument if builder state is invalid (missing IMO number or LocalIdBuilder)
         */
        [[nodiscard]] UniversalId build() const;

        /**
         * @brief Convert builder state to string representation
         * @return String representation in format: data.dnv.com/{imo-number}{local-id}
         * @throws std::invalid_argument if IMO number or LocalIdBuilder is missing or invalid
         */
        [[nodiscard]] inline std::string toString() const;

        /**
         * @brief Append string representation to a string
         * @param out String to append the string representation to
         * @throws std::invalid_argument if IMO number or LocalIdBuilder is missing
         */
        inline void toString(std::string& out) const;

        /**
         * @brief Create UniversalId from string representation
         * @param universalIdStr String representation of UniversalId (e.g.,
         * "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet")
         * @return Optional containing UniversalId if parsing succeeded, nullopt otherwise
         */
        [[nodiscard]] static std::optional<UniversalId> fromString(std::string_view universalIdStr);

        /**
         * @brief Create UniversalId from string representation with error reporting
         * @param universalIdStr String representation of UniversalId
         * @param errors Output parameter filled with parsing errors if parsing fails
         * @return Optional containing UniversalId if parsing succeeded, nullopt otherwise
         */
        [[nodiscard]] static std::optional<UniversalId> fromString(
            std::string_view universalIdStr, ParsingErrors& errors);

    private:
        static std::optional<UniversalId> fromString(std::string_view universalIdStr, ParsingErrors* outErrors);

        std::optional<ImoNumber> m_imoNumber;           ///< IMO number for vessel identification
        std::optional<LocalIdBuilder> m_localIdBuilder; ///< LocalIdBuilder for sensor identification
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/UniversalIdBuilder.inl"
