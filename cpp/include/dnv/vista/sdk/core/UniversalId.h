/**
 * @file UniversalId.h
 * @brief Immutable VIS Universal ID representation
 * @details Provides an immutable UniversalId class representing a globally unique sensor identifier
 *          combining vessel identification (IMO number) with sensor identification (LocalId)
 *          Format:
 * data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet
 */

#pragma once

#include "ImoNumber.h"
#include "LocalId.h"
#include "UniversalIdBuilder.h"

#include <optional>
#include <string>
#include <string_view>

namespace dnv::vista::sdk
{
    class ParsingErrors;

    /**
     * @class UniversalId
     * @brief Immutable representation of a VIS Universal ID
     * @details Represents a globally unique sensor identifier combining IMO number and LocalId
     *          UniversalId is constructed from a UniversalIdBuilder and provides read-only access to components
     *          Once constructed, a UniversalId cannot be modified. To create variations, access the builder
     *          via builder() and create a new UniversalId instance
     *
     * @note UniversalId follows the format: {naming-entity}/{imo-number}{local-id}
     *       Example:
     * data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet
     */
    class UniversalId final
    {
        friend class UniversalIdBuilder;

    private:
        /**
         * @brief Construct UniversalId from builder
         * @param builder UniversalIdBuilder with validated state
         * @throws std::invalid_argument if builder is invalid (missing IMO number or LocalId)
         */
        explicit UniversalId(const UniversalIdBuilder& builder);

    public:
        UniversalId() = delete;
        UniversalId(const UniversalId&) = default;
        UniversalId(UniversalId&&) noexcept = default;
        ~UniversalId() = default;

        UniversalId& operator=(const UniversalId&) = default;
        UniversalId& operator=(UniversalId&&) noexcept = default;

        [[nodiscard]] inline bool operator==(const UniversalId& other) const noexcept;

        /**
         * @brief Get ISO 19848 Annex C naming entity
         * @return String view containing the naming entity ("data.dnv.com")
         */
        [[nodiscard]] static constexpr std::string_view namingEntity() noexcept;

        /** @brief Get the IMO number */
        [[nodiscard]] inline const ImoNumber& imoNumber() const noexcept;

        /** @brief Get the LocalId */
        [[nodiscard]] inline const LocalId& localId() const noexcept;

        /** @brief Get the builder that created this UniversalId */
        [[nodiscard]] inline const UniversalIdBuilder& builder() const noexcept;

        /**
         * @brief Convert to string representation (lvalue-qualified)
         * @return String representation in format: data.dnv.com/{imo-number}{local-id}
         */
        [[nodiscard]] inline std::string toString() const&;

        /**
         * @brief Convert to string representation (rvalue-qualified)
         * @return String representation with potential move optimization
         */
        [[nodiscard]] inline std::string toString() &&;

        /**
         * @brief Append string representation to a string
         * @param out String to append the string representation to
         */
        inline void toString(std::string& out) const;

        /**
         * @brief Create UniversalId from string representation
         * @param universalIdStr String representation of UniversalId
         * @return Optional containing UniversalId if parsing succeeded, nullopt otherwise
         * @note This method silently fails on parse errors. Use the overload with ParsingErrors for diagnostics
         */
        [[nodiscard]] static std::optional<UniversalId> fromString(std::string_view universalIdStr) noexcept;

        /**
         * @brief Create UniversalId from string representation with error reporting
         * @param universalIdStr String representation of UniversalId
         * @param errors Output parameter filled with parsing errors if parsing fails
         * @return Optional containing UniversalId if parsing succeeded, nullopt otherwise
         */
        [[nodiscard]] static std::optional<UniversalId> fromString(
            std::string_view universalIdStr, ParsingErrors& errors) noexcept;

    private:
        UniversalIdBuilder m_builder; ///< Internal builder containing all state
        LocalId m_localId;            ///< Pre-built LocalId for efficient access
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/UniversalId.inl"
