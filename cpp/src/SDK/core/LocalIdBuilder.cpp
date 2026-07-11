#include "dnv/vista/sdk/core/LocalIdBuilder.h"

#include "internal/parsing/LocalIdParsingErrorBuilder.h"
#include "dnv/vista/sdk/detail/core/ISO19848AnnexC.h"
#include "dnv/vista/sdk/core/LocalId.h"
#include "dnv/vista/sdk/core/ParsingErrors.h"
#include "dnv/vista/sdk/core/VIS.h"

#include <algorithm>
#include <cctype>

namespace dnv::vista::sdk
{
    namespace
    {
        using namespace dnv::vista::sdk;
        using namespace dnv::vista::sdk::internal;

        /**
         * @brief Normalize a common name for URI-safe verbose mode output
         * @param name The common name to normalize
         * @param builder String to append the normalized name to
         */
        inline void appendNormalizedCommonName(std::string_view name, std::string& out)
        {
            auto collapsed = string::collapseWhitespace(name);

            char prevChar = '\0';
            for (char ch : collapsed)
            {
                char current = (ch == ' ') ? '.' : string::toLower(ch);

                if (!string::isUriUnreserved(current) && current != '.')
                {
                    continue;
                }

                if (current == '.' && prevChar == '.')
                {
                    continue;
                }

                out += current;
                prevChar = current;
            }
        }

        // Returns the index of the '/' preceding the next state keyword (sec, meta, ~),
        // and the index just after that keyword's '/'. Returns {npos, npos} if not found.
        struct NextStateIndexes
        {
            size_t nextIndex; // index of the '/' before the keyword
            size_t endIndex;  // index just after keyword + '/'
        };

        NextStateIndexes getNextStateIndexes(std::string_view str, LocalIdParsingState state)
        {
            constexpr size_t npos = std::string_view::npos;

            auto findKw = [&](std::string_view kw) -> size_t {
                size_t pos = 0;
                while (pos < str.size())
                {
                    auto slash = str.find('/', pos);
                    if (slash == npos)
                    {
                        break;
                    }
                    auto after = str.substr(slash + 1);
                    if (after.starts_with(kw))
                    {
                        auto end = slash + 1 + kw.size();
                        if (end >= str.size() || str[end] == '/')
                        {
                            return slash;
                        }
                    }
                    pos = slash + 1;
                }
                return npos;
            };

            size_t tildeIdx = str.find('~');
            size_t endTildeSlash = tildeIdx != npos ? str.find('/', tildeIdx) : npos;
            size_t endTilde = endTildeSlash != npos ? endTildeSlash + 1 : npos;

            size_t metaIdx = findKw("meta");
            size_t endMeta = metaIdx != npos ? metaIdx + 6 : npos; // '/meta/'

            if (state == LocalIdParsingState::PrimaryItem)
            {
                size_t secIdx = findKw("sec");
                size_t endSec = secIdx != npos ? secIdx + 5 : npos; // '/sec/'

                if (secIdx != npos)
                {
                    return { secIdx, endSec };
                }

                if (tildeIdx != npos && (metaIdx == npos || tildeIdx < metaIdx))
                {
                    return { tildeIdx, endTilde };
                }

                return { metaIdx, endMeta };
            }
            else // SecondaryItem
            {
                if (tildeIdx != npos && (metaIdx == npos || tildeIdx < metaIdx))
                {
                    return { tildeIdx, endTilde };
                }

                return { metaIdx, endMeta };
            }
        }

    } // namespace

    LocalId LocalIdBuilder::build() const
    {
        if (!isValid())
        {
            throw std::invalid_argument{
                "Cannot build LocalId: VisVersion, primary item, and at least one metadata tag are required"
            };
        }

        return LocalId{ *this };
    }

    void LocalIdBuilder::toString(std::string& out) const
    {
        out += '/';
        out += internal::iso19848::annexC::VersionedNamingRule;

        if (m_visVersion.has_value())
        {
            out += "/vis-";
            out += VisVersions::toString(*m_visVersion);
        }

        if (m_primaryItem.has_value())
        {
            out += '/';
            out += m_primaryItem->toString();
            out += '/';
        }

        if (m_secondaryItem.has_value())
        {
            out += "sec/";
            out += m_secondaryItem->toString();
            out += '/';
        }

        if (m_verboseMode)
        {
            if (m_primaryItem.has_value())
            {
                auto commonNamesVec = m_primaryItem->commonNames();
                for (const auto& [depth, name] : commonNamesVec)
                {
                    out += '~';
                    appendNormalizedCommonName(name, out);

                    const auto& node = (*m_primaryItem)[depth];
                    if (node.location().has_value())
                    {
                        out += '.';
                        out += node.location()->value();
                    }
                    out += '/';
                }
            }

            if (m_secondaryItem.has_value())
            {
                auto commonNamesVec = m_secondaryItem->commonNames();
                const char* prefix = "~for.";
                for (const auto& [depth, name] : commonNamesVec)
                {
                    out += prefix;
                    if (std::string_view(prefix) != "~")
                    {
                        prefix = "~";
                    }

                    appendNormalizedCommonName(name, out);

                    const auto& node = (*m_secondaryItem)[depth];
                    if (node.location().has_value())
                    {
                        out += '.';
                        out += node.location()->value();
                    }
                    out += '/';
                }
            }
        }

        out += "meta/";
        if (m_quantity.has_value())
        {
            m_quantity->toString(out);
        }
        if (m_content.has_value())
        {
            m_content->toString(out);
        }
        if (m_calculation.has_value())
        {
            m_calculation->toString(out);
        }
        if (m_state.has_value())
        {
            m_state->toString(out);
        }
        if (m_command.has_value())
        {
            m_command->toString(out);
        }
        if (m_type.has_value())
        {
            m_type->toString(out);
        }
        if (m_position.has_value())
        {
            m_position->toString(out);
        }
        if (m_detail.has_value())
        {
            m_detail->toString(out);
        }

        if (!out.empty() && out.back() == '/')
        {
            out.pop_back();
        }
    }

    std::optional<LocalId> LocalIdBuilder::fromString(std::string_view localIdStr) noexcept
    {
        return fromString(localIdStr, nullptr);
    }

    std::optional<LocalId> LocalIdBuilder::fromString(std::string_view localIdStr, ParsingErrors& errors) noexcept
    {
        try
        {
            internal::LocalIdParsingErrorBuilder errorBuilder;
            auto result = fromString(localIdStr, &errorBuilder);
            errors = errorBuilder.build();
            return result;
        }
        catch (const std::bad_alloc&)
        {
            return std::nullopt;
        }
    }

    std::optional<LocalId> LocalIdBuilder::fromString(std::string_view str, LocalIdParsingErrorBuilder* eb) noexcept
    {
        if (str.empty())
        {
            if (eb)
            {
                eb->addError(LocalIdParsingState::EmptyState, "LocalId string is empty");
            }
            return std::nullopt;
        }

        if (str[0] != '/')
        {
            if (eb)
            {
                eb->addError(LocalIdParsingState::Formatting, "Invalid format: missing '/' as first character");
            }
            return std::nullopt;
        }

        const auto& vis = VIS::instance();

        VisVersion visVersion{};
        const Gmod* gmod = nullptr;
        const Locations* locations = nullptr;
        const Codebooks* codebooks = nullptr;

        std::optional<GmodPath> primaryItem;
        std::optional<GmodPath> secondaryItem;
        std::optional<MetadataTag> qty, cnt, calc, stateTag, cmd, type, pos, detail;
        bool verbose = false;
        bool invalidSecondaryItem = false;

        int primaryItemStart = -1;
        int secondaryItemStart = -1;

        auto state = LocalIdParsingState::NamingRule;

        int i = 1;

        auto advance = [&](std::string_view segment) { i += static_cast<int>(segment.size()) + 1; };

        auto advanceState = [&](std::string_view segment) {
            i += static_cast<int>(segment.size()) + 1;
            state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
        };

        auto jumpTo = [&](LocalIdParsingState to) { state = to; };

        auto jumpToWith = [&](std::string_view segment, LocalIdParsingState to) {
            i += static_cast<int>(segment.size()) + 1;
            state = to;
        };

        while (state <= LocalIdParsingState::MetaDetail)
        {
            int nextStart = std::min(static_cast<int>(str.size()), i);
            auto nextSlash = str.find('/', static_cast<size_t>(nextStart));
            std::string_view segment = nextSlash == std::string_view::npos
                                           ? str.substr(static_cast<size_t>(nextStart))
                                           : str.substr(static_cast<size_t>(nextStart), nextSlash - nextStart);

            switch (state)
            {
                case LocalIdParsingState::NamingRule:
                {
                    if (segment.empty())
                    {
                        if (eb)
                        {
                            eb->addError(LocalIdParsingState::NamingRule, "Missing naming rule");
                        }
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    if (segment != LocalIdBuilder::namingRule())
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::NamingRule,
                                std::string{ "Invalid naming rule, expected '" } +
                                    std::string{ internal::iso19848::annexC::VersionedNamingRule } + "', got '" +
                                    std::string{ segment } + "'");
                        }
                        return std::nullopt;
                    }

                    advanceState(segment);
                    break;
                }

                case LocalIdParsingState::VisVersion:
                {
                    if (segment.empty())
                    {
                        if (eb)
                        {
                            eb->addError(LocalIdParsingState::VisVersion, "Missing VIS version");
                        }
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    if (!segment.starts_with("vis-"))
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::VisVersion,
                                std::string{ "Invalid VIS version format, expected 'vis-X-Ya', got '" } +
                                    std::string{ segment } + "'");
                        }
                        return std::nullopt;
                    }

                    auto versionStr = segment.substr(4);
                    if (!VisVersions::fromString(versionStr, visVersion))
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::VisVersion,
                                std::string{ "Unknown VIS version: '" } + std::string{ versionStr } + "'");
                        }
                        return std::nullopt;
                    }

                    gmod = &vis.gmod(visVersion);
                    locations = &vis.locations(visVersion);
                    codebooks = &vis.codebooks(visVersion);

                    advanceState(segment);
                    break;
                }

                case LocalIdParsingState::PrimaryItem:
                {
                    if (segment.empty())
                    {
                        bool pathInvalid = false;

                        if (primaryItemStart != -1)
                        {
                            auto path = str.substr(
                                static_cast<size_t>(primaryItemStart), static_cast<size_t>(i - 1 - primaryItemStart));
                            ParsingErrors parseErrors;
                            if (!GmodPath::fromShortPath(path, *gmod, *locations, parseErrors).has_value())
                            {
                                pathInvalid = true;

                                if (eb)
                                {
                                    for (const auto& [t, msg] : parseErrors)
                                    {
                                        eb->addError(
                                            LocalIdParsingState::PrimaryItem,
                                            std::string{ "Invalid GmodPath in Primary item: " } + std::string{ path } +
                                                " - " + msg);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (eb)
                            {
                                eb->addError(
                                    LocalIdParsingState::PrimaryItem,
                                    "Invalid or missing Primary item. Local IDs require atleast primary item and 1 "
                                    "metadata tag.");
                            }
                        }

                        if (!pathInvalid)
                        {
                            if (eb)
                            {
                                eb->addError(
                                    LocalIdParsingState::PrimaryItem,
                                    "Invalid or missing '/meta' prefix after Primary item");
                            }
                        }

                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    auto dashIdx = segment.find('-');
                    auto code = dashIdx != std::string_view::npos ? segment.substr(0, dashIdx) : segment;

                    if (primaryItemStart == -1)
                    {
                        if (eb && !gmod->node(code).has_value())
                        {
                            eb->addError(
                                LocalIdParsingState::PrimaryItem,
                                std::string{ "Invalid start GmodNode in Primary item: " } + std::string{ code });
                        }

                        primaryItemStart = i;
                        advance(segment);
                    }
                    else
                    {
                        bool isSec = segment.starts_with("sec") && (segment.size() == 3 || segment[3] == '/');
                        bool isMeta = segment.starts_with("meta") && (segment.size() == 4 || segment[4] == '/');
                        bool isTilde = !segment.empty() && segment[0] == '~';

                        if (isSec || isMeta || isTilde)
                        {
                            auto path = str.substr(
                                static_cast<size_t>(primaryItemStart), static_cast<size_t>(i - 1 - primaryItemStart));

                            ParsingErrors parseErrors;
                            auto parsed = GmodPath::fromShortPath(path, *gmod, *locations, parseErrors);
                            if (!parsed.has_value())
                            {
                                if (eb)
                                {
                                    for (const auto& [t, msg] : parseErrors)
                                    {
                                        eb->addError(
                                            LocalIdParsingState::PrimaryItem,
                                            std::string{ "Invalid GmodPath in Primary item: " } + std::string{ path } +
                                                " - " + msg);
                                    }
                                }

                                auto nx = getNextStateIndexes(str, LocalIdParsingState::PrimaryItem);
                                if (nx.endIndex != std::string_view::npos)
                                {
                                    i = static_cast<int>(nx.endIndex);
                                }
                                else
                                {
                                    i = static_cast<int>(str.size());
                                }

                                LocalIdParsingState nextState = isSec    ? LocalIdParsingState::SecondaryItem
                                                                : isMeta ? LocalIdParsingState::MetaQuantity
                                                                         : LocalIdParsingState::ItemDescription;
                                jumpTo(nextState);
                                break;
                            }

                            primaryItem = std::move(parsed);

                            if (isTilde)
                            {
                                jumpTo(LocalIdParsingState::ItemDescription);
                            }
                            else
                            {
                                LocalIdParsingState nextState =
                                    isSec ? LocalIdParsingState::SecondaryItem : LocalIdParsingState::MetaQuantity;
                                jumpToWith(segment, nextState);
                            }
                            break;
                        }

                        if (!gmod->node(code).has_value())
                        {
                            if (eb)
                            {
                                eb->addError(
                                    LocalIdParsingState::PrimaryItem,
                                    std::string{ "Invalid GmodNode in Primary item: " } + std::string{ code });
                            }

                            auto nx = getNextStateIndexes(str, LocalIdParsingState::PrimaryItem);

                            if (nx.nextIndex == std::string_view::npos)
                            {
                                if (eb)
                                {
                                    eb->addError(
                                        LocalIdParsingState::PrimaryItem,
                                        "Invalid or missing '/meta' prefix after Primary item");
                                }
                                return std::nullopt;
                            }

                            auto lastPart = str.substr(static_cast<size_t>(i), nx.nextIndex - static_cast<size_t>(i));

                            if (eb)
                            {
                                eb->addError(
                                    LocalIdParsingState::PrimaryItem,
                                    std::string{ "Invalid GmodPath: Last part in Primary item: " } +
                                        std::string{ lastPart });
                            }

                            auto nextSegment = str.substr(nx.nextIndex + 1);
                            LocalIdParsingState nextState =
                                nextSegment.starts_with("sec")    ? LocalIdParsingState::SecondaryItem
                                : nextSegment.starts_with("meta") ? LocalIdParsingState::MetaQuantity
                                                                  : LocalIdParsingState::ItemDescription;

                            i = static_cast<int>(nx.endIndex);
                            jumpTo(nextState);
                            break;
                        }

                        advance(segment);
                    }
                    break;
                }

                case LocalIdParsingState::SecondaryItem:
                {
                    if (segment.empty())
                    {
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    auto dashIdx = segment.find('-');
                    auto code = dashIdx != std::string_view::npos ? segment.substr(0, dashIdx) : segment;

                    if (secondaryItemStart == -1)
                    {
                        if (segment == "sec")
                        {
                            advance(segment);
                            break;
                        }

                        if (eb && !gmod->node(code).has_value())
                        {
                            eb->addError(
                                LocalIdParsingState::SecondaryItem,
                                std::string{ "Invalid start GmodNode in Secondary item: " } + std::string{ code });
                        }

                        secondaryItemStart = i;
                        advance(segment);
                    }
                    else
                    {
                        bool isMeta = segment.starts_with("meta") && (segment.size() == 4 || segment[4] == '/');
                        bool isTilde = !segment.empty() && segment[0] == '~';

                        if (isMeta || isTilde)
                        {
                            auto path = str.substr(
                                static_cast<size_t>(secondaryItemStart),
                                static_cast<size_t>(i - 1 - secondaryItemStart));

                            ParsingErrors parseErrors;
                            auto parsed = GmodPath::fromShortPath(path, *gmod, *locations, parseErrors);
                            if (!parsed.has_value())
                            {
                                invalidSecondaryItem = true;
                                if (eb)
                                {
                                    eb->addError(
                                        LocalIdParsingState::SecondaryItem,
                                        std::string{ "Invalid GmodPath in Secondary item: " } + std::string{ path });
                                }

                                auto nx = getNextStateIndexes(str, LocalIdParsingState::SecondaryItem);
                                if (nx.endIndex != std::string_view::npos)
                                {
                                    i = static_cast<int>(nx.endIndex);
                                }
                                else
                                {
                                    i = static_cast<int>(str.size());
                                }

                                LocalIdParsingState nextState =
                                    isMeta ? LocalIdParsingState::MetaQuantity : LocalIdParsingState::ItemDescription;
                                jumpTo(nextState);
                                break;
                            }

                            secondaryItem = std::move(parsed);

                            if (isTilde)
                            {
                                jumpTo(LocalIdParsingState::ItemDescription);
                            }
                            else
                            {
                                jumpToWith(segment, LocalIdParsingState::MetaQuantity);
                            }
                            break;
                        }

                        if (!gmod->node(code).has_value())
                        {
                            invalidSecondaryItem = true;
                            if (eb)
                            {
                                eb->addError(
                                    LocalIdParsingState::SecondaryItem,
                                    std::string{ "Invalid GmodNode in Secondary item: " } + std::string{ code });
                            }

                            auto nx = getNextStateIndexes(str, LocalIdParsingState::SecondaryItem);

                            if (nx.nextIndex == std::string_view::npos)
                            {
                                if (eb)
                                {
                                    eb->addError(
                                        LocalIdParsingState::SecondaryItem,
                                        "Invalid or missing '/meta' prefix after Secondary item");
                                }
                                return std::nullopt;
                            }

                            auto nextSegment = str.substr(nx.nextIndex + 1);
                            LocalIdParsingState nextState = nextSegment.starts_with("meta")
                                                                ? LocalIdParsingState::MetaQuantity
                                                                : LocalIdParsingState::ItemDescription;

                            i = static_cast<int>(nx.endIndex);
                            jumpTo(nextState);
                            break;
                        }

                        advance(segment);
                    }
                    break;
                }

                case LocalIdParsingState::ItemDescription:
                {
                    if (segment.empty())
                    {
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    verbose = true;

                    auto metaPos = str.find("/meta");
                    if (metaPos == std::string_view::npos)
                    {
                        if (eb)
                        {
                            eb->addError(LocalIdParsingState::ItemDescription, "Missing '/meta' section");
                        }
                        return std::nullopt;
                    }

                    size_t metaEnd = metaPos + 5; // points at the '/' after "/meta", or end of string
                    if (metaEnd < str.size() && str[metaEnd] == '/')
                    {
                        ++metaEnd;
                    }
                    i = static_cast<int>(metaEnd);
                    jumpTo(LocalIdParsingState::MetaQuantity);
                    break;
                }

                case LocalIdParsingState::MetaQuantity:
                case LocalIdParsingState::MetaContent:
                case LocalIdParsingState::MetaCalculation:
                case LocalIdParsingState::MetaState:
                case LocalIdParsingState::MetaCommand:
                case LocalIdParsingState::MetaType:
                case LocalIdParsingState::MetaPosition:
                case LocalIdParsingState::MetaDetail:
                {
                    if (segment.empty())
                    {
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    auto tildePos = segment.find('~');
                    auto dashPos = segment.find('-');
                    size_t prefixEnd = std::min(
                        tildePos == std::string_view::npos ? segment.size() : tildePos,
                        dashPos == std::string_view::npos ? segment.size() : dashPos);

                    if (prefixEnd == segment.size())
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::Formatting,
                                std::string{ "Invalid metadata tag: missing prefix '-' or '~' in " } +
                                    std::string{ segment });
                        }
                        advance(segment);
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    auto prefix = segment.substr(0, prefixEnd);

                    LocalIdParsingState actualState;
                    if (prefix == "qty")
                    {
                        actualState = LocalIdParsingState::MetaQuantity;
                    }
                    else if (prefix == "cnt")
                    {
                        actualState = LocalIdParsingState::MetaContent;
                    }
                    else if (prefix == "calc")
                    {
                        actualState = LocalIdParsingState::MetaCalculation;
                    }
                    else if (prefix == "state")
                    {
                        actualState = LocalIdParsingState::MetaState;
                    }
                    else if (prefix == "cmd")
                    {
                        actualState = LocalIdParsingState::MetaCommand;
                    }
                    else if (prefix == "type")
                    {
                        actualState = LocalIdParsingState::MetaType;
                    }
                    else if (prefix == "pos")
                    {
                        actualState = LocalIdParsingState::MetaPosition;
                    }
                    else if (prefix == "detail")
                    {
                        actualState = LocalIdParsingState::MetaDetail;
                    }
                    else
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::Formatting,
                                std::string{ "Invalid metadata tag: unknown prefix " } + std::string{ prefix });
                        }
                        return std::nullopt;
                    }

                    if (actualState < state)
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::Formatting,
                                std::string{ "Invalid metadata tag: unknown prefix " } + std::string{ prefix });
                        }
                        return std::nullopt;
                    }

                    if (actualState > state)
                    {
                        jumpTo(actualState);
                        break;
                    }

                    bool isTilde = segment[prefixEnd] == '~';
                    auto value = segment.substr(prefixEnd + 1);

                    CodebookName cbName;
                    try
                    {
                        cbName = CodebookNames::fromPrefix(prefix);
                    }
                    catch (...)
                    {
                        if (eb)
                        {
                            eb->addError(
                                LocalIdParsingState::Formatting,
                                std::string{ "Invalid metadata tag: unknown prefix " } + std::string{ prefix });
                        }
                        return std::nullopt;
                    }

                    if (value.empty())
                    {
                        if (eb)
                        {
                            eb->addError(
                                state,
                                std::string{ "Invalid " } + std::string{ CodebookNames::toString(cbName) } +
                                    " metadata tag: missing value");
                        }
                        return std::nullopt;
                    }

                    auto tag = (*codebooks)[cbName].createTag(value);

                    if (!tag.has_value())
                    {
                        if (eb)
                        {
                            eb->addError(
                                state,
                                std::string{ "Invalid metadata tag value for " } + std::string{ prefix } + ": '" +
                                    std::string{ value } + "'");
                        }
                        advance(segment);
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    if (!isTilde && tag->isCustom() && cbName != CodebookName::Position &&
                        cbName != CodebookName::Detail)
                    {
                        if (eb)
                        {
                            auto cbNameStr = CodebookNames::toString(cbName);
                            eb->addError(
                                state,
                                std::string{ "Invalid " } + std::string{ cbNameStr } + " metadata tag: '" +
                                    std::string{ value } + "'. Use prefix '~' for custom values");
                        }
                        advance(segment);
                        state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                        break;
                    }

                    switch (cbName)
                    {
                        case CodebookName::Quantity:
                        {
                            qty = tag;
                            break;
                        }
                        case CodebookName::Content:
                        {
                            cnt = tag;
                            break;
                        }
                        case CodebookName::Calculation:
                        {
                            calc = tag;
                            break;
                        }
                        case CodebookName::State:
                        {
                            stateTag = tag;
                            break;
                        }
                        case CodebookName::Command:
                        {
                            cmd = tag;
                            break;
                        }
                        case CodebookName::Type:
                        {
                            type = tag;
                            break;
                        }
                        case CodebookName::Position:
                        {
                            pos = tag;
                            break;
                        }
                        case CodebookName::Detail:
                        {
                            detail = tag;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }

                    LocalIdParsingState nextMetaState =
                        static_cast<LocalIdParsingState>(static_cast<int>(actualState) + 1);
                    jumpToWith(segment, nextMetaState);
                    break;
                }

                default:
                {
                    i += static_cast<int>(segment.size()) + 1;
                    state = static_cast<LocalIdParsingState>(static_cast<int>(state) + 1);
                    break;
                }
            }
        }

        bool hasMetadata = qty.has_value() || cnt.has_value() || calc.has_value() || stateTag.has_value() ||
                           cmd.has_value() || type.has_value() || pos.has_value() || detail.has_value();

        if (!hasMetadata)
        {
            if (eb)
            {
                eb->addError(
                    LocalIdParsingState::Completeness,
                    "No metadata tags specified. Local IDs require atleast 1 metadata tag.");
            }
        }

        bool succeeded = !(eb && eb->hasError()) && !invalidSecondaryItem && hasMetadata;

        if (!succeeded)
        {
            return std::nullopt;
        }

        try
        {
            auto builder = LocalIdBuilder::create(visVersion);

            if (primaryItem.has_value())
            {
                builder = builder.withPrimaryItem(*primaryItem);
            }
            if (secondaryItem.has_value())
            {
                builder = builder.withSecondaryItem(*secondaryItem);
            }
            if (verbose)
            {
                builder = builder.withVerboseMode(true);
            }
            if (qty.has_value())
            {
                builder = builder.withMetadataTag(*qty);
            }
            if (cnt.has_value())
            {
                builder = builder.withMetadataTag(*cnt);
            }
            if (calc.has_value())
            {
                builder = builder.withMetadataTag(*calc);
            }
            if (stateTag.has_value())
            {
                builder = builder.withMetadataTag(*stateTag);
            }
            if (cmd.has_value())
            {
                builder = builder.withMetadataTag(*cmd);
            }
            if (type.has_value())
            {
                builder = builder.withMetadataTag(*type);
            }
            if (pos.has_value())
            {
                builder = builder.withMetadataTag(*pos);
            }
            if (detail.has_value())
            {
                builder = builder.withMetadataTag(*detail);
            }

            return builder.build();
        }
        catch (const std::exception& e)
        {
            if (eb)
            {
                eb->addError(LocalIdParsingState::Completeness, e.what());
            }
            return std::nullopt;
        }
    }
} // namespace dnv::vista::sdk
