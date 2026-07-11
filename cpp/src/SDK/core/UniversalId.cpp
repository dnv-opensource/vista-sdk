#include "dnv/vista/sdk/core/UniversalId.h"

#include "dnv/vista/sdk/core/ParsingErrors.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    namespace
    {
        const LocalIdBuilder& requireValidLocalId(const UniversalIdBuilder& builder)
        {
            if (!builder.isValid())
            {
                throw std::invalid_argument{
                    "Cannot create UniversalId: builder is invalid (missing IMO number or LocalId)"
                };
            }

            return *builder.localId();
        }
    } // namespace

    UniversalId::UniversalId(const UniversalIdBuilder& builder)
        : m_builder{ builder },
          m_localId{ requireValidLocalId(builder).build() }
    {}

    std::optional<UniversalId> UniversalId::fromString(std::string_view universalIdStr) noexcept
    {
        return UniversalIdBuilder::fromString(universalIdStr);
    }

    std::optional<UniversalId> UniversalId::fromString(std::string_view universalIdStr, ParsingErrors& errors) noexcept
    {
        return UniversalIdBuilder::fromString(universalIdStr, errors);
    }
} // namespace dnv::vista::sdk
