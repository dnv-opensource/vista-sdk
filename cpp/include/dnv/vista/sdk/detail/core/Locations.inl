namespace dnv::vista::sdk
{
    inline Location::Location(std::string_view value) noexcept
        : m_value{ value }
    {}

    inline bool Location::operator==(const Location& other) const noexcept
    {
        return m_value == other.m_value;
    }

    inline const std::string& Location::value() const noexcept
    {
        return m_value;
    }

    inline RelativeLocation::RelativeLocation(
        char code, std::string_view name, const Location& location, std::optional<std::string> definition) noexcept
        : m_code{ code },
          m_name{ name },
          m_location{ location },
          m_definition{ std::move(definition) }
    {}

    inline bool RelativeLocation::operator==(const RelativeLocation& other) const noexcept
    {
        return m_code == other.m_code;
    }

    inline char RelativeLocation::code() const noexcept
    {
        return m_code;
    }

    inline const std::string& RelativeLocation::name() const noexcept
    {
        return m_name;
    }

    inline const std::optional<std::string>& RelativeLocation::definition() const noexcept
    {
        return m_definition;
    }

    inline const Location& RelativeLocation::location() const noexcept
    {
        return m_location;
    }

    inline VisVersion Locations::version() const noexcept
    {
        return m_visVersion;
    }

    inline const std::vector<RelativeLocation>& Locations::relativeLocations() const noexcept
    {
        return m_relativeLocations;
    }

    inline const std::unordered_map<LocationGroup, std::vector<RelativeLocation>>& Locations::groups() const noexcept
    {
        return m_groups;
    }
} // namespace dnv::vista::sdk
