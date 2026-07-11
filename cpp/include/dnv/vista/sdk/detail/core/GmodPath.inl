#include <stdexcept>

namespace dnv::vista::sdk
{
    inline bool GmodPath::operator==(const GmodPath& other) const noexcept
    {
        if (m_parents.size() != other.m_parents.size())
        {
            return false;
        }

        for (size_t i = 0; i < m_parents.size(); ++i)
        {
            if (m_parents[i] != other.m_parents[i])
            {
                return false;
            }
        }

        return m_node == other.m_node;
    }

    inline const GmodNode& GmodPath::operator[](size_t index) const
    {
        if (index > m_parents.size())
        {
            throw std::out_of_range{ "Index out of range for GmodPath indexer" };
        }

        return index < m_parents.size() ? m_parents[index] : m_node;
    }

    inline GmodNode& GmodPath::operator[](size_t index)
    {
        if (index > m_parents.size())
        {
            throw std::out_of_range{ "Index out of range for GmodPath indexer" };
        }

        return index < m_parents.size() ? m_parents[index] : m_node;
    }

    inline VisVersion GmodPath::version() const noexcept
    {
        return m_visVersion;
    }

    inline const GmodNode& GmodPath::node() const noexcept
    {
        return m_node;
    }

    inline const std::vector<GmodNode>& GmodPath::parents() const noexcept
    {
        return m_parents;
    }

    inline size_t GmodPath::length() const noexcept
    {
        return m_parents.size() + 1;
    }

    inline bool GmodPath::isValid(std::span<const GmodNode> parents, const GmodNode& node)
    {
        if (parents.empty())
        {
            return false;
        }

        if (!parents[0].isRoot())
        {
            return false;
        }

        for (size_t i = 0; i < parents.size(); ++i)
        {
            const auto& parent = parents[i];
            const auto nextIndex = i + 1;
            const auto& child = nextIndex < parents.size() ? parents[nextIndex] : node;

            if (!parent.isChild(child))
            {
                return false;
            }
        }

        return true;
    }

    inline bool GmodPath::isMappable() const noexcept
    {
        return m_node.isMappable();
    }

    inline std::string GmodPath::toString() const
    {
        std::string out;
        this->toString(out);
        return out;
    }

    inline void GmodPath::toString(std::string& out, char separator) const
    {
        for (const auto& parent : m_parents)
        {
            if (!parent.isLeafNode())
            {
                continue;
            }

            parent.toString(out);
            out += separator;
        }

        m_node.toString(out);
    }

    inline void GmodPath::toFullPathString(std::string& out) const
    {
        for (const auto& [depth, pathNode] : fullPath())
        {
            pathNode.toString(out);

            if (depth != (length() - 1))
            {
                out += '/';
            }
        }
    }

    inline std::string GmodPath::toStringDump() const
    {
        std::string out;
        toStringDump(out);
        return out;
    }

    inline void GmodPath::toStringDump(std::string& out) const
    {
        for (const auto& [depth, pathNode] : fullPath())
        {
            if (depth == 0)
            {
                continue;
            }

            if (depth != 1)
            {
                out += " | ";
            }

            out += pathNode.code();

            const auto& name = pathNode.metadata().name();
            if (!name.empty())
            {
                out += "/N:";
                out += name;
            }

            const auto& commonName = pathNode.metadata().commonName();
            if (commonName.has_value() && !commonName->empty())
            {
                out += "/CN:";
                out += *commonName;
            }

            const auto normalAssignmentName = this->normalAssignmentName(depth);
            if (normalAssignmentName.has_value() && !normalAssignmentName->empty())
            {
                out += "/NAN:";
                out += *normalAssignmentName;
            }
        }
    }

    inline GmodPath::FullPathRange GmodPath::fullPath() const
    {
        return FullPathRange(this, 0);
    }

    inline GmodPath::FullPathRange GmodPath::fullPathFrom(size_t fromDepth) const
    {
        if (fromDepth > m_parents.size())
        {
            throw std::out_of_range{ "fromDepth is out of range" };
        }
        return FullPathRange{ this, fromDepth };
    }

    inline const std::vector<int>& GmodIndividualizableSet::nodeIndices() const noexcept
    {
        return m_nodeIndices;
    }

    inline std::optional<Location> GmodIndividualizableSet::location() const
    {
        if (!m_path.has_value())
        {
            throw std::runtime_error{ "GmodIndividualizableSet has already been built" };
        }

        if (m_nodeIndices.empty())
        {
            return std::nullopt;
        }

        return (*m_path)[m_nodeIndices[0]].location();
    }
} // namespace dnv::vista::sdk
