#include "dnv/vista/sdk/utils/StringUtils.h"

namespace dnv::vista::sdk
{
    inline bool GmodNodeMetadata::operator==(const GmodNodeMetadata& other) const noexcept
    {
        return m_category == other.m_category && m_type == other.m_type && m_name == other.m_name &&
               m_commonName == other.m_commonName && m_definition == other.m_definition &&
               m_commonDefinition == other.m_commonDefinition && m_installSubstructure == other.m_installSubstructure &&
               m_normalAssignmentNames == other.m_normalAssignmentNames;
    }

    inline std::string_view GmodNodeMetadata::category() const noexcept
    {
        return m_category;
    }

    inline std::string_view GmodNodeMetadata::type() const noexcept
    {
        return m_type;
    }

    inline std::string_view GmodNodeMetadata::fullType() const noexcept
    {
        return m_fullType;
    }

    inline std::string_view GmodNodeMetadata::name() const noexcept
    {
        return m_name;
    }

    inline const std::optional<std::string>& GmodNodeMetadata::commonName() const noexcept
    {
        return m_commonName;
    }

    inline const std::optional<std::string>& GmodNodeMetadata::definition() const noexcept
    {
        return m_definition;
    }

    inline const std::optional<std::string>& GmodNodeMetadata::commonDefinition() const noexcept
    {
        return m_commonDefinition;
    }

    inline std::optional<bool> GmodNodeMetadata::installSubstructure() const noexcept
    {
        return m_installSubstructure;
    }

    inline const std::unordered_map<std::string, std::string, StringHash, std::equal_to<>>& GmodNodeMetadata::
        normalAssignmentNames() const noexcept
    {
        return m_normalAssignmentNames;
    }

    inline bool GmodNode::operator==(const GmodNode& other) const noexcept
    {
        if (m_code != other.m_code)
        {
            return false;
        }

        if (!m_location.has_value() && !other.m_location.has_value())
        {
            return true;
        }

        return m_location == other.m_location;
    }

    inline VisVersion GmodNode::version() const noexcept
    {
        return m_visVersion;
    }

    inline std::string_view GmodNode::code() const noexcept
    {
        return m_code;
    }

    inline const std::optional<Location>& GmodNode::location() const noexcept
    {
        return m_location;
    }

    inline const GmodNodeMetadata& GmodNode::metadata() const noexcept
    {
        return *m_metadata;
    }

    inline const StackVector<GmodNode*, 16>& GmodNode::children() const noexcept
    {
        return m_children;
    }

    inline const StackVector<GmodNode*, 16>& GmodNode::parents() const noexcept
    {
        return m_parents;
    }

    inline std::optional<const GmodNode*> GmodNode::productType() const noexcept
    {
        if (m_children.size() != 1)
        {
            return std::nullopt;
        }
        if (!string::contains(m_metadata->category(), "FUNCTION"))
        {
            return std::nullopt;
        }
        const GmodNode* child = m_children[0];
        if (!child)
        {
            return std::nullopt;
        }
        if (child->m_metadata->category() != "PRODUCT")
        {
            return std::nullopt;
        }
        if (child->m_metadata->type() != "TYPE")
        {
            return std::nullopt;
        }

        return child;
    }

    inline std::optional<const GmodNode*> GmodNode::productSelection() const noexcept
    {
        if (m_children.size() != 1)
        {
            return std::nullopt;
        }
        if (!string::contains(m_metadata->category(), "FUNCTION"))
        {
            return std::nullopt;
        }
        const GmodNode* child = m_children[0];
        if (!child)
        {
            return std::nullopt;
        }
        if (!string::contains(child->m_metadata->category(), "PRODUCT"))
        {
            return std::nullopt;
        }
        if (child->m_metadata->type() != "SELECTION")
        {
            return std::nullopt;
        }

        return child;
    }

    inline bool GmodNode::isFunctionComposition() const noexcept
    {
        return (m_metadata->category() == "ASSET FUNCTION" || m_metadata->category() == "PRODUCT FUNCTION") &&
               m_metadata->type() == "COMPOSITION";
    }

    inline bool GmodNode::isMappable() const noexcept
    {
        auto productTypeOpt = productType();
        if (productTypeOpt.has_value())
        {
            return false;
        }
        auto productSelectionOpt = productSelection();
        if (productSelectionOpt.has_value())
        {
            return false;
        }
        if (string::contains(m_metadata->category(), "PRODUCT") && m_metadata->type() == "SELECTION")
        {
            return false;
        }
        if (m_metadata->category() == "ASSET")
        {
            return false;
        }
        if (m_code.empty())
        {
            return false;
        }

        char lastChar = m_code.back();
        if (lastChar == 'a' || lastChar == 's')
        {
            return false;
        }

        return true;
    }

    inline bool GmodNode::isProductSelection() const noexcept
    {
        return m_metadata->category() == "PRODUCT" && m_metadata->type() == "SELECTION";
    }

    inline bool GmodNode::isProductType() const noexcept
    {
        return m_metadata->category() == "PRODUCT" && m_metadata->type() == "TYPE";
    }

    inline bool GmodNode::isAsset() const noexcept
    {
        return m_metadata->category() == "ASSET";
    }

    inline bool GmodNode::isLeafNode() const noexcept
    {
        return m_metadata->fullType() == "ASSET FUNCTION LEAF" || m_metadata->fullType() == "PRODUCT FUNCTION LEAF";
    }

    inline bool GmodNode::isFunctionNode() const noexcept
    {
        return m_metadata->category() != "PRODUCT" && m_metadata->category() != "ASSET";
    }

    inline bool GmodNode::isAssetFunctionNode() const noexcept
    {
        return m_metadata->category() == "ASSET FUNCTION";
    }

    inline bool GmodNode::isRoot() const noexcept
    {
        return m_code == "VE";
    }

    inline bool GmodNode::isChild(const GmodNode& node) const noexcept
    {
        return isChild(node.m_code);
    }

    inline bool GmodNode::isChild(std::string_view code) const noexcept
    {
        if (!m_childrenSet)
        {
            return false;
        }
        return m_childrenSet->contains(code);
    }

    inline std::string GmodNode::toString() const noexcept
    {
        if (!m_location.has_value())
        {
            return m_code;
        }

        StringBuilder sb{ m_code.size() + 1 + m_location->value().size() };
        toString(sb);
        return sb.toString();
    }

    inline void GmodNode::toString(StringBuilder& builder) const noexcept
    {
        builder += m_code;

        if (m_location.has_value())
        {
            builder += '-';
            builder += m_location->value();
        }
    }

    inline bool GmodNode::isIndividualizable(bool isTargetNode, bool isInSet) const noexcept
    {
        if (m_metadata->type() == "GROUP")
        {
            return false;
        }
        if (m_metadata->type() == "SELECTION")
        {
            return false;
        }
        if (isProductType())
        {
            return false;
        }
        if (m_metadata->category() == "ASSET" && m_metadata->type() == "TYPE")
        {
            return false;
        }
        if (isFunctionComposition())
        {
            if (m_code.empty())
            {
                return false;
            }

            return m_code.back() == 'i' || isInSet || isTargetNode;
        }

        return true;
    }

    inline void GmodNode::setLocation(const Location& location) noexcept
    {
        m_location = location;
    }

    inline void GmodNode::setLocation(Location&& location) noexcept
    {
        m_location = std::move(location);
    }

    inline GmodNode GmodNode::withLocation(const Location& location) const noexcept
    {
        GmodNode result = *this;
        result.m_location = location;

        return result;
    }

    inline GmodNode GmodNode::withoutLocation() const
    {
        if (!m_location.has_value())
        {
            return *this;
        }

        GmodNode result = *this;
        result.m_location = std::nullopt;

        return result;
    }
} // namespace dnv::vista::sdk
