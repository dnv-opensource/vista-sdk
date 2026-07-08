/**
 * @file EmbeddedTestData.h
 * @brief Access to test data files embedded at build time
 * @details Provides access to testdata/ *.json and *.txt files embedded via dnv_vista_sdk_embed_blobs.
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk
{
    /**
     * @brief Access to embedded test data resources
     * @details Wraps the generated testdata registry. Use text() for raw content
     *          and listFiles() to enumerate available files.
     */
    class EmbeddedTestData
    {
    public:
        /**
         * @brief List all available test data filenames
         * @return Vector of filenames (e.g., ["Locations.json", "LocalIds.txt"])
         */
        static std::vector<std::string> listFiles();

        /**
         * @brief Load a test data file as raw text
         * @param filename Name of the file (e.g., "Locations.json")
         * @return File contents, or empty string if not found
         */
        static std::string text(std::string_view filename);
    };
} // namespace dnv::vista::sdk
