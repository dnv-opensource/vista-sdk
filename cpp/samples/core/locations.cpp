/**
 * @file locations.cpp
 * @brief Demonstrates usage of vista-sdk Locations and LocationBuilder APIs
 * @details This sample shows how to parse locations, validate location strings,
 *          use the LocationBuilder fluent API, and handle location components
 */

#include <dnv/vista/sdk/core/LocationBuilder.h>
#include <dnv/vista/sdk/core/ParsingErrors.h>
#include <dnv/vista/sdk/core/VIS.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk Locations Sample ===\n\n";

    {
        std::cout << "1. Locations: Accessing location data for a VIS version\n";
        std::cout << "----------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        auto version = vis.latest();
        const auto& locations = vis.locations(version);

        std::cout << "Locations for version   : " << VisVersions::toString(locations.version()) << "\n";
        std::cout << "Total relative locations: " << locations.relativeLocations().size() << "\n";
        std::cout << "Location groups         : " << locations.groups().size() << "\n\n";
    }

    {
        std::cout << "2. Location Parsing: Valid location strings\n";
        std::cout << "----------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        std::vector<std::string> validLocations = { "1", "5", "42", "1P", "2CF", "3SU", "10FI", "CFOU" };

        std::cout << std::boolalpha;
        for (const auto& locStr : validLocations)
        {
            auto location = locations.fromString(locStr);
            constexpr size_t columnWidth = 7;
            size_t padding = locStr.length() < columnWidth ? columnWidth - locStr.length() : 0;
            std::cout << "  '" << locStr << "'" << std::string(padding, ' ') << "-> ";
            if (location.has_value())
            {
                std::cout << "Valid: '" << location->value() << "'\n";
            }
            else
            {
                std::cout << "Invalid (unexpected)\n";
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "3. Location Parsing: Invalid location strings with errors\n";
        std::cout << "------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        std::vector<std::string> invalidLocations = {
            "",         // Empty
            "   ",      // Whitespace only
            "X",        // Invalid character
            "1X",       // Invalid character after number
            "ZPS",      // Invalid first character
            "PC",       // Multiple from same group (Side)
            "1PS",      // Multiple from same group (Side)
            "1UL",      // Multiple from same group (Vertical)
            "2IO",      // Multiple from same group (Transverse)
            "SP1",      // Number not at start
            "1SPA",     // Not alphabetically sorted
            "10PSFI",   // Multiple from Side (P+S) and multiple groups
            "ACFIMOPSU" // Multiple from Side (P+S) and Transverse (I+O)
        };

        std::cout << std::boolalpha;
        for (const auto& locStr : invalidLocations)
        {
            ParsingErrors errors;
            auto location = locations.fromString(locStr, errors);

            std::cout << "  \"" << locStr << "\" -> Invalid\n";
            if (!location.has_value() && errors.hasErrors())
            {
                std::cout << "    Errors:\n";
                for (const auto& error : errors)
                {
                    std::cout << "      - " << error.message << "\n";
                }
            }
        }
        std::cout << "\n";
    }

    {
        std::cout << "4. RelativeLocations: Exploring location metadata\n";
        std::cout << "----------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        std::cout << "Location codes and their meanings:\n";
        for (const auto& relLoc : locations.relativeLocations())
        {
            std::cout << "  " << relLoc.code() << " - " << relLoc.name();
            if (relLoc.definition().has_value())
            {
                std::cout << " (" << relLoc.definition().value() << ")";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "5. Location Groups: Accessing grouped location codes\n";
        std::cout << "-------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        const auto& groups = locations.groups();

        std::cout << "Location groups:\n";
        for (const auto& [group, relLocs] : groups)
        {
            std::cout << "  ";
            switch (group)
            {
                case LocationGroup::Side:
                    std::cout << std::left << std::setw(14) << "Side:";
                    break;
                case LocationGroup::Vertical:
                    std::cout << std::left << std::setw(14) << "Vertical:";
                    break;
                case LocationGroup::Transverse:
                    std::cout << std::left << std::setw(14) << "Transverse:";
                    break;
                case LocationGroup::Longitudinal:
                    std::cout << std::left << std::setw(14) << "Longitudinal:";
                    break;
                default:
                    std::cout << std::left << std::setw(14) << "Other:";
                    break;
            }

            for (size_t i = 0; i < relLocs.size(); ++i)
            {
                std::cout << relLocs[i].code();
                if (i < relLocs.size() - 1)
                    std::cout << ", ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    {
        std::cout << "6. LocationBuilder: Creating locations with fluent API\n";
        std::cout << "--------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        // Create builder and build location step by step
        auto builder = LocationBuilder::create(locations).withNumber(1);

        std::cout << "Building location step by step:\n";
        std::cout << "  After withNumber(1)        : " << builder.toString() << "\n";

        builder = builder.withSide('P');
        std::cout << "  After withSide('P')        : " << builder.toString() << "\n";

        builder = builder.withVertical('U');
        std::cout << "  After withVertical('U')    : " << builder.toString() << "\n";

        builder = builder.withTransverse('I');
        std::cout << "  After withTransverse('I')  : " << builder.toString() << "\n";

        builder = builder.withLongitudinal('F');
        std::cout << "  After withLongitudinal('F'): " << builder.toString() << "\n";

        auto location = builder.build();
        std::cout << "  Final location             : " << location.value() << "\n\n";
    }

    {
        std::cout << "7. LocationBuilder: Method chaining\n";
        std::cout << "--------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        // Build location with method chaining
        auto location = LocationBuilder::create(locations)
                            .withNumber(5)
                            .withSide('S')
                            .withVertical('M')
                            .withLongitudinal('A')
                            .build();

        std::cout << "Location built with chaining: " << location.value() << "\n\n";
    }

    {
        std::cout << "8. LocationBuilder: Parsing existing locations\n";
        std::cout << "-------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        // Parse an existing location
        auto existingLoc = locations.fromString("2CF").value();

        auto builder = LocationBuilder::create(locations).withLocation(existingLoc);

        std::cout << "Original location: " << existingLoc.value() << "\n";
        std::cout << "Builder state after parsing:\n";
        std::cout << "  " << std::left << "Number      :"
                  << (builder.number().has_value() ? std::to_string(builder.number().value()) : "none") << "\n";
        std::cout << "  " << std::left
                  << "Side        :" << (builder.side().has_value() ? std::string(1, builder.side().value()) : "none")
                  << "\n";
        std::cout << "  " << std::left << "Vertical    :"
                  << (builder.vertical().has_value() ? std::string(1, builder.vertical().value()) : "none") << "\n";
        std::cout << "  " << std::left << "Transverse  :"
                  << (builder.transverse().has_value() ? std::string(1, builder.transverse().value()) : "none") << "\n";
        std::cout << "  " << std::left << "Longitudinal:"
                  << (builder.longitudinal().has_value() ? std::string(1, builder.longitudinal().value()) : "none")
                  << "\n\n";
    }

    {
        std::cout << "9. LocationBuilder: Modifying locations\n";
        std::cout << "------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        // Start with a location
        auto original = locations.fromString("1P").value();
        std::cout << "Original: " << original.value() << "\n";

        // Modify it
        auto builder = LocationBuilder::create(locations)
                           .withLocation(original)
                           .withoutSide()      // Remove side
                           .withVertical('U'); // Add vertical

        auto modified = builder.build();
        std::cout << "Modified (removed side, added vertical)   : " << modified.value() << "\n";

        // Create variation
        auto variation = LocationBuilder::create(locations)
                             .withLocation(original)
                             .withNumber(5)      // Change number
                             .withVertical('L'); // Add vertical

        std::cout << "Variation (changed number, added vertical): " << variation.build().value() << "\n\n";
    }

    {
        std::cout << "10. LocationBuilder: Using withCode() and withNumber() methods\n";
        std::cout << "---------------------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        auto builder = LocationBuilder::create(locations)
                           .withNumber(3)  // int -> set number component
                           .withCode('C')  // char -> automatically routes to withSide()
                           .withCode('M')  // char -> automatically routes to withVertical()
                           .withCode('O')  // char -> automatically routes to withTransverse()
                           .withCode('F'); // char -> automatically routes to withLongitudinal()

        auto location = builder.build();
        std::cout << "Location built with withCode(): " << location.value() << "\n";
        std::cout << "Components automatically routed to correct groups\n\n";
    }

    {
        std::cout << "11. LocationBuilder: Removing components\n";
        std::cout << "-------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        auto builder = LocationBuilder::create(locations)
                           .withNumber(1)
                           .withSide('P')
                           .withVertical('U')
                           .withTransverse('I')
                           .withLongitudinal('F');

        std::cout << "Full location               : " << builder.toString() << "\n";

        builder = builder.withoutNumber();
        std::cout << "After withoutNumber()       : " << builder.toString() << "\n";

        builder = builder.withoutTransverse();
        std::cout << "After withoutTransverse()   : " << builder.toString() << "\n";

        builder = builder.withoutValue(LocationGroup::Vertical);
        std::cout << "After withoutValue(Vertical): " << builder.toString() << "\n\n";
    }

    {
        std::cout << "12. LocationBuilder: Validation errors\n";
        std::cout << "----------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        std::cout << "Testing invalid operations:\n";

        // Invalid number (< 1)
        try
        {
            [[maybe_unused]] auto builder = LocationBuilder::create(locations).withNumber(0);
            std::cout << "  withNumber(0): Accepted (unexpected)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "  withNumber(0): Rejected - " << e.what() << "\n";
        }

        // Invalid character for Side
        try
        {
            [[maybe_unused]] auto builder = LocationBuilder::create(locations).withSide('X');
            std::cout << "  withSide('X'): Accepted (unexpected)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "  withSide('X'): Rejected - " << e.what() << "\n";
        }

        // Invalid character for withCode()
        try
        {
            [[maybe_unused]] auto builder = LocationBuilder::create(locations).withCode('Z');
            std::cout << "  withCode('Z'): Accepted (unexpected)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "  withCode('Z'): Rejected - " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "13. Complete workflow: Parse, modify, validate\n";
        std::cout << "------------------------------------------------\n";

        const auto& vis = VIS::instance();
        const auto& locations = vis.locations(vis.latest());

        // Parse user input
        std::string userInput = "3SU";
        auto parsedLocation = locations.fromString(userInput);

        if (parsedLocation.has_value())
        {
            std::cout << "User input: \"" << userInput << "\" -> Valid\n";

            // Modify it
            auto modified = LocationBuilder::create(locations)
                                .withLocation(parsedLocation.value())
                                .withNumber(10)
                                .withVertical('U')
                                .withLongitudinal('F')
                                .build();

            std::cout << "Modified location: " << modified.value() << "\n";

            // Validate the modified location can be re-parsed
            auto revalidated = locations.fromString(modified.value());
            if (revalidated.has_value())
            {
                std::cout << "Modified location is valid: " << revalidated->value() << "\n";
            }
        }
        else
        {
            std::cout << "User input: \"" << userInput << "\" -> Invalid\n";
        }

        std::cout << "\n";
    }

    return 0;
}
