/**
 * @file DataChannel.h
 * @brief Data Channel domain model for ISO 19848 compliance
 */

#pragma once

#include <dnv/vista/sdk/Export.h>

#include "dnv/vista/sdk/core/LocalId.h"
#include "dnv/vista/sdk/transport/serialization/json/SerializableDocument.h"
#include "dnv/vista/sdk/transport/ISO19848.h"
#include "dnv/vista/sdk/transport/Results.h"
#include "dnv/vista/sdk/transport/ShipId.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <optional>
#include <regex>
#include <string>

namespace dnv::vista::sdk::transport::datachannel
{
    //=====================================================================
    // Type aliases
    //=====================================================================

    /** @brief Extension point for arbitrary custom headers on a Package (ISO 19848 Table 10) */
    using CustomHeaders = serialization::json::SerializableDocument;

    /** @brief Extension point for arbitrary custom name objects on a NameObject (ISO 19848 Table 22) */
    using CustomNameObjects = serialization::json::SerializableDocument;

    /** @brief Extension point for arbitrary custom elements on a Unit (ISO 19848 Table 21) */
    using CustomElements = serialization::json::SerializableDocument;

    /** @brief Extension point for arbitrary custom properties on a Property (ISO 19848 Table 16) */
    using CustomProperties = serialization::json::SerializableDocument;

    //=====================================================================
    // ConfigurationReference class - Table 11
    //=====================================================================

    /**
     * @brief Configuration reference with version and timestamp (ISO 19848 ConfigurationReference)
     * @details ISO 19848:2024 Table 11: ConfigurationReference structure
     *          - ID: Mandatory, String - Identifier of Data Channel List itself
     *          - Version: Optional, String - Version of DataChannelList
     *          - TimeStamp: Mandatory, DateTime - Modified Date and Time
     */
    class ConfigurationReference final
    {
    public:
        /**
         * @brief Construct ConfigurationReference with all fields
         * @param id Configuration identifier (Mandatory, ISO 19848 Table 11, ID element - Identifier of Data Channel
         * List itself)
         * @param timeStamp Configuration timestamp (Mandatory, ISO 19848 Table 11, TimeStamp element - Modified Date
         * and Time)
         * @param version Optional version string (ISO 19848 Table 11, Version element - Version of DataChannelList)
         */
        DNV_VISTA_SDK_CPP_API explicit ConfigurationReference(
            std::string id, DateTimeOffset timeStamp, std::optional<std::string> version = std::nullopt);

        ConfigurationReference() = default;
        ConfigurationReference(const ConfigurationReference&) = default;
        ConfigurationReference(ConfigurationReference&&) noexcept = default;
        ~ConfigurationReference() = default;

        ConfigurationReference& operator=(const ConfigurationReference&) = default;
        ConfigurationReference& operator=(ConfigurationReference&&) noexcept = default;

        /**
         * @brief Get configuration identifier
         * @return Configuration identifier string (ISO 19848 Table 11, ID element)
         */
        [[nodiscard]] inline const std::string& id() const noexcept;

        /**
         * @brief Get optional version
         * @return Optional version string (ISO 19848 Table 11, Version element)
         */
        [[nodiscard]] inline const std::optional<std::string>& version() const noexcept;

        /**
         * @brief Get timestamp
         * @return Configuration timestamp (ISO 19848 Table 11, TimeStamp element)
         */
        [[nodiscard]] inline const DateTimeOffset& timeStamp() const noexcept;

        /**
         * @brief Set configuration identifier
         * @param id Configuration identifier string (ISO 19848 Table 11, ID element)
         */
        inline void setId(std::string_view id);

        /**
         * @brief Set optional version
         * @param version Optional version string (ISO 19848 Table 11, Version element)
         */
        inline void setVersion(std::optional<std::string> version);

        /**
         * @brief Set timestamp
         * @param timeStamp Configuration timestamp (ISO 19848 Table 11, TimeStamp element)
         */
        inline void setTimeStamp(DateTimeOffset timeStamp);

    private:
        std::string m_id;
        std::optional<std::string> m_version;
        DateTimeOffset m_timeStamp;
    };

    //=====================================================================
    // VersionInformation class - Table 12
    //=====================================================================

    /**
     * @brief Version information for naming rules (ISO 19848 VersionInformation)
     * @details ISO 19848:2024 Table 12: VersionInformation structure
     *          - NamingRule: Mandatory, String - Name of naming scheme
     *          - NamingSchemeVersion: Mandatory, String - Version of naming scheme corresponding to the Naming Rule
     *            When a DataChannelID string is changed, this shall be changed
     *            Adding/removing DataChannel shall keep the version
     *          - ReferenceURL: Optional, String - Definition of Local Data Name structure
     */
    class VersionInformation final
    {
    public:
        /** @brief Construct VersionInformation with default Annex C naming rule, scheme version, and reference URL */
        DNV_VISTA_SDK_CPP_API VersionInformation();

        /**
         * @brief Construct VersionInformation with all fields
         * @param namingRule Naming rule identifier (Mandatory, ISO 19848 Table 12, NamingRule element - Name of naming
         * scheme)
         * @param namingSchemeVersion Naming scheme version (Mandatory, ISO 19848 Table 12, NamingSchemeVersion element)
         * @param referenceUrl Optional reference URL (ISO 19848 Table 12, ReferenceURL element - Definition of Local
         * Data Name structure)
         */
        DNV_VISTA_SDK_CPP_API VersionInformation(
            std::string namingRule,
            std::string namingSchemeVersion,
            std::optional<std::string> referenceUrl = std::nullopt);

        VersionInformation(const VersionInformation&) = default;
        VersionInformation(VersionInformation&&) noexcept = default;
        ~VersionInformation() = default;

        VersionInformation& operator=(const VersionInformation&) = default;
        VersionInformation& operator=(VersionInformation&&) noexcept = default;

        /**
         * @brief Get naming rule
         * @return Naming rule identifier (ISO 19848 Table 12, NamingRule element)
         */
        [[nodiscard]] inline const std::string& namingRule() const noexcept;

        /**
         * @brief Get naming scheme version
         * @return Naming scheme version string (ISO 19848 Table 12, NamingSchemeVersion element)
         */
        [[nodiscard]] inline const std::string& namingSchemeVersion() const noexcept;

        /**
         * @brief Get optional reference URL
         * @return Optional reference URL (ISO 19848 Table 12, ReferenceURL element)
         */
        [[nodiscard]] inline const std::optional<std::string>& referenceUrl() const noexcept;

        /**
         * @brief Set naming rule
         * @param namingRule Naming rule identifier (ISO 19848 Table 12, NamingRule element)
         */
        inline void setNamingRule(std::string_view namingRule);

        /**
         * @brief Set naming scheme version
         * @param namingSchemeVersion Naming scheme version string (ISO 19848 Table 12, NamingSchemeVersion element)
         */
        inline void setNamingSchemeVersion(std::string_view namingSchemeVersion);

        /**
         * @brief Set optional reference URL
         * @param referenceUrl Optional reference URL (ISO 19848 Table 12, ReferenceURL element)
         */
        inline void setReferenceUrl(std::optional<std::string> referenceUrl);

    private:
        std::string m_namingRule;
        std::string m_namingSchemeVersion;
        std::optional<std::string> m_referenceUrl;
    };

    //=====================================================================
    // Range class - Table 20
    //=====================================================================

    /**
     * @brief Numeric range with low and high bounds (ISO 19848 Range)
     * @details ISO 19848:2024 Table 20: Range structure
     *          - Low: Mandatory, Real - Lower limit of measuring range
     *          - High: Mandatory, Real - Upper limit of measuring range
     *          Validates that low value is less than high value
     */
    class Range final
    {
    public:
        /**
         * @brief Construct Range with low and high bounds
         * @param low Lower bound (Mandatory, ISO 19848 Table 20, Low element - Lower limit of measuring range)
         * @param high Upper bound (Mandatory, ISO 19848 Table 20, High element - Upper limit of measuring range)
         */
        DNV_VISTA_SDK_CPP_API Range(double low, double high);

        Range() = default;
        Range(const Range&) = default;
        Range(Range&&) noexcept = default;
        ~Range() = default;

        Range& operator=(const Range&) = default;
        Range& operator=(Range&&) noexcept = default;

        /**
         * @brief Get lower bound
         * @return Lower bound value (ISO 19848 Table 20, Low element)
         */
        [[nodiscard]] inline double low() const noexcept;

        /**
         * @brief Get upper bound
         * @return Upper bound value (ISO 19848 Table 20, High element)
         */
        [[nodiscard]] inline double high() const noexcept;

        /**
         * @brief Set lower bound (must be less than high)
         * @param low Lower bound value (ISO 19848 Table 20, Low element)
         */
        inline void setLow(double low);

        /**
         * @brief Set upper bound (must be greater than low)
         * @param high Upper bound value (ISO 19848 Table 20, High element)
         */
        inline void setHigh(double high);

    private:
        double m_low = std::numeric_limits<double>::lowest();
        double m_high = std::numeric_limits<double>::max();
    };

    //=====================================================================
    // Restriction class - Table 19
    //=====================================================================

    class Format;

    /**
     * @brief Value restriction constraints (ISO 19848 Restriction)
     * @details ISO 19848:2024 Table 19: Restriction structure
     *          - Enumeration: Optional, String[] - Described in accordance with Data Channel Type format shown in 5.2
     * b) Table 3
     *          - FractionDigits: Optional, NonNegativeInteger
     *          - Length: Optional, NonNegativeInteger
     *          - MaxExclusive: Optional, Real
     *          - MaxInclusive: Optional, Real
     *          - MaxLength: Optional, NonNegativeInteger
     *          - MinExclusive: Optional, Real
     *          - MinInclusive: Optional, Real
     *          - MinLength: Optional, NonNegativeInteger
     *          - Pattern: Optional, String
     *          - TotalDigits: Optional, PositiveInteger
     *          - WhiteSpace: Optional, One of: "Preserve", "Replace", "Collapse"
     *          Defines constraints for format validation including enumeration, length, pattern, numeric bounds
     */
    class Restriction final
    {
    public:
        /**
         * @brief White space handling modes
         * @details Specifies how white space (line feeds, tabs, spaces, carriage returns) is handled
         */
        enum class WhiteSpace : std::uint8_t
        {
            Preserve = 0, ///< Preserve all white space
            Replace,      ///< Replace sequences of white space with single space
            Collapse      ///< Collapse sequences of white space and trim leading/trailing
        };

        Restriction() = default;
        Restriction(const Restriction&) = default;
        Restriction(Restriction&&) noexcept = default;
        ~Restriction() = default;

        /**
         * @brief Copy assignment operator
         * @return Reference to this Restriction
         */
        Restriction& operator=(const Restriction&) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this Restriction
         */
        Restriction& operator=(Restriction&&) noexcept = default;

        /**
         * @brief Get enumeration of acceptable values
         * @return Optional vector of acceptable string values (ISO 19848 Table 19, Enumeration element)
         */
        [[nodiscard]] inline const std::optional<std::vector<std::string>>& enumeration() const noexcept;

        /**
         * @brief Get maximum decimal places allowed
         * @return Optional maximum number of fraction digits (ISO 19848 Table 19, FractionDigits element)
         */
        [[nodiscard]] inline const std::optional<std::uint32_t>& fractionDigits() const noexcept;

        /**
         * @brief Get exact number of characters or list items
         * @return Optional exact length (ISO 19848 Table 19, Length element)
         */
        [[nodiscard]] inline const std::optional<std::uint32_t>& length() const noexcept;

        /**
         * @brief Get upper bounds (exclusive)
         * @return Optional exclusive maximum value (ISO 19848 Table 19, MaxExclusive element)
         */
        [[nodiscard]] inline const std::optional<double>& maxExclusive() const noexcept;

        /**
         * @brief Get upper bounds (inclusive)
         * @return Optional inclusive maximum value (ISO 19848 Table 19, MaxInclusive element)
         */
        [[nodiscard]] inline const std::optional<double>& maxInclusive() const noexcept;

        /**
         * @brief Get maximum number of characters or list items
         * @return Optional maximum length (ISO 19848 Table 19, MaxLength element)
         */
        [[nodiscard]] inline const std::optional<std::uint32_t>& maxLength() const noexcept;

        /**
         * @brief Get lower bounds (exclusive)
         * @return Optional exclusive minimum value (ISO 19848 Table 19, MinExclusive element)
         */
        [[nodiscard]] inline const std::optional<double>& minExclusive() const noexcept;

        /**
         * @brief Get lower bounds (inclusive)
         * @return Optional inclusive minimum value (ISO 19848 Table 19, MinInclusive element)
         */
        [[nodiscard]] inline const std::optional<double>& minInclusive() const noexcept;

        /**
         * @brief Get minimum number of characters or list items
         * @return Optional minimum length (ISO 19848 Table 19, MinLength element)
         */
        [[nodiscard]] inline const std::optional<std::uint32_t>& minLength() const noexcept;

        /**
         * @brief Get regex pattern for acceptable character sequences
         * @return Optional regex pattern string (ISO 19848 Table 19, Pattern element)
         */
        [[nodiscard]] inline const std::optional<std::string>& pattern() const noexcept;

        /**
         * @brief Get exact number of digits (must be > 0)
         * @return Optional total number of digits (ISO 19848 Table 19, TotalDigits element)
         */
        [[nodiscard]] inline const std::optional<std::uint32_t>& totalDigits() const noexcept;

        /**
         * @brief Get white space handling mode
         * @return Optional white space handling mode (ISO 19848 Table 19, WhiteSpace element - "Preserve", "Replace",
         * or "Collapse")
         */
        [[nodiscard]] inline const std::optional<WhiteSpace>& whiteSpace() const noexcept;

        /**
         * @brief Set enumeration
         * @param enumeration Optional vector of acceptable values (ISO 19848 Table 19, Enumeration element)
         */
        inline void setEnumeration(std::optional<std::vector<std::string>> enumeration);

        /**
         * @brief Set fraction digits
         * @param fractionDigits Optional maximum number of fraction digits (ISO 19848 Table 19, FractionDigits element)
         */
        inline void setFractionDigits(std::optional<std::uint32_t> fractionDigits);

        /**
         * @brief Set length
         * @param length Optional exact length (ISO 19848 Table 19, Length element)
         */
        inline void setLength(std::optional<std::uint32_t> length);

        /**
         * @brief Set max exclusive
         * @param maxExclusive Optional exclusive maximum value (ISO 19848 Table 19, MaxExclusive element)
         */
        inline void setMaxExclusive(std::optional<double> maxExclusive);

        /**
         * @brief Set max inclusive
         * @param maxInclusive Optional inclusive maximum value (ISO 19848 Table 19, MaxInclusive element)
         */
        inline void setMaxInclusive(std::optional<double> maxInclusive);

        /**
         * @brief Set max length
         * @param maxLength Optional maximum length (ISO 19848 Table 19, MaxLength element)
         */
        inline void setMaxLength(std::optional<std::uint32_t> maxLength);

        /**
         * @brief Set min exclusive
         * @param minExclusive Optional exclusive minimum value (ISO 19848 Table 19, MinExclusive element)
         */
        inline void setMinExclusive(std::optional<double> minExclusive);

        /**
         * @brief Set min inclusive
         * @param minInclusive Optional inclusive minimum value (ISO 19848 Table 19, MinInclusive element)
         */
        inline void setMinInclusive(std::optional<double> minInclusive);

        /**
         * @brief Set min length
         * @param minLength Optional minimum length (ISO 19848 Table 19, MinLength element)
         */
        inline void setMinLength(std::optional<std::uint32_t> minLength);

        /**
         * @brief Set pattern
         * @param pattern Optional regex pattern string (ISO 19848 Table 19, Pattern element)
         */
        inline void setPattern(std::optional<std::string> pattern);

        /**
         * @brief Set total digits (must be > 0)
         * @param totalDigits Optional total number of digits (ISO 19848 Table 19, TotalDigits element)
         */
        inline void setTotalDigits(std::optional<std::uint32_t> totalDigits);

        /**
         * @brief Set white space handling
         * @param whiteSpace Optional white space handling mode (ISO 19848 Table 19, WhiteSpace element)
         */
        inline void setWhiteSpace(std::optional<WhiteSpace> whiteSpace);

        /**
         * @brief Validate a string value against restriction constraints
         * @param value The string value to validate
         * @param format The format containing the data type
         * @return Validation result indicating success or error messages
         * @details Validates the value against enumeration, numeric bounds, length, pattern, etc.
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API ValidateResult<> validateValue(
            std::string_view value, const Format& format) const;

    private:
        /**
         * @brief Validate numeric value against bounds
         * @param number The number to validate
         * @return Validation result indicating success or error messages
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API ValidateResult<> validateNumber(double number) const;

        std::optional<std::vector<std::string>> m_enumeration;
        std::optional<std::uint32_t> m_fractionDigits;
        std::optional<std::uint32_t> m_length;
        std::optional<double> m_maxExclusive;
        std::optional<double> m_maxInclusive;
        std::optional<std::uint32_t> m_maxLength;
        std::optional<double> m_minExclusive;
        std::optional<double> m_minInclusive;
        std::optional<std::uint32_t> m_minLength;
        std::optional<std::string> m_pattern;
        std::optional<std::regex> m_cachedPattern;
        std::optional<std::uint32_t> m_totalDigits;
        std::optional<WhiteSpace> m_whiteSpace;
    };

    //=====================================================================
    // Format class - Table 18
    //=====================================================================

    /**
     * @brief Data format type with optional restriction (ISO 19848 Format)
     * @details ISO 19848:2024 Table 18: Format structure
     *          - Type: Mandatory, String - Described in accordance with Data Channel Type format shown in 5.2 b)
     *          - Restriction: Optional, j) Restriction
     *          Defines the data type and optional validation constraints for a data channel
     */
    class Format final
    {
    public:
        /**
         * @brief Construct Format with all fields
         * @param type Format type string (ISO 19848 Table 18, Type element)
         * @param restriction Optional restriction constraints (ISO 19848 Table 18, Restriction element)
         */
        DNV_VISTA_SDK_CPP_API explicit Format(std::string type, std::optional<Restriction> restriction = std::nullopt);

        Format(const Format&) = default;
        Format(Format&&) noexcept = default;
        ~Format() = default;

        Format& operator=(const Format&) = default;
        Format& operator=(Format&&) noexcept = default;

        /**
         * @brief Get format type
         * @return Format type string (ISO 19848 Table 18, Type element)
         */
        [[nodiscard]] inline const std::string& type() const noexcept;

        /**
         * @brief Get optional restriction
         * @return Optional restriction constraints (ISO 19848 Table 18, Restriction element)
         */
        [[nodiscard]] inline const std::optional<Restriction>& restriction() const noexcept;

        /**
         * @brief Set format type
         * @param type Format type string (ISO 19848 Table 18, Type element)
         * @throws std::invalid_argument if type is not a recognized ISO 19848 format type
         */
        DNV_VISTA_SDK_CPP_API void setType(std::string_view type);

        /**
         * @brief Set optional restriction
         * @param restriction Optional restriction constraints (ISO 19848 Table 18, Restriction element)
         */
        inline void setRestriction(std::optional<Restriction> restriction);

        /**
         * @brief Validate a string value against this format
         * @param value The string value to validate
         * @param parsedValue Output parameter for the parsed value
         * @return Validation result indicating success or error messages
         * @details Validates the value against the format type and optional restriction
         */
        [[nodiscard]] DNV_VISTA_SDK_CPP_API ValidateResult<> validateValue(
            std::string_view value, Value& parsedValue) const;

    private:
        std::string m_type;
        std::optional<Restriction> m_restriction;
    };

    //=====================================================================
    // DataChannelType class - Table 17
    //=====================================================================

    /**
     * @brief Data channel type with update cycle and calculation period (ISO 19848 DataChannelType)
     * @details ISO 19848:2024 Table 17: DataChannelType structure
     *          - Type: Mandatory, String - Described in accordance with Data Channel Type format shown in 5.2 a)
     *          - UpdateCycle: Optional, Real
     *          - CalculationPeriod: Optional, Real
     *          Defines the data channel type classification and optional timing parameters
     */
    class DataChannelType final
    {
    public:
        /**
         * @brief Construct DataChannelType with all fields
         * @param type Data channel type string (ISO 19848 Table 17, Type element)
         * @param updateCycle Optional update cycle in seconds (ISO 19848 Table 17, UpdateCycle element)
         * @param calculationPeriod Optional calculation period in seconds (ISO 19848 Table 17, CalculationPeriod
         * element)
         */
        DNV_VISTA_SDK_CPP_API explicit DataChannelType(
            std::string type,
            std::optional<double> updateCycle = std::nullopt,
            std::optional<double> calculationPeriod = std::nullopt);

        DataChannelType() = default;
        DataChannelType(const DataChannelType&) = default;
        DataChannelType(DataChannelType&&) noexcept = default;
        ~DataChannelType() = default;

        DataChannelType& operator=(const DataChannelType&) = default;
        DataChannelType& operator=(DataChannelType&&) noexcept = default;

        /**
         * @brief Get data channel type
         * @return Data channel type string (ISO 19848 Table 17, Type element)
         */
        [[nodiscard]] inline const std::string& type() const noexcept;

        /**
         * @brief Get optional update cycle
         * @return Optional update cycle in seconds (ISO 19848 Table 17, UpdateCycle element)
         */
        [[nodiscard]] inline const std::optional<double>& updateCycle() const noexcept;

        /**
         * @brief Get optional calculation period
         * @return Optional calculation period in seconds (ISO 19848 Table 17, CalculationPeriod element)
         */
        [[nodiscard]] inline const std::optional<double>& calculationPeriod() const noexcept;

        /**
         * @brief Check if type is Alert
         * @return true if type is Alert, false otherwise
         */
        [[nodiscard]] inline bool isAlert() const noexcept;

        /**
         * @brief Set data channel type
         * @param type Data channel type string (ISO 19848 Table 17, Type element)
         * @throws std::invalid_argument if type is not a recognized ISO 19848 data channel type
         */
        DNV_VISTA_SDK_CPP_API void setType(std::string_view type);

        /**
         * @brief Set optional update cycle
         * @param updateCycle Optional update cycle in seconds (ISO 19848 Table 17, UpdateCycle element)
         */
        inline void setUpdateCycle(std::optional<double> updateCycle);

        /**
         * @brief Set optional calculation period
         * @param calculationPeriod Optional calculation period in seconds (ISO 19848 Table 17, CalculationPeriod
         * element)
         */
        inline void setCalculationPeriod(std::optional<double> calculationPeriod);

    private:
        std::string m_type;
        std::optional<double> m_updateCycle;
        std::optional<double> m_calculationPeriod;
    };

    //=====================================================================
    // NameObject class - Table 22
    //=====================================================================

    /**
     * @brief Name object with naming rule and custom extensions (ISO 19848 NameObject)
     * @details ISO 19848:2024 Table 22: NameObject structure
     *          - NamingRule: Mandatory, String - See 5.1.3
     *          Additionally supports custom NameObjects extension point (xs:any) for defining LocalDataName structure
     */
    class NameObject final
    {
    public:
        /**
         * @brief Construct NameObject with default Annex C naming rule ("/dnv-v2")
         */
        DNV_VISTA_SDK_CPP_API NameObject();

        /**
         * @brief Construct NameObject with explicit naming rule
         * @param namingRule Naming rule identifier (ISO 19848 Table 22, NamingRule element)
         * @param customNameObjects Optional custom name objects (extension point)
         */
        DNV_VISTA_SDK_CPP_API explicit NameObject(
            std::string namingRule, std::optional<CustomNameObjects> customNameObjects = std::nullopt);

        NameObject(const NameObject&) = default;
        NameObject(NameObject&&) noexcept = default;
        ~NameObject() = default;

        NameObject& operator=(const NameObject&) = default;
        NameObject& operator=(NameObject&&) noexcept = default;

        /**
         * @brief Get naming rule
         * @return Naming rule identifier (ISO 19848 Table 22, NamingRule element)
         */
        [[nodiscard]] inline const std::string& namingRule() const noexcept;

        /**
         * @brief Get optional custom name objects
         * @return Optional custom name objects (extension point)
         */
        [[nodiscard]] inline const std::optional<CustomNameObjects>& customNameObjects() const noexcept;

        /**
         * @brief Set naming rule
         * @param namingRule Naming rule identifier (ISO 19848 Table 22, NamingRule element)
         */
        inline void setNamingRule(std::string_view namingRule);

        /**
         * @brief Set optional custom name objects
         * @param customNameObjects Optional custom name objects (extension point)
         */
        inline void setCustomNameObjects(std::optional<CustomNameObjects> customNameObjects);

    private:
        std::string m_namingRule;
        std::optional<CustomNameObjects> m_customNameObjects;
    };

    //=====================================================================
    // Unit class - Table 21
    //=====================================================================

    /**
     * @brief Unit of measurement with symbol and quantity name (ISO 19848 Unit)
     * @details ISO 19848:2024 Table 21: Unit structure
     *          - UnitSymbol: Mandatory, String - Unit symbol defined in the ISO 80000 series,
     *            the IEC 80000 series, or Table 4
     *          - QuantityName: Optional, String - Quantity name defined in the ISO 80000 series,
     *            the IEC 80000 series, or Table 4
     *          Additionally supports custom elements extension point
     */
    class Unit final
    {
        // TODO : Validate Unit according to ISO 80000, IEC 80000 or ISO 19848 Table 4
    public:
        /**
         * @brief Construct Unit with all fields
         * @param unitSymbol Unit symbol from ISO 80000/IEC 80000/Table 4 (ISO 19848 Table 21, UnitSymbol element)
         * @param quantityName Optional quantity name from ISO 80000/IEC 80000/Table 4 (ISO 19848 Table 21, QuantityName
         * element)
         * @param customElements Optional custom elements (extension point)
         */
        DNV_VISTA_SDK_CPP_API explicit Unit(
            std::string unitSymbol,
            std::optional<std::string> quantityName = std::nullopt,
            std::optional<CustomElements> customElements = std::nullopt);

        Unit() = default;
        Unit(const Unit&) = default;
        Unit(Unit&&) noexcept = default;
        ~Unit() = default;

        Unit& operator=(const Unit&) = default;
        Unit& operator=(Unit&&) noexcept = default;

        /**
         * @brief Get unit symbol
         * @return Unit symbol string (ISO 19848 Table 21, UnitSymbol element)
         */
        [[nodiscard]] inline const std::string& unitSymbol() const noexcept;

        /**
         * @brief Get optional quantity name
         * @return Optional quantity name (ISO 19848 Table 21, QuantityName element)
         */
        [[nodiscard]] inline const std::optional<std::string>& quantityName() const noexcept;

        /**
         * @brief Get optional custom elements
         * @return Optional custom elements (extension point)
         */
        [[nodiscard]] inline const std::optional<CustomElements>& customElements() const noexcept;

        /**
         * @brief Set unit symbol
         * @param unitSymbol Unit symbol from ISO 80000/IEC 80000/Table 4 (ISO 19848 Table 21, UnitSymbol element)
         */
        inline void setUnitSymbol(std::string_view unitSymbol);

        /**
         * @brief Set optional quantity name
         * @param quantityName Optional quantity name from ISO 80000/IEC 80000/Table 4 (ISO 19848 Table 21, QuantityName
         * element)
         */
        inline void setQuantityName(std::optional<std::string> quantityName);

        /**
         * @brief Set optional custom elements
         * @param customElements Optional custom elements (extension point)
         */
        inline void setCustomElements(std::optional<CustomElements> customElements);

    private:
        std::string m_unitSymbol;
        std::optional<std::string> m_quantityName;
        std::optional<CustomElements> m_customElements;
    };

    //=====================================================================
    // Header class - Table 10
    //=====================================================================

    /**
     * @brief Package header with metadata (ISO 19848 Header)
     * @details ISO 19848:2024 Table 10: Header structure
     *          - ShipID: Mandatory, String. See 5.1.2
     *          - DataChannelListID: Mandatory, c) ConfigurationReference. See Table 11
     *          - VersionInformation: Optional, d) VersionInformation. Version information of referenced standard for
     *            making DataChannelID. See Table 12 (unlimited)
     *          - Author: Optional, String. Author of Data Channel List
     *          - DateCreated: Optional, DateTime. Date when Package is created
     *          Additionally supports custom headers extension point
     */
    class Header final
    {
    public:
        /**
         * @brief Construct Header with all fields
         * @param shipId Ship identifier (ISO 19848 Table 10, ShipID element)
         * @param dataChannelListId Configuration reference (ISO 19848 Table 10, DataChannelListID element)
         * @param versionInformation Optional version information (ISO 19848 Table 10, VersionInformation element)
         * @param author Optional author of Data Channel List (ISO 19848 Table 10, Author element)
         * @param dateCreated Optional creation timestamp (ISO 19848 Table 10, DateCreated element)
         * @param customHeaders Optional custom headers (extension point)
         */
        DNV_VISTA_SDK_CPP_API Header(
            ShipId shipId,
            ConfigurationReference dataChannelListId,
            std::optional<VersionInformation> versionInformation = std::nullopt,
            std::optional<std::string> author = std::nullopt,
            std::optional<DateTimeOffset> dateCreated = std::nullopt,
            std::optional<CustomHeaders> customHeaders = std::nullopt);

        Header() = delete;
        Header(const Header&) = default;
        Header(Header&&) noexcept = default;
        ~Header() = default;

        /**
         * @brief Copy assignment operator
         * @return Reference to this Header
         */
        Header& operator=(const Header&) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this Header
         */
        Header& operator=(Header&&) noexcept = default;

        /**
         * @brief Get optional author
         * @return Optional author of Data Channel List (ISO 19848 Table 10, Author element)
         */
        [[nodiscard]] inline const std::optional<std::string>& author() const noexcept;

        /**
         * @brief Get ship identifier
         * @return Ship identifier (ISO 19848 Table 10, ShipID element)
         */
        [[nodiscard]] inline const ShipId& shipId() const noexcept;

        /**
         * @brief Get optional creation date
         * @return Optional creation timestamp (ISO 19848 Table 10, DateCreated element)
         */
        [[nodiscard]] inline const std::optional<DateTimeOffset>& dateCreated() const noexcept;

        /**
         * @brief Get data channel list configuration reference
         * @return Configuration reference (ISO 19848 Table 10, DataChannelListID element)
         */
        [[nodiscard]] inline const ConfigurationReference& dataChannelListId() const noexcept;

        /**
         * @brief Get version information
         * @return Optional version information (ISO 19848 Table 10, VersionInformation element)
         */
        [[nodiscard]] inline const std::optional<VersionInformation>& versionInformation() const noexcept;

        /**
         * @brief Get optional custom headers dictionary
         * @return Optional custom headers (extension point)
         */
        [[nodiscard]] inline const std::optional<CustomHeaders>& customHeaders() const noexcept;

        /**
         * @brief Set optional author
         * @param author Optional author of Data Channel List (ISO 19848 Table 10, Author element)
         */
        inline void setAuthor(std::optional<std::string> author);

        /**
         * @brief Set optional creation date
         * @param dateCreated Optional creation timestamp (ISO 19848 Table 10, DateCreated element)
         */
        inline void setDateCreated(std::optional<DateTimeOffset> dateCreated);

        /**
         * @brief Set ship identifier
         * @param shipId Ship identifier (ISO 19848 Table 10, ShipID element)
         */
        inline void setShipId(ShipId shipId);

        /**
         * @brief Set data channel list configuration reference
         * @param dataChannelListId Configuration reference (ISO 19848 Table 10, DataChannelListID element)
         */
        inline void setDataChannelListId(ConfigurationReference dataChannelListId);

        /**
         * @brief Set optional version information
         * @param versionInformation Optional version information (ISO 19848 Table 10, VersionInformation element)
         */
        inline void setVersionInformation(std::optional<VersionInformation> versionInformation);

        /**
         * @brief Set optional custom headers dictionary
         * @param customHeaders Optional custom headers (extension point)
         */
        inline void setCustomHeaders(std::optional<CustomHeaders> customHeaders);

    private:
        ShipId m_shipId;
        ConfigurationReference m_dataChannelListId;
        std::optional<VersionInformation> m_versionInformation;
        std::optional<std::string> m_author;
        std::optional<DateTimeOffset> m_dateCreated;
        std::optional<CustomHeaders> m_customHeaders;
    };

    //=====================================================================
    // Property class - Table 16
    //=====================================================================

    /**
     * @brief Data channel property with type, format, range, and unit (ISO 19848 Property)
     * @details ISO 19848:2024 Table 16: Property structure
     *          - DataChannelType: Mandatory, i) DataChannelType - See Table 17 and 5.2 a)
     *          - Format: Mandatory, j) Format - See Table 18 and 5.2 b)
     *          - Range: Mandatory^a, l) Range - See Table 20 and 5.2 c)
     *          - Unit: Mandatory^a, m) Unit - See Table 21 and 5.2 d)
     *          - QualityCoding: Optional, String - See 5.2 e)
     *          - AlertPriority: Optional, String - See 5.2 f)
     *          - Name: Optional, String - See 5.2 g)
     *          - Remarks: Optional, String - See 5.2 h)
     *          Note: ^a Range and Unit are required only when Data Channel Type is "decimal"
     *          Additionally supports custom properties extension point
     */
    class Property final
    {
    public:
        /**
         * @brief Construct Property with all fields
         * @param dataChannelType Data channel type (ISO 19848 Table 16, DataChannelType element)
         * @param format Format specification (ISO 19848 Table 16, Format element)
         * @param range Range for decimal types (ISO 19848 Table 16, Range element - required when DataChannelType is
         * "decimal")
         * @param unit Unit for decimal types (ISO 19848 Table 16, Unit element - required when DataChannelType is
         * "decimal")
         * @param qualityCoding Optional quality coding (ISO 19848 Table 16, QualityCoding element)
         * @param alertPriority Optional alert priority (ISO 19848 Table 16, AlertPriority element)
         * @param name Optional human-readable name (ISO 19848 Table 16, Name element)
         * @param remarks Optional remarks (ISO 19848 Table 16, Remarks element)
         * @param customProperties Optional custom properties (extension point)
         */
        DNV_VISTA_SDK_CPP_API Property(
            DataChannelType dataChannelType,
            Format format,
            std::optional<Range> range = std::nullopt,
            std::optional<Unit> unit = std::nullopt,
            std::optional<std::string> qualityCoding =
                std::nullopt, // TODO : Validate - 'In the case of “IEC 61162-STATUS”, “A” (Data valid) and
                              // “V” (Data invalid) are used for the data quality'
            std::optional<std::string> alertPriority =
                std::nullopt, // TODO : Validate - 'In the case in which Data Channel Type is “Alert”, Alert Priority
                              // shall be provided. Priority level and criteria for classification shall be in
                              // accordance with IEC 62923-1:2018, 6.2.2.1'
            std::optional<std::string> name = std::nullopt,
            std::optional<std::string> remarks = std::nullopt,
            std::optional<CustomProperties> customProperties = std::nullopt);

        Property() = delete;
        Property(const Property&) = default;
        Property(Property&&) noexcept = default;
        ~Property() = default;

        Property& operator=(const Property&) = default;
        Property& operator=(Property&&) noexcept = default;

        /**
         * @brief Get data channel type
         * @return Data channel type (ISO 19848 Table 16, DataChannelType element)
         */
        [[nodiscard]] inline const DataChannelType& dataChannelType() const noexcept;

        /**
         * @brief Get format specification
         * @return Format specification (ISO 19848 Table 16, Format element)
         */
        [[nodiscard]] inline const Format& format() const noexcept;

        /**
         * @brief Get optional range
         * @return Optional range (ISO 19848 Table 16, Range element - required when DataChannelType is "decimal")
         */
        [[nodiscard]] inline const std::optional<Range>& range() const noexcept;

        /**
         * @brief Get optional unit
         * @return Optional unit (ISO 19848 Table 16, Unit element - required when DataChannelType is "decimal")
         */
        [[nodiscard]] inline const std::optional<Unit>& unit() const noexcept;

        /**
         * @brief Get optional quality coding
         * @return Optional quality coding string (ISO 19848 Table 16, QualityCoding element)
         */
        [[nodiscard]] inline const std::optional<std::string>& qualityCoding() const noexcept;

        /**
         * @brief Get optional alert priority
         * @return Optional alert priority string (ISO 19848 Table 16, AlertPriority element)
         */
        [[nodiscard]] inline const std::optional<std::string>& alertPriority() const noexcept;

        /**
         * @brief Get optional name
         * @return Optional name string (ISO 19848 Table 16, Name element)
         */
        [[nodiscard]] inline const std::optional<std::string>& name() const noexcept;

        /**
         * @brief Get optional remarks
         * @return Optional remarks string (ISO 19848 Table 16, Remarks element)
         */
        [[nodiscard]] inline const std::optional<std::string>& remarks() const noexcept;

        /**
         * @brief Get optional custom properties
         * @return Optional custom properties (extension point)
         */
        [[nodiscard]] inline const std::optional<CustomProperties>& customProperties() const noexcept;

        /**
         * @brief Set data channel type
         * @param dataChannelType Data channel type (ISO 19848 Table 16, DataChannelType element)
         */
        inline void setDataChannelType(DataChannelType dataChannelType);

        /**
         * @brief Set format specification
         * @param format Format specification (ISO 19848 Table 16, Format element)
         */
        inline void setFormat(Format format);

        /**
         * @brief Set optional range
         * @param range Range for decimal types (ISO 19848 Table 16, Range element - required when DataChannelType is
         * "decimal")
         */
        inline void setRange(std::optional<Range> range);

        /**
         * @brief Set optional unit
         * @param unit Unit for decimal types (ISO 19848 Table 16, Unit element - required when DataChannelType is
         * "decimal")
         */
        inline void setUnit(std::optional<Unit> unit);

        /**
         * @brief Set optional quality coding
         * @param qualityCoding Optional quality coding string (ISO 19848 Table 16, QualityCoding element)
         */
        inline void setQualityCoding(std::optional<std::string> qualityCoding);

        /**
         * @brief Set optional alert priority
         * @param alertPriority Optional alert priority string (ISO 19848 Table 16, AlertPriority element)
         */
        inline void setAlertPriority(std::optional<std::string> alertPriority);

        /**
         * @brief Set optional name
         * @param name Optional name string (ISO 19848 Table 16, Name element)
         */
        inline void setName(std::optional<std::string> name);

        /**
         * @brief Set optional remarks
         * @param remarks Optional remarks string (ISO 19848 Table 16, Remarks element)
         */
        inline void setRemarks(std::optional<std::string> remarks);

        /**
         * @brief Set optional custom properties
         * @param customProperties Optional custom properties (extension point)
         */
        inline void setCustomProperties(std::optional<CustomProperties> customProperties);

        /**
         * @brief Validate property consistency
         * @return Validation result indicating success or error messages
         * @details Validates that:
         *          - Range is present for decimal formats
         *          - Unit is present for decimal formats
         *          - AlertPriority is present for alert types
         */
        [[nodiscard]] inline ValidateResult<> validate() const;

    private:
        DataChannelType m_dataChannelType;
        Format m_format;
        std::optional<Range> m_range;
        std::optional<Unit> m_unit;
        std::optional<std::string> m_qualityCoding;
        std::optional<std::string> m_alertPriority;
        std::optional<std::string> m_name;
        std::optional<std::string> m_remarks;
        std::optional<CustomProperties> m_customProperties;
    };

    //=====================================================================
    // DataChannelId class - Table 15
    //=====================================================================

    /**
     * @brief Data channel identifier with LocalId, ShortId, and NameObject (ISO 19848 DataChannelID)
     * @details ISO 19848:2024 Table 15: DataChannelID structure
     *          - LocalID: Mandatory, String - See 5.1.3
     *          - ShortID: Optional, String - See 5.1.4
     *          - NameObject: Optional, n) NameObject - Definition of Local Data Name structure, see Table 22
     */
    class DataChannelId final
    {
    public:
        /**
         * @brief Construct DataChannelId with all fields
         * @param localId Local identifier (ISO 19848 Table 15, LocalID element)
         * @param shortId Optional short identifier (ISO 19848 Table 15, ShortID element)
         * @param nameObject Optional name object (ISO 19848 Table 15, NameObject element)
         */
        DNV_VISTA_SDK_CPP_API explicit DataChannelId(
            LocalId localId,
            std::optional<std::string> shortId = std::nullopt,
            std::optional<NameObject> nameObject = std::nullopt);

        DataChannelId() = delete;
        DataChannelId(const DataChannelId&) = default;
        DataChannelId(DataChannelId&&) noexcept = default;
        ~DataChannelId() = default;

        DataChannelId& operator=(const DataChannelId&) = default;
        DataChannelId& operator=(DataChannelId&&) noexcept = default;

        /**
         * @brief Get local identifier
         * @return Local identifier (ISO 19848 Table 15, LocalID element)
         */
        [[nodiscard]] inline const LocalId& localId() const noexcept;

        /**
         * @brief Get optional short identifier
         * @return Optional short identifier string (ISO 19848 Table 15, ShortID element)
         */
        [[nodiscard]] inline const std::optional<std::string>& shortId() const noexcept;

        /**
         * @brief Get optional name object
         * @return Optional name object (ISO 19848 Table 15, NameObject element)
         */
        [[nodiscard]] inline const std::optional<NameObject>& nameObject() const noexcept;

        /**
         * @brief Set local identifier
         * @param localId Local identifier (ISO 19848 Table 15, LocalID element)
         */
        inline void setLocalId(LocalId localId);

        /**
         * @brief Set optional short identifier
         * @param shortId Optional short identifier string (ISO 19848 Table 15, ShortID element)
         */
        inline void setShortId(std::optional<std::string> shortId);

        /**
         * @brief Set optional name object
         * @param nameObject Optional name object (ISO 19848 Table 15, NameObject element)
         */
        inline void setNameObject(std::optional<NameObject> nameObject);

    private:
        LocalId m_localId;
        std::optional<std::string> m_shortId;
        std::optional<NameObject> m_nameObject;
    };

    //=====================================================================
    // DataChannel class - Table 14
    //=====================================================================

    /**
     * @brief Data channel with identifier and property (ISO 19848 DataChannel)
     * @details ISO 19848:2024 Table 14: DataChannel structure
     *          - DataChannelID: Mandatory, g) DataChannelID - See Table 15
     *          - Property: Mandatory, h) Property - See Table 16
     */
    class DataChannel final
    {
    public:
        /**
         * @brief Construct DataChannel with all fields
         * @param dataChannelId Data channel identifier (ISO 19848 Table 14, DataChannelID element)
         * @param property Property specification (ISO 19848 Table 14, Property element)
         * @throws std::invalid_argument if property validation fails
         */
        DNV_VISTA_SDK_CPP_API DataChannel(DataChannelId dataChannelId, Property property);

        DataChannel() = delete;
        DataChannel(const DataChannel&) = default;
        DataChannel(DataChannel&&) noexcept = default;
        ~DataChannel() = default;

        DataChannel& operator=(const DataChannel&) = default;
        DataChannel& operator=(DataChannel&&) noexcept = default;

        /**
         * @brief Get data channel identifier
         * @return Data channel identifier (ISO 19848 Table 14, DataChannelID element)
         */
        [[nodiscard]] inline const DataChannelId& dataChannelId() const noexcept;

        /**
         * @brief Get property specification
         * @return Property specification (ISO 19848 Table 14, Property element)
         */
        [[nodiscard]] inline const Property& property() const noexcept;

        /**
         * @brief Set data channel identifier
         * @param dataChannelId Data channel identifier (ISO 19848 Table 14, DataChannelID element)
         */
        inline void setDataChannelId(DataChannelId dataChannelId);

        /**
         * @brief Set property specification with validation
         * @param property Property to set (ISO 19848 Table 14, Property element)
         * @throws std::invalid_argument if property validation fails
         */
        inline void setProperty(Property property);

    private:
        DataChannelId m_dataChannelId;
        Property m_property;
    };

    //=====================================================================
    // DataChannelList class - Table 13
    //=====================================================================

    /**
     * @brief Collection of data channels with lookup by LocalId, ShortId, and index (ISO 19848 DataChannelList)
     * @details ISO 19848:2024 Table 13: DataChannelList structure
     *          - DataChannel: Mandatory, f) DataChannel - See Table 14 (unlimited)
     */
    class DataChannelList final
    {
    public:
        /** @brief Iterator type for mutable access to data channels */
        using Iterator = std::vector<DataChannel>::iterator;

        /** @brief Iterator type for read-only access to data channels */
        using ConstIterator = std::vector<DataChannel>::const_iterator;

        /**
         * @brief Construct DataChannelList with initial data channels
         * @param dataChannels Vector of data channels to add (ISO 19848 Table 13, DataChannel element)
         */
        DNV_VISTA_SDK_CPP_API explicit DataChannelList(const std::vector<DataChannel>& dataChannels);

        DataChannelList() = default;
        DataChannelList(const DataChannelList&) = default;
        DataChannelList(DataChannelList&&) noexcept = default;
        ~DataChannelList() = default;

        /**
         * @brief Copy assignment operator
         * @return Reference to this DataChannelList
         */
        DataChannelList& operator=(const DataChannelList&) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this DataChannelList
         */
        DataChannelList& operator=(DataChannelList&&) noexcept = default;

        /**
         * @brief Get read-only vector of all data channels
         * @return Vector of all data channels (ISO 19848 Table 13, DataChannel element)
         */
        [[nodiscard]] inline const std::vector<DataChannel>& dataChannels() const noexcept;

        /**
         * @brief Get number of data channels
         * @return Number of data channels in the collection
         */
        [[nodiscard]] inline std::size_t size() const noexcept;

        /**
         * @brief Get data channel by short ID
         * @param shortId Short identifier to lookup
         * @return Optional reference to data channel if found
         */
        [[nodiscard]] inline std::optional<std::reference_wrapper<const DataChannel>> from(
            std::string_view shortId) const noexcept;

        /**
         * @brief Get data channel by local ID
         * @param localId Local identifier to lookup
         * @return Optional reference to data channel if found
         */
        [[nodiscard]] inline std::optional<std::reference_wrapper<const DataChannel>> from(
            const LocalId& localId) const noexcept;

        /**
         * @brief Add a data channel to the collection
         * @param dataChannel Data channel to add
         * @throws std::invalid_argument if LocalId already exists or ShortId conflicts
         */
        inline void add(DataChannel dataChannel);

        /**
         * @brief Add multiple data channels to the collection
         * @param dataChannels Vector of data channels to add
         * @throws std::invalid_argument if any LocalId already exists or ShortId conflicts
         */
        inline void add(const std::vector<DataChannel>& dataChannels);

        /**
         * @brief Remove a data channel from the collection
         * @param item Data channel to remove
         * @return true if removed, false if not found
         */
        inline bool remove(const DataChannel& item);

        /** @brief Clear all data channels from the collection */
        inline void clear();

        /**
         * @brief Get data channel by short ID
         * @param shortId Short identifier
         * @return Reference to data channel
         * @throws std::out_of_range if not found
         */
        [[nodiscard]] inline const DataChannel& operator[](std::string_view shortId) const;

        /**
         * @brief Get data channel by index
         * @param index Index in list
         * @return Reference to data channel
         * @throws std::out_of_range if index invalid
         */
        [[nodiscard]] inline const DataChannel& operator[](std::size_t index) const;

        /**
         * @brief Get data channel by local ID
         * @param localId Local identifier
         * @return Reference to data channel
         * @throws std::out_of_range if not found
         */
        [[nodiscard]] inline const DataChannel& operator[](const LocalId& localId) const;

        /**
         * @brief Get iterator to beginning
         * @details Iterates DataChannel values in insertion order
         * @return Iterator to first data channel
         */
        [[nodiscard]] inline Iterator begin() noexcept;

        /** @brief Const overload of begin() */
        [[nodiscard]] inline ConstIterator begin() const noexcept;

        /** @brief Get iterator to end */
        [[nodiscard]] inline Iterator end() noexcept;

        /** @brief Const overload of end() */
        [[nodiscard]] inline ConstIterator end() const noexcept;

    private:
        std::vector<DataChannel> m_dataChannels;
    };

    //=====================================================================
    // Package class - Table 9
    //=====================================================================

    /**
     * @brief Package containing header and data channel list (ISO 19848 Package)
     * @details ISO 19848:2024 Table 9: Package structure
     *          - Header: Mandatory, b) Header - See Table 10
     *          - DataChannelList: Mandatory, e) DataChannelList - See Table 13
     */
    class Package final
    {
    public:
        /**
         * @brief Construct Package with all fields
         * @param header Package header (ISO 19848 Table 9, Header element)
         * @param dataChannelList Data channel list (ISO 19848 Table 9, DataChannelList element)
         */
        DNV_VISTA_SDK_CPP_API Package(Header header, DataChannelList dataChannelList);

        Package() = delete;
        Package(const Package&) = default;
        Package(Package&&) noexcept = default;
        ~Package() = default;

        /**
         * @brief Copy assignment operator
         * @return Reference to this Package
         */
        Package& operator=(const Package&) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this Package
         */
        Package& operator=(Package&&) noexcept = default;

        /**
         * @brief Get package header
         * @return Package header (ISO 19848 Table 9, Header element)
         */
        [[nodiscard]] inline const Header& header() const noexcept;

        /**
         * @brief Get data channel list
         * @return Data channel list (ISO 19848 Table 9, DataChannelList element)
         */
        [[nodiscard]] inline const DataChannelList& dataChannelList() const noexcept;

        /**
         * @brief Set package header
         * @param header Package header (ISO 19848 Table 9, Header element)
         */
        inline void setHeader(Header header);

        /**
         * @brief Set data channel list
         * @param dataChannelList Data channel list (ISO 19848 Table 9, DataChannelList element)
         */
        inline void setDataChannelList(DataChannelList dataChannelList);

    private:
        Header m_header;
        DataChannelList m_dataChannelList;
    };

    //=====================================================================
    // DataChannelListPackage class
    //=====================================================================

    /**
     * @brief Top-level data channel list package wrapper (ISO 19848 DataChannelListPackage)
     * @details Wraps a Package (Table 9) for ISO 19848 data channel list transmission
     */
    class DataChannelListPackage final
    {
    public:
        /**
         * @brief Construct DataChannelListPackage with package
         * @param package Package containing header and data channel list (ISO 19848 Table 9)
         */
        DNV_VISTA_SDK_CPP_API explicit DataChannelListPackage(Package package);

        DataChannelListPackage() = delete;
        DataChannelListPackage(const DataChannelListPackage&) = default;
        DataChannelListPackage(DataChannelListPackage&&) noexcept = default;
        ~DataChannelListPackage() = default;

        /**
         * @brief Copy assignment operator
         * @return Reference to this DataChannelListPackage
         */
        DataChannelListPackage& operator=(const DataChannelListPackage&) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this DataChannelListPackage
         */
        DataChannelListPackage& operator=(DataChannelListPackage&&) noexcept = default;

        /**
         * @brief Get package
         * @return Package containing header and data channel list (ISO 19848 Table 9)
         */
        [[nodiscard]] inline const Package& package() const noexcept;

        /**
         * @brief Get data channel list (convenience accessor)
         * @return Data channel list from the wrapped package (ISO 19848 Table 13)
         */
        [[nodiscard]] inline const DataChannelList& dataChannelList() const noexcept;

        /**
         * @brief Set package
         * @param package Package containing header and data channel list (ISO 19848 Table 9)
         */
        inline void setPackage(Package package);

    private:
        Package m_package;
    };
} // namespace dnv::vista::sdk::transport::datachannel

#include "dnv/vista/sdk/detail/transport/datachannel/DataChannel.inl"
