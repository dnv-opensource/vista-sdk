#include "dnv/vista/sdk/transport/datachannel/DataChannel.h"

#include "dnv/vista/sdk/detail/core/ISO19848AnnexC.h"

#include <algorithm>

namespace dnv::vista::sdk::transport::datachannel
{
    ConfigurationReference::ConfigurationReference(
        std::string id, DateTimeOffset timeStamp, std::optional<std::string> version)
        : m_id{ std::move(id) },
          m_version{ std::move(version) },
          m_timeStamp{ std::move(timeStamp) }
    {}

    VersionInformation::VersionInformation()
        : m_namingRule{ internal::iso19848::annexC::NamingRule },
          m_namingSchemeVersion{ internal::iso19848::annexC::NamingSchemeVersion },
          m_referenceUrl{ internal::iso19848::annexC::ReferenceUrl }
    {}

    VersionInformation::VersionInformation(
        std::string namingRule, std::string namingSchemeVersion, std::optional<std::string> referenceUrl)
        : m_namingRule{ std::move(namingRule) },
          m_namingSchemeVersion{ std::move(namingSchemeVersion) },
          m_referenceUrl{ std::move(referenceUrl) }
    {}

    NameObject::NameObject()
        : m_namingRule{ std::string{ "/" } + std::string{ internal::iso19848::annexC::VersionedNamingRule } }
    {}

    NameObject::NameObject(std::string namingRule, std::optional<CustomNameObjects> customNameObjects)
        : m_namingRule{ std::move(namingRule) },
          m_customNameObjects{ std::move(customNameObjects) }
    {}

    Range::Range(double low, double high)
        : m_low{ low },
          m_high{ high }
    {
        if (low > high)
        {
            throw std::invalid_argument{ "Low value must be less than high value" };
        }
    }

    ValidateResult<> Restriction::validateNumber(double number) const
    {
        std::vector<std::string> errors;

        if (m_maxExclusive && number >= m_maxExclusive.value())
        {
            errors.push_back(
                "Value " + std::to_string(number) + " is greater than or equal to " +
                std::to_string(m_maxExclusive.value()));
        }
        if (m_maxInclusive && number > m_maxInclusive.value())
        {
            errors.push_back(
                "Value " + std::to_string(number) + " is greater than " + std::to_string(m_maxInclusive.value()));
        }
        if (m_minExclusive && number <= m_minExclusive.value())
        {
            errors.push_back(
                "Value " + std::to_string(number) + " is less than or equal to " +
                std::to_string(m_minExclusive.value()));
        }
        if (m_minInclusive && number < m_minInclusive.value())
        {
            errors.push_back(
                "Value " + std::to_string(number) + " is less than " + std::to_string(m_minInclusive.value()));
        }
        if (!errors.empty())
        {
            return ValidateResult<>::invalid(std::move(errors));
        }

        return ValidateResult<>::ok();
    }

    ValidateResult<> Restriction::validateValue(std::string_view value, const Format& format) const
    {
        // Check enumeration
        if (m_enumeration)
        {
            const auto& enumVec = m_enumeration.value();
            if (std::find(enumVec.begin(), enumVec.end(), value) == enumVec.end())
            {
                return ValidateResult<>::invalid("Value " + std::string{ value } + " is not in the enumeration");
            }
        }

        // Get format data type from ISO19848
        auto formatTypes = ISO19848::instance().formatDataTypes(ISO19848::instance().latest());
        auto parseResult = formatTypes.fromString(format.type());
        if (!parseResult)
        {
            return ValidateResult<>::invalid("Invalid format type: " + format.type());
        }

        const auto& dataType = parseResult.value().value();

        // Validate based on type
        auto validateResult = dataType.validate(value);
        if (!validateResult)
        {
            return ValidateResult<>::invalid(validateResult.errors());
        }

        if (!validateResult.value().has_value())
        {
            return ValidateResult<>::invalid("Failed to parse value");
        }

        const auto& parsedValue = validateResult.value().value();

        // Type-specific validation
        if (auto dec = parsedValue.decimal())
        {
            if (m_fractionDigits)
            {
                if (dec->decimalPlacesCount() > m_fractionDigits.value())
                {
                    return ValidateResult<>::invalid("Value has more decimal places than allowed");
                }
            }

            auto numResult = validateNumber(dec->toDouble());
            if (!numResult)
            {
                return numResult;
            }

            if (m_totalDigits)
            {
                auto numDigits = dec->totalDigitsCount();
                if (numDigits != m_totalDigits.value())
                {
                    return ValidateResult<>::invalid(
                        "Value has " + std::to_string(numDigits) + " digits but should be " +
                        std::to_string(m_totalDigits.value()));
                }
            }
        }
        else if (auto i = parsedValue.integer())
        {
            auto numResult = validateNumber(static_cast<double>(*i));
            if (!numResult)
            {
                return numResult;
            }

            if (m_totalDigits)
            {
                auto numDigits = Decimal{ *i }.totalDigitsCount();
                if (numDigits != m_totalDigits.value())
                {
                    return ValidateResult<>::invalid(
                        "Value has " + std::to_string(numDigits) + " digits but should be " +
                        std::to_string(m_totalDigits.value()));
                }
            }
        }
        else if (auto str = parsedValue.string())
        {
            auto length = str->length();

            if (m_length && length != m_length.value())
            {
                return ValidateResult<>::invalid(
                    "Value has length " + std::to_string(length) + " but should be " +
                    std::to_string(m_length.value()));
            }
            if (m_maxLength && length > m_maxLength.value())
            {
                return ValidateResult<>::invalid(
                    "Value has length " + std::to_string(length) + " but should be less than " +
                    std::to_string(m_maxLength.value()));
            }
            if (m_minLength && length < m_minLength.value())
            {
                return ValidateResult<>::invalid(
                    "Value has length " + std::to_string(length) + " but should be greater than " +
                    std::to_string(m_minLength.value()));
            }
            if (m_cachedPattern)
            {
                if (!std::regex_match(std::string{ *str }, *m_cachedPattern))
                {
                    return ValidateResult<>::invalid(
                        "Value " + std::string{ *str } + " does not match pattern " + m_pattern.value());
                }
            }
        }

        return ValidateResult<>::ok();
    }

    Format::Format(std::string type, std::optional<Restriction> restriction)
        : m_restriction{ std::move(restriction) }
    {
        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(iso.latest());
        auto result = formatTypes.fromString(type);

        if (!result)
        {
            throw std::invalid_argument{ "Invalid format type: " + type };
        }

        m_type = result.value()->type;
    }

    ValidateResult<> Format::validateValue(std::string_view value, Value& parsedValue) const
    {
        auto formatTypes = ISO19848::instance().formatDataTypes(ISO19848::instance().latest());
        auto parseResult = formatTypes.fromString(m_type);
        if (!parseResult)
        {
            return ValidateResult<>::invalid("Invalid format type: " + m_type);
        }

        const auto& dataType = parseResult.value().value();

        auto validateResult = dataType.validate(value);

        if (!validateResult)
        {
            return ValidateResult<>::invalid(validateResult.errors());
        }

        if (!validateResult.value().has_value())
        {
            return ValidateResult<>::invalid("Failed to parse value");
        }

        parsedValue = std::move(validateResult.value().value());

        if (m_restriction)
        {
            return m_restriction.value().validateValue(value, *this);
        }

        return ValidateResult<>::ok();
    }

    void Format::setType(std::string_view type)
    {
        auto& iso = ISO19848::instance();
        auto formatTypes = iso.formatDataTypes(iso.latest());
        auto result = formatTypes.fromString(type);

        if (!result)
        {
            throw std::invalid_argument{ "Invalid format type: " + std::string{ type } };
        }

        m_type = result.value()->type;
    }

    DataChannelType::DataChannelType(
        std::string type, std::optional<double> updateCycle, std::optional<double> calculationPeriod)
        : m_updateCycle{ updateCycle },
          m_calculationPeriod{ calculationPeriod }
    {
        auto& iso = ISO19848::instance();
        auto names = iso.dataChannelTypeNames(iso.latest());
        auto result = names.fromString(type);

        if (!result)
        {
            throw std::invalid_argument{ "Invalid data channel type: " + type };
        }

        m_type = result.value()->type;

        if (updateCycle.has_value() && updateCycle.value() < 0.0)
        {
            throw std::invalid_argument{ "UpdateCycle must be non-negative" };
        }
        if (calculationPeriod.has_value() && calculationPeriod.value() < 0.0)
        {
            throw std::invalid_argument{ "CalculationPeriod must be non-negative" };
        }
    }

    void DataChannelType::setType(std::string_view type)
    {
        auto& iso = ISO19848::instance();
        auto names = iso.dataChannelTypeNames(iso.latest());
        auto result = names.fromString(type);

        if (!result)
        {
            throw std::invalid_argument{ "Invalid data channel type: " + std::string{ type } };
        }

        m_type = result.value()->type;
    }

    Unit::Unit(
        std::string unitSymbol, std::optional<std::string> quantityName, std::optional<CustomElements> customElements)
        : m_unitSymbol{ std::move(unitSymbol) },
          m_quantityName{ std::move(quantityName) },
          m_customElements{ std::move(customElements) }
    {}

    Header::Header(
        ShipId shipId,
        ConfigurationReference dataChannelListId,
        std::optional<VersionInformation> versionInformation,
        std::optional<std::string> author,
        std::optional<DateTimeOffset> dateCreated,
        std::optional<CustomHeaders> customHeaders)
        : m_shipId{ std::move(shipId) },
          m_dataChannelListId{ std::move(dataChannelListId) },
          m_versionInformation{ std::move(versionInformation) },
          m_author{ std::move(author) },
          m_dateCreated{ std::move(dateCreated) },
          m_customHeaders{ std::move(customHeaders) }
    {}

    Property::Property(
        DataChannelType dataChannelType,
        Format format,
        std::optional<Range> range,
        std::optional<Unit> unit,
        std::optional<std::string> qualityCoding,
        std::optional<std::string> alertPriority,
        std::optional<std::string> name,
        std::optional<std::string> remarks,
        std::optional<CustomProperties> customProperties)
        : m_dataChannelType{ std::move(dataChannelType) },
          m_format{ std::move(format) },
          m_range{ std::move(range) },
          m_unit{ std::move(unit) },
          m_qualityCoding{ std::move(qualityCoding) },
          m_alertPriority{ std::move(alertPriority) },
          m_name{ std::move(name) },
          m_remarks{ std::move(remarks) },
          m_customProperties{ std::move(customProperties) }
    {}

    DataChannelId::DataChannelId(
        LocalId localId, std::optional<std::string> shortId, std::optional<NameObject> nameObject)
        : m_localId{ std::move(localId) },
          m_shortId{ std::move(shortId) },
          m_nameObject{ std::move(nameObject) }
    {}

    DataChannel::DataChannel(DataChannelId dataChannelId, Property property)
        : m_dataChannelId{ std::move(dataChannelId) },
          m_property{ std::move(property) }
    {
        detail::throwIfInvalid(m_property.validate());
    }

    DataChannelList::DataChannelList(const std::vector<DataChannel>& dataChannels)
    {
        add(dataChannels);
    }

    Package::Package(Header header, DataChannelList dataChannelList)
        : m_header{ std::move(header) },
          m_dataChannelList{ std::move(dataChannelList) }
    {}

    DataChannelListPackage::DataChannelListPackage(Package package)
        : m_package{ std::move(package) }
    {}
} // namespace dnv::vista::sdk::transport::datachannel
