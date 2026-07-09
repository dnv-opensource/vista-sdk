/**
 * @file imonumber.cpp
 * @brief Demonstrates usage of vista-sdk ImoNumber API
 * @details This sample shows how to create, validate, and parse IMO numbers
 *          with checksum verification per IMO Resolution A.600(15)
 */

#include <dnv/vista/sdk/core/ImoNumber.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace dnv::vista::sdk;

    std::cout << "=== vista-sdk ImoNumber Sample ===\n\n";

    {
        std::cout << "1. ImoNumber: Creating from integer\n";
        std::cout << "--------------------------------------\n";

        // Create valid IMO numbers
        auto imo1 = ImoNumber{ 9074729 };
        auto imo2 = ImoNumber{ 9785811 };
        auto imo3 = ImoNumber{ 1234567 };

        std::cout << "Created IMO numbers:\n";
        std::cout << "  " << imo1.toString() << "\n";
        std::cout << "  " << imo2.toString() << "\n";
        std::cout << "  " << imo3.toString() << "\n";

        // Try invalid number (will throw)
        std::cout << "\nAttempting to create invalid IMO number (1234507):\n";
        try
        {
            [[maybe_unused]] auto invalid = ImoNumber{ 1234507 };
            std::cout << "  ERROR: Invalid number accepted (unexpected)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "  Correctly rejected: " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "2. ImoNumber: Creating from string\n";
        std::cout << "-------------------------------------\n";

        // With "IMO" prefix
        auto imo1 = ImoNumber{ "IMO9074729" };
        std::cout << "From 'IMO9074729': " << imo1.toString() << "\n";

        // Without "IMO" prefix
        auto imo2 = ImoNumber{ "9785811" };
        std::cout << "From '9785811'   : " << imo2.toString() << "\n";

        // Both create equivalent objects
        std::cout << std::boolalpha;
        std::cout << "Are they equal? " << (imo1 == ImoNumber{ "IMO9074729" }) << "\n";
        std::cout << "Are they equal? " << (imo2 == ImoNumber{ "IMO9785811" }) << "\n";

        // Try invalid string (will throw)
        std::cout << "\nAttempting to create from invalid string ('IM9074729'):\n";
        try
        {
            [[maybe_unused]] auto invalid = ImoNumber{ "IM9074729" };
            std::cout << "  ERROR: Invalid string accepted (unexpected)\n";
        }
        catch (const std::invalid_argument& e)
        {
            std::cout << "  Correctly rejected: " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "3. ImoNumber::isValid(): Validating integers\n";
        std::cout << "-----------------------------------------------\n";

        std::vector<int> testNumbers = {
            9074729,  // Valid
            9785811,  // Valid
            1234567,  // Valid
            1234507,  // Invalid checksum
            123456,   // Too short
            12345678, // Too long
            0,        // Invalid
            -1        // Invalid
        };

        std::cout << std::boolalpha;
        std::cout << "Validation results:\n";
        for (int num : testNumbers)
        {
            bool valid = ImoNumber::isValid(num);
            std::cout << "  " << std::setw(10) << num << ": " << valid << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "4. ImoNumber::fromString(): Safe parsing without exceptions\n";
        std::cout << "-------------------------------------------------------------\n";

        std::vector<std::string> testStrings = {
            "9074729",   "IMO9785811", "1234567", // Valid
            "1234507",                            // Invalid checksum
            "IM9074729",                          // Typo in prefix
            "",                                   // Empty
            "abc"                                 // Not a number
        };

        std::cout << "Parsing results:\n";
        for (const auto& str : testStrings)
        {
            auto result = ImoNumber::fromString(str);

            std::cout << "  '" << str << "'" << std::string(10 - str.length() + 1, ' ') << "-> ";

            if (result.has_value())
            {
                std::cout << result->toString() << "\n";
            }
            else
            {
                std::cout << "Invalid\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "5. ImoNumber::fromString(): Optional usage patterns\n";
        std::cout << "----------------------------------------------------\n";

        std::string input = "IMO9074729";

        // Pattern 1: Direct if with optional
        if (auto imo = ImoNumber::fromString(input))
        {
            std::cout << "Parsed successfully: " << imo->toString() << "\n";
        }
        else
        {
            std::cout << "Parse failed\n";
        }

        // Pattern 2: has_value() check
        auto result = ImoNumber::fromString("9785811");
        if (result.has_value())
        {
            std::cout << "Parsed successfully: " << result.value().toString() << "\n";
        }

        std::cout << "\n";
    }

    {
        std::cout << "6. ImoNumber: Comparison operators\n";
        std::cout << "-------------------------------------\n";

        auto imo1 = ImoNumber{ 9074729 };
        auto imo2 = ImoNumber{ "IMO9074729" };
        auto imo3 = ImoNumber{ 9785811 };

        std::cout << std::boolalpha;
        std::cout << "Equality:\n";
        std::cout << "  9074729 == IMO9074729? " << (imo1 == imo2) << "\n";
        std::cout << "  9074729 == 9785811   ? " << (imo1 == imo3) << "\n";

        std::cout << "\nInequality:\n";
        std::cout << "  9074729 != 9785811   ? " << (imo1 != imo3) << "\n";
        std::cout << "  9074729 != IMO9074729? " << (imo1 != imo2) << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "7. ImoNumber: toString() formatting\n";
        std::cout << "--------------------------------------\n";

        auto imo = ImoNumber{ 9074729 };

        std::cout << "String representation:\n";
        std::cout << "  toString(): " << imo.toString() << "\n";
        std::cout << "  Format    : Always includes \"IMO\" prefix\n";
        std::cout << "  Format    : Always 7 digits after \"IMO\"\n";

        // Different input formats, same output
        std::cout << "\nConsistent output:\n";
        std::cout << "  From int 9074729 : " << ImoNumber{ 9074729 }.toString() << "\n";
        std::cout << "  From '9074729'   : " << ImoNumber{ "9074729" }.toString() << "\n";
        std::cout << "  From 'IMO9074729': " << ImoNumber{ "IMO9074729" }.toString() << "\n";

        std::cout << "\n";
    }

    {
        std::cout << "8. ImoNumber: Real-world examples\n";
        std::cout << "------------------------------------\n";

        // Real IMO numbers
        struct Ship
        {
            std::string name;
            int imoNumber;
        };

        std::vector<Ship> ships = { { "Queen Mary 2", 9241061 },
                                    { "Peter Faber", 8027781 },
                                    { "Oasis of the Seas", 9383936 },
                                    { "Symphony of the Seas", 9744001 } };

        std::cout << "Famous ships:\n";
        for (const auto& ship : ships)
        {
            if (ImoNumber::isValid(ship.imoNumber))
            {
                auto imo = ImoNumber{ ship.imoNumber };
                std::cout << "  " << std::setw(20) << std::left << ship.name;
                std::cout << ": " << imo.toString() << "\n";
            }
        }

        std::cout << "\n";
    }

    {
        std::cout << "9. ImoNumber: Error handling patterns\n";
        std::cout << "-----------------------------------------\n";

        std::vector<std::string> inputs = { "9074729", "1234507" }; // Valid, Invalid

        for (const auto& userInput : inputs)
        {
            std::cout << "\nTesting with '" << userInput << "':\n";

            std::cout << "  Pattern 1: Constructor with try-catch (throws on error):\n";
            try
            {
                auto imo = ImoNumber{ userInput };
                std::cout << "    Success: " << imo.toString() << "\n";
            }
            catch (const std::invalid_argument& e)
            {
                std::cout << "    Error: " << e.what() << "\n";
            }

            std::cout << "  Pattern 2: fromString() with optional (no exceptions):\n";
            if (auto imo = ImoNumber::fromString(userInput))
            {
                std::cout << "    Success: " << imo->toString() << "\n";
            }
            else
            {
                std::cout << "    Error: Invalid IMO number format\n";
            }
        }

        std::cout << "\n";
    }

    return 0;
}
