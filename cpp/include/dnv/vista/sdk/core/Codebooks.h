/**
 * @file Codebooks.h
 * @brief VIS codebooks container with version-specific codebook access
 * @details Container for all VIS codebooks for a specific version. Provides access
 *          to individual Codebook instances and convenience methods for creating tags
 */

#pragma once

#include "Codebook.h"

#include <array>
#include <optional>

namespace dnv::vista::sdk
{
    struct CodebooksDto;
    enum class VisVersion : std::uint8_t;

    /**
     * @brief Container for all VIS codebooks for a specific version
     * @details Holds all 11 codebook types (Position, Quantity, Content, etc.) for a given
     *          VIS version. Provides indexed access and convenience methods for tag creation
     */
    class Codebooks final
    {
        friend class VIS;

    private:
        /**
         * @brief Constructs Codebooks for a specific VIS version from a DTO
         * @param version The VIS version these codebooks belong to
         * @param dto DTO containing all codebook data
         */
        explicit Codebooks(VisVersion version, const CodebooksDto& dto);

    public:
        Codebooks() = delete;
        Codebooks(const Codebooks&) = default;
        Codebooks(Codebooks&&) noexcept = default;
        ~Codebooks() = default;

        Codebooks& operator=(const Codebooks&) = default;
        Codebooks& operator=(Codebooks&&) noexcept = default;

        /**
         * @brief Get a codebook by name
         * @param name The codebook name
         * @return Const reference to the requested Codebook
         * @throws std::out_of_range if name is invalid
         */
        inline const Codebook& operator[](CodebookName name) const;

        /**
         * @brief Get the VIS version for these codebooks
         * @return The VisVersion enum value
         */
        inline VisVersion version() const noexcept;

    private:
        /** @brief VIS version for these codebooks */
        VisVersion m_version;

        /** @brief Fixed-size array holding all codebooks */
        std::array<std::optional<Codebook>, static_cast<size_t>(CodebookName::Detail)> m_codebooks;
    };
} // namespace dnv::vista::sdk

#include "dnv/vista/sdk/detail/core/Codebooks.inl"
