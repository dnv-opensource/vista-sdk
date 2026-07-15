namespace dnv::vista::sdk::transport::datachannel
{
    namespace detail
    {
        inline void throwIfInvalid(const ValidateResult<>& result)
        {
            if (!result)
            {
                std::string errorMsg = "Invalid property - Messages='[";
                for (std::size_t i = 0; i < result.errors().size(); ++i)
                {
                    if (i > 0)
                    {
                        errorMsg += ", ";
                    }
                    errorMsg += result.errors()[i];
                }
                errorMsg += "]'";

                throw std::invalid_argument{ errorMsg };
            }
        }
    } // namespace detail

    inline const std::string& ConfigurationReference::id() const noexcept
    {
        return m_id;
    }

    inline const std::optional<std::string>& ConfigurationReference::version() const noexcept
    {
        return m_version;
    }

    inline const DateTimeOffset& ConfigurationReference::timeStamp() const noexcept
    {
        return m_timeStamp;
    }

    inline void ConfigurationReference::setId(std::string_view id)
    {
        m_id = id;
    }

    inline void ConfigurationReference::setVersion(std::optional<std::string> version)
    {
        m_version = std::move(version);
    }

    inline void ConfigurationReference::setTimeStamp(DateTimeOffset timeStamp)
    {
        m_timeStamp = std::move(timeStamp);
    }

    inline const std::string& VersionInformation::namingRule() const noexcept
    {
        return m_namingRule;
    }

    inline const std::string& VersionInformation::namingSchemeVersion() const noexcept
    {
        return m_namingSchemeVersion;
    }

    inline const std::optional<std::string>& VersionInformation::referenceUrl() const noexcept
    {
        return m_referenceUrl;
    }

    inline void VersionInformation::setNamingRule(std::string_view namingRule)
    {
        m_namingRule = namingRule;
    }

    inline void VersionInformation::setNamingSchemeVersion(std::string_view namingSchemeVersion)
    {
        m_namingSchemeVersion = namingSchemeVersion;
    }

    inline void VersionInformation::setReferenceUrl(std::optional<std::string> referenceUrl)
    {
        m_referenceUrl = std::move(referenceUrl);
    }

    inline double Range::low() const noexcept
    {
        return m_low;
    }

    inline double Range::high() const noexcept
    {
        return m_high;
    }

    inline void Range::setLow(double low)
    {
        if (low > m_high)
        {
            throw std::invalid_argument{ "Low value must be less than high value" };
        }

        m_low = low;
    }

    inline void Range::setHigh(double high)
    {
        if (high < m_low)
        {
            throw std::invalid_argument{ "High value must be greater than low value" };
        }

        m_high = high;
    }

    inline const std::optional<std::vector<std::string>>& Restriction::enumeration() const noexcept
    {
        return m_enumeration;
    }

    inline const std::optional<std::uint32_t>& Restriction::fractionDigits() const noexcept
    {
        return m_fractionDigits;
    }

    inline const std::optional<std::uint32_t>& Restriction::length() const noexcept
    {
        return m_length;
    }

    inline const std::optional<double>& Restriction::maxExclusive() const noexcept
    {
        return m_maxExclusive;
    }

    inline const std::optional<double>& Restriction::maxInclusive() const noexcept
    {
        return m_maxInclusive;
    }

    inline const std::optional<std::uint32_t>& Restriction::maxLength() const noexcept
    {
        return m_maxLength;
    }

    inline const std::optional<double>& Restriction::minExclusive() const noexcept
    {
        return m_minExclusive;
    }

    inline const std::optional<double>& Restriction::minInclusive() const noexcept
    {
        return m_minInclusive;
    }

    inline const std::optional<std::uint32_t>& Restriction::minLength() const noexcept
    {
        return m_minLength;
    }

    inline const std::optional<std::string>& Restriction::pattern() const noexcept
    {
        return m_pattern;
    }

    inline const std::optional<std::uint32_t>& Restriction::totalDigits() const noexcept
    {
        return m_totalDigits;
    }

    inline const std::optional<Restriction::WhiteSpace>& Restriction::whiteSpace() const noexcept
    {
        return m_whiteSpace;
    }

    inline void Restriction::setEnumeration(std::optional<std::vector<std::string>> enumeration)
    {
        m_enumeration = std::move(enumeration);
    }

    inline void Restriction::setFractionDigits(std::optional<std::uint32_t> fractionDigits)
    {
        m_fractionDigits = fractionDigits;
    }

    inline void Restriction::setLength(std::optional<std::uint32_t> length)
    {
        m_length = length;
    }

    inline void Restriction::setMaxExclusive(std::optional<double> maxExclusive)
    {
        m_maxExclusive = maxExclusive;
    }

    inline void Restriction::setMaxInclusive(std::optional<double> maxInclusive)
    {
        m_maxInclusive = maxInclusive;
    }

    inline void Restriction::setMaxLength(std::optional<std::uint32_t> maxLength)
    {
        m_maxLength = maxLength;
    }

    inline void Restriction::setMinExclusive(std::optional<double> minExclusive)
    {
        m_minExclusive = minExclusive;
    }

    inline void Restriction::setMinInclusive(std::optional<double> minInclusive)
    {
        m_minInclusive = minInclusive;
    }

    inline void Restriction::setMinLength(std::optional<std::uint32_t> minLength)
    {
        m_minLength = minLength;
    }

    inline void Restriction::setPattern(std::optional<std::string> pattern)
    {
        if (pattern)
        {
            m_cachedPattern.emplace(*pattern);
        }
        else
        {
            m_cachedPattern.reset();
        }
        m_pattern = std::move(pattern);
    }

    inline void Restriction::setTotalDigits(std::optional<std::uint32_t> totalDigits)
    {
        if (totalDigits.has_value() && totalDigits.value() == 0)
        {
            throw std::invalid_argument{ "TotalDigits must be greater than zero" };
        }

        m_totalDigits = totalDigits;
    }

    inline void Restriction::setWhiteSpace(std::optional<WhiteSpace> whiteSpace)
    {
        m_whiteSpace = whiteSpace;
    }

    inline const std::string& Format::type() const noexcept
    {
        return m_type;
    }

    inline const std::optional<Restriction>& Format::restriction() const noexcept
    {
        return m_restriction;
    }

    inline void Format::setRestriction(std::optional<Restriction> restriction)
    {
        m_restriction = std::move(restriction);
    }

    inline const std::string& DataChannelType::type() const noexcept
    {
        return m_type;
    }

    inline const std::optional<double>& DataChannelType::updateCycle() const noexcept
    {
        return m_updateCycle;
    }

    inline const std::optional<double>& DataChannelType::calculationPeriod() const noexcept
    {
        return m_calculationPeriod;
    }

    inline bool DataChannelType::isAlert() const noexcept
    {
        return m_type == "Alert";
    }

    inline void DataChannelType::setUpdateCycle(std::optional<double> updateCycle)
    {
        if (updateCycle.has_value() && updateCycle.value() < 0.0)
        {
            throw std::invalid_argument{ "UpdateCycle must be non-negative" };
        }

        m_updateCycle = updateCycle;
    }

    inline void DataChannelType::setCalculationPeriod(std::optional<double> calculationPeriod)
    {
        if (calculationPeriod.has_value() && calculationPeriod.value() < 0.0)
        {
            throw std::invalid_argument{ "CalculationPeriod must be non-negative" };
        }

        m_calculationPeriod = calculationPeriod;
    }

    inline const std::string& NameObject::namingRule() const noexcept
    {
        return m_namingRule;
    }

    inline const std::optional<CustomNameObjects>& NameObject::customNameObjects() const noexcept
    {
        return m_customNameObjects;
    }

    inline void NameObject::setNamingRule(std::string_view namingRule)
    {
        m_namingRule = namingRule;
    }

    inline void NameObject::setCustomNameObjects(std::optional<CustomNameObjects> customNameObjects)
    {
        m_customNameObjects = std::move(customNameObjects);
    }

    inline const std::string& Unit::unitSymbol() const noexcept
    {
        return m_unitSymbol;
    }

    inline const std::optional<std::string>& Unit::quantityName() const noexcept
    {
        return m_quantityName;
    }

    inline const std::optional<CustomElements>& Unit::customElements() const noexcept
    {
        return m_customElements;
    }

    inline void Unit::setUnitSymbol(std::string_view unitSymbol)
    {
        m_unitSymbol = unitSymbol;
    }

    inline void Unit::setQuantityName(std::optional<std::string> quantityName)
    {
        m_quantityName = std::move(quantityName);
    }

    inline void Unit::setCustomElements(std::optional<CustomElements> customElements)
    {
        m_customElements = std::move(customElements);
    }

    inline const std::optional<std::string>& Header::author() const noexcept
    {
        return m_author;
    }

    inline const ShipId& Header::shipId() const noexcept
    {
        return m_shipId;
    }

    inline const std::optional<DateTimeOffset>& Header::dateCreated() const noexcept
    {
        return m_dateCreated;
    }

    inline const ConfigurationReference& Header::dataChannelListId() const noexcept
    {
        return m_dataChannelListId;
    }

    inline const std::optional<VersionInformation>& Header::versionInformation() const noexcept
    {
        return m_versionInformation;
    }

    inline const std::optional<CustomHeaders>& Header::customHeaders() const noexcept
    {
        return m_customHeaders;
    }

    inline void Header::setAuthor(std::optional<std::string> author)
    {
        m_author = std::move(author);
    }

    inline void Header::setDateCreated(std::optional<DateTimeOffset> dateCreated)
    {
        m_dateCreated = std::move(dateCreated);
    }

    inline void Header::setShipId(ShipId shipId)
    {
        m_shipId = std::move(shipId);
    }

    inline void Header::setDataChannelListId(ConfigurationReference dataChannelListId)
    {
        m_dataChannelListId = std::move(dataChannelListId);
    }

    inline void Header::setVersionInformation(std::optional<VersionInformation> versionInformation)
    {
        m_versionInformation = std::move(versionInformation);
    }

    inline void Header::setCustomHeaders(std::optional<CustomHeaders> customHeaders)
    {
        m_customHeaders = std::move(customHeaders);
    }

    inline const DataChannelType& Property::dataChannelType() const noexcept
    {
        return m_dataChannelType;
    }

    inline const Format& Property::format() const noexcept
    {
        return m_format;
    }

    inline const std::optional<Range>& Property::range() const noexcept
    {
        return m_range;
    }

    inline const std::optional<Unit>& Property::unit() const noexcept
    {
        return m_unit;
    }

    inline const std::optional<std::string>& Property::qualityCoding() const noexcept
    {
        return m_qualityCoding;
    }

    inline const std::optional<std::string>& Property::alertPriority() const noexcept
    {
        return m_alertPriority;
    }

    inline const std::optional<std::string>& Property::name() const noexcept
    {
        return m_name;
    }

    inline const std::optional<std::string>& Property::remarks() const noexcept
    {
        return m_remarks;
    }

    inline const std::optional<CustomProperties>& Property::customProperties() const noexcept
    {
        return m_customProperties;
    }

    inline void Property::setDataChannelType(DataChannelType dataChannelType)
    {
        m_dataChannelType = std::move(dataChannelType);
    }

    inline void Property::setFormat(Format format)
    {
        m_format = std::move(format);
    }

    inline void Property::setRange(std::optional<Range> range)
    {
        m_range = std::move(range);
    }

    inline void Property::setUnit(std::optional<Unit> unit)
    {
        m_unit = std::move(unit);
    }

    inline void Property::setQualityCoding(std::optional<std::string> qualityCoding)
    {
        m_qualityCoding = std::move(qualityCoding);
    }

    inline void Property::setAlertPriority(std::optional<std::string> alertPriority)
    {
        m_alertPriority = std::move(alertPriority);
    }

    inline void Property::setName(std::optional<std::string> name)
    {
        m_name = std::move(name);
    }

    inline void Property::setRemarks(std::optional<std::string> remarks)
    {
        m_remarks = std::move(remarks);
    }

    inline void Property::setCustomProperties(std::optional<CustomProperties> customProperties)
    {
        m_customProperties = std::move(customProperties);
    }

    inline ValidateResult<> Property::validate() const
    {
        std::vector<std::string> errors;

        if (m_format.type() == "Decimal" && !m_range.has_value())
        {
            errors.push_back("Range is required for Decimal format type");
        }
        if (m_format.type() == "Decimal" && !m_unit.has_value())
        {
            errors.push_back("Unit is required for Decimal format type");
        }
        if (m_dataChannelType.isAlert() && !m_alertPriority.has_value())
        {
            errors.push_back("AlertPriority is required for Alert DataChannelType");
        }
        if (!errors.empty())
        {
            return ValidateResult<>::invalid(std::move(errors));
        }

        return ValidateResult<>::ok();
    }

    inline const LocalId& DataChannelId::localId() const noexcept
    {
        return m_localId;
    }

    inline const std::optional<std::string>& DataChannelId::shortId() const noexcept
    {
        return m_shortId;
    }

    inline const std::optional<NameObject>& DataChannelId::nameObject() const noexcept
    {
        return m_nameObject;
    }

    inline void DataChannelId::setLocalId(LocalId localId)
    {
        m_localId = std::move(localId);
    }

    inline void DataChannelId::setShortId(std::optional<std::string> shortId)
    {
        m_shortId = std::move(shortId);
    }

    inline void DataChannelId::setNameObject(std::optional<NameObject> nameObject)
    {
        m_nameObject = std::move(nameObject);
    }

    inline const DataChannelId& DataChannel::dataChannelId() const noexcept
    {
        return m_dataChannelId;
    }

    inline const Property& DataChannel::property() const noexcept
    {
        return m_property;
    }

    inline void DataChannel::setDataChannelId(DataChannelId dataChannelId)
    {
        m_dataChannelId = std::move(dataChannelId);
    }

    inline void DataChannel::setProperty(Property property)
    {
        detail::throwIfInvalid(property.validate());

        m_property = std::move(property);
    }

    inline const std::vector<DataChannel>& DataChannelList::dataChannels() const noexcept
    {
        return m_dataChannels;
    }

    inline std::size_t DataChannelList::size() const noexcept
    {
        return m_dataChannels.size();
    }

    inline std::optional<std::reference_wrapper<const DataChannel>> DataChannelList::from(
        std::string_view shortId) const noexcept
    {
        auto it = std::find_if(m_dataChannels.begin(), m_dataChannels.end(), [&shortId](const DataChannel& dc) {
            return dc.dataChannelId().shortId().has_value() && dc.dataChannelId().shortId().value() == shortId;
        });

        if (it == m_dataChannels.end())
        {
            return std::nullopt;
        }

        return std::cref(*it);
    }

    inline std::optional<std::reference_wrapper<const DataChannel>> DataChannelList::from(
        const LocalId& localId) const noexcept
    {
        auto it = std::find_if(m_dataChannels.begin(), m_dataChannels.end(), [&localId](const DataChannel& dc) {
            return dc.dataChannelId().localId() == localId;
        });

        if (it == m_dataChannels.end())
        {
            return std::nullopt;
        }

        return std::cref(*it);
    }

    inline void DataChannelList::add(DataChannel dataChannel)
    {
        add(std::vector<DataChannel>{ std::move(dataChannel) });
    }

    inline void DataChannelList::add(const std::vector<DataChannel>& dataChannels)
    {
        for (const auto& dc : dataChannels)
        {
            // Check LocalId uniqueness against existing items
            auto localIdExists =
                std::any_of(m_dataChannels.begin(), m_dataChannels.end(), [&dc](const DataChannel& existing) {
                    return existing.dataChannelId().localId() == dc.dataChannelId().localId();
                });
            if (localIdExists)
            {
                throw std::invalid_argument{ "DataChannel with LocalId already exists" };
            }

            // Check ShortId uniqueness if present
            if (dc.dataChannelId().shortId().has_value())
            {
                const auto& shortId = dc.dataChannelId().shortId().value();
                auto shortIdExists =
                    std::any_of(m_dataChannels.begin(), m_dataChannels.end(), [&shortId](const DataChannel& existing) {
                        return existing.dataChannelId().shortId().has_value() &&
                               existing.dataChannelId().shortId().value() == shortId;
                    });
                if (shortIdExists)
                {
                    throw std::invalid_argument{ "DataChannel with ShortId already exists" };
                }
            }

            m_dataChannels.push_back(dc);
        }
    }

    inline bool DataChannelList::remove(const DataChannel& item)
    {
        auto it = std::find_if(m_dataChannels.begin(), m_dataChannels.end(), [&item](const DataChannel& dc) {
            return dc.dataChannelId().localId() == item.dataChannelId().localId();
        });

        if (it != m_dataChannels.end())
        {
            m_dataChannels.erase(it);
            return true;
        }
        return false;
    }

    inline void DataChannelList::clear()
    {
        m_dataChannels.clear();
    }

    inline const DataChannel& DataChannelList::operator[](std::string_view shortId) const
    {
        auto it = std::find_if(m_dataChannels.begin(), m_dataChannels.end(), [&shortId](const DataChannel& dc) {
            return dc.dataChannelId().shortId().has_value() && dc.dataChannelId().shortId().value() == shortId;
        });

        if (it == m_dataChannels.end())
        {
            throw std::out_of_range{ "DataChannel with ShortId not found" };
        }

        return *it;
    }

    inline const DataChannel& DataChannelList::operator[](std::size_t index) const
    {
        if (index >= m_dataChannels.size())
        {
            throw std::out_of_range{ "DataChannel index out of range" };
        }

        return m_dataChannels[index];
    }

    inline const DataChannel& DataChannelList::operator[](const LocalId& localId) const
    {
        auto it = std::find_if(m_dataChannels.begin(), m_dataChannels.end(), [&localId](const DataChannel& dc) {
            return dc.dataChannelId().localId() == localId;
        });

        if (it == m_dataChannels.end())
        {
            throw std::out_of_range{ "DataChannel with LocalId not found" };
        }

        return *it;
    }

    inline DataChannelList::Iterator DataChannelList::begin() noexcept
    {
        return m_dataChannels.begin();
    }

    inline DataChannelList::ConstIterator DataChannelList::begin() const noexcept
    {
        return m_dataChannels.begin();
    }

    inline DataChannelList::Iterator DataChannelList::end() noexcept
    {
        return m_dataChannels.end();
    }

    inline DataChannelList::ConstIterator DataChannelList::end() const noexcept
    {
        return m_dataChannels.end();
    }

    inline const Header& Package::header() const noexcept
    {
        return m_header;
    }

    inline const DataChannelList& Package::dataChannelList() const noexcept
    {
        return m_dataChannelList;
    }

    inline void Package::setHeader(Header header)
    {
        m_header = std::move(header);
    }

    inline void Package::setDataChannelList(DataChannelList dataChannelList)
    {
        m_dataChannelList = std::move(dataChannelList);
    }

    inline const Package& DataChannelListPackage::package() const noexcept
    {
        return m_package;
    }

    inline const DataChannelList& DataChannelListPackage::dataChannelList() const noexcept
    {
        return m_package.dataChannelList();
    }

    inline void DataChannelListPackage::setPackage(Package package)
    {
        m_package = std::move(package);
    }
} // namespace dnv::vista::sdk::transport::datachannel
