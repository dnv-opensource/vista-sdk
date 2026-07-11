#include "dnv/vista/sdk/core/UniversalIdBuilder.h"

#include "internal/parsing/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/detail/core/ISO19848AnnexC.h"
#include "dnv/vista/sdk/core/ParsingErrors.h"
#include "dnv/vista/sdk/core/UniversalId.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    UniversalId UniversalIdBuilder::build() const
    {
        if (!isValid())
        {
            throw std::invalid_argument{
                "Cannot build UniversalId: builder is invalid (missing IMO number or LocalId)"
            };
        }
        return UniversalId{ *this };
    }

    std::optional<UniversalId> UniversalIdBuilder::fromString(std::string_view universalIdStr)
    {
        return fromString(universalIdStr, nullptr);
    }

    std::optional<UniversalId> UniversalIdBuilder::fromString(std::string_view universalIdStr, ParsingErrors& errors)
    {
        return fromString(universalIdStr, &errors);
    }

    std::optional<UniversalId> UniversalIdBuilder::fromString(std::string_view universalIdStr, ParsingErrors* outErrors)
    {
        using namespace internal;

        // Format: data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/...
        LocalIdParsingErrorBuilder errorBuilder;

        if (universalIdStr.empty())
        {
            if (outErrors)
            {
                errorBuilder.addError(LocalIdParsingState::NamingRule, "Failed to find localId start segment");
                *outErrors = errorBuilder.build();
            }
            return std::nullopt;
        }

        // Find the start of LocalId: "/dnv-v2/..."
        static const std::string localIdPrefix =
            std::string{ "/" } + std::string{ iso19848::annexC::NamingRule } + "-v";
        const auto localIdStartPos = universalIdStr.find(localIdPrefix);
        if (localIdStartPos == std::string_view::npos)
        {
            if (outErrors)
            {
                errorBuilder.addError(LocalIdParsingState::NamingRule, "Failed to find localId start segment");
                *outErrors = errorBuilder.build();
            }
            return std::nullopt;
        }

        const auto universalIdSegment = universalIdStr.substr(0, localIdStartPos);
        const auto localIdSegment = universalIdStr.substr(localIdStartPos);

        std::optional<LocalId> localId;
        if (outErrors)
        {
            ParsingErrors localIdErrors;
            localId = LocalIdBuilder::fromString(localIdSegment, localIdErrors);
            if (!localId.has_value())
            {
                *outErrors = localIdErrors;
                return std::nullopt;
            }
        }
        else
        {
            localId = LocalIdBuilder::fromString(localIdSegment);
            if (!localId.has_value())
            {
                return std::nullopt;
            }
        }

        const auto& localIdBuilder = localId->builder();

        // Parse "data.dnv.com/IMO1234567"
        const auto slashPos = universalIdSegment.find('/');
        if (slashPos == std::string_view::npos)
        {
            if (outErrors)
            {
                errorBuilder.addError(
                    LocalIdParsingState::NamingEntity, "Invalid Universal ID format: missing IMO number");
                *outErrors = errorBuilder.build();
            }
            return std::nullopt;
        }

        const auto namingEntitySegment = universalIdSegment.substr(0, slashPos);
        const auto afterNamingEntity = universalIdSegment.substr(slashPos + 1);

        if (namingEntitySegment != iso19848::annexC::NamingEntity)
        {
            if (outErrors)
            {
                errorBuilder.addError(
                    LocalIdParsingState::NamingEntity,
                    std::string{ "Naming entity segment didn't match. Found: " } + std::string{ namingEntitySegment });
                *outErrors = errorBuilder.build();
            }
            return std::nullopt;
        }

        // IMO number = everything before any additional slash
        const auto imoSlash = afterNamingEntity.find('/');
        const auto imoSegment =
            imoSlash == std::string_view::npos ? afterNamingEntity : afterNamingEntity.substr(0, imoSlash);

        std::optional<ImoNumber> imoNumber;
        if (!imoSegment.empty())
        {
            auto imo = ImoNumber::fromString(imoSegment);
            if (!imo.has_value())
            {
                if (outErrors)
                {
                    errorBuilder.addError(LocalIdParsingState::IMONumber, "Invalid IMO number segment");
                    *outErrors = errorBuilder.build();
                }
                return std::nullopt;
            }
            imoNumber = imo;
        }

        if (!localIdBuilder.version().has_value())
        {
            if (outErrors)
            {
                errorBuilder.addError(LocalIdParsingState::VisVersion, "Invalid VisVersion in LocalId");
                *outErrors = errorBuilder.build();
            }
            return std::nullopt;
        }

        auto builder = UniversalIdBuilder::create(*localIdBuilder.version()).withLocalId(localIdBuilder);

        if (imoNumber.has_value())
        {
            builder = builder.withImoNumber(*imoNumber);
        }

        if (!builder.isValid())
        {
            if (outErrors)
            {
                errorBuilder.addError(LocalIdParsingState::Completeness, "UniversalIdBuilder is invalid after parsing");
                *outErrors = errorBuilder.build();
            }
            return std::nullopt;
        }

        if (outErrors)
        {
            *outErrors = errorBuilder.build();
        }

        return builder.build();
    }
} // namespace dnv::vista::sdk
