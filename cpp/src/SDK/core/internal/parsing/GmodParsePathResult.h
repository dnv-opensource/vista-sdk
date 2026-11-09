/**
 * @file GmodParsePathResult.h
 * @brief Internal result type for Gmod path parsing operations
 * @details Result monad (Ok/Error) for propagating detailed parsing errors from internal parsing functions.
 */

#pragma once

#include "dnv/vista/sdk/core/GmodPath.h"

#include <string>
#include <variant>

namespace dnv::vista::sdk::internal
{
    class GmodParsePathResult
    {
    private:
        struct OkState
        {
            GmodPath path;
        };

        struct ErrorState
        {
            std::string message;
        };

        explicit GmodParsePathResult(OkState s)
            : m_state{ std::move(s) }
        {}
        explicit GmodParsePathResult(ErrorState s)
            : m_state{ std::move(s) }
        {}

    public:
        [[nodiscard]] static GmodParsePathResult ok(GmodPath path)
        {
            return GmodParsePathResult{ OkState{ std::move(path) } };
        }

        [[nodiscard]] static GmodParsePathResult error(std::string message)
        {
            return GmodParsePathResult{ ErrorState{ std::move(message) } };
        }

        [[nodiscard]] explicit operator bool() const noexcept { return std::holds_alternative<OkState>(m_state); }

        [[nodiscard]] GmodPath& path() { return std::get<OkState>(m_state).path; }
        [[nodiscard]] const GmodPath& path() const { return std::get<OkState>(m_state).path; }

        [[nodiscard]] const std::string& errorMessage() const { return std::get<ErrorState>(m_state).message; }

    private:
        std::variant<OkState, ErrorState> m_state;
    };
} // namespace dnv::vista::sdk::internal
