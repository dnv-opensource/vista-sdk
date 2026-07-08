#include "dnv/vista/sdk/core/Codebooks.h"

#include "dto/CodebooksDto.h"

#include <stdexcept>

namespace dnv::vista::sdk
{
    Codebooks::Codebooks(VisVersion version, const CodebooksDto& dto)
        : m_version{ version }
    {
        for (const auto& item : dto.items)
        {
            Codebook cb{ item };
            auto idx = static_cast<size_t>(cb.name()) - 1;
            if (idx < m_codebooks.size())
            {
                m_codebooks[idx].emplace(std::move(cb));
            }
        }

        CodebookDto detailDto{ "detail", {} };
        Codebook detail{ detailDto };
        auto idx = static_cast<size_t>(detail.name()) - 1;
        if (idx < m_codebooks.size())
        {
            m_codebooks[idx].emplace(std::move(detail));
        }
    }
} // namespace dnv::vista::sdk
