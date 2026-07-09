#include <doctest/doctest.h>

#include <dnv/vista/sdk/core/ParsingErrors.h>

#include <algorithm>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        ParsingErrors makeErrors(std::vector<ParsingErrors::ErrorEntry> entries)
        {
            return ParsingErrors{ std::move(entries) };
        }
    } // namespace

    TEST_SUITE("ParsingErrors")
    {
        TEST_CASE("default constructor creates empty")
        {
            ParsingErrors errors;

            CHECK_EQ(errors.count(), 0u);
            CHECK_FALSE(errors.hasErrors());
        }

        TEST_CASE("constructor moves entries")
        {
            auto errors = makeErrors({ { "Invalid", "First error" }, { "InvalidCode", "Second error" } });

            CHECK_EQ(errors.count(), 2u);
            CHECK(errors.hasErrors());
        }

        TEST_CASE("hasErrorType")
        {
            auto errors = makeErrors({ { "Invalid", "msg" }, { "InvalidCode", "msg" } });

            CHECK(errors.hasErrorType("Invalid"));
            CHECK(errors.hasErrorType("InvalidCode"));
            CHECK_FALSE(errors.hasErrorType("InvalidOrder"));
            CHECK_FALSE(errors.hasErrorType(""));
        }

        TEST_CASE("equality")
        {
            ParsingErrors empty1;
            ParsingErrors empty2;
            CHECK(empty1 == empty2);

            auto a = makeErrors({ { "Invalid", "msg" } });
            auto b = makeErrors({ { "Invalid", "msg" } });
            auto c = makeErrors({ { "InvalidCode", "other" } });

            CHECK(a == b);
            CHECK_FALSE(a == c);
        }

        TEST_CASE("ErrorEntry equality")
        {
            ParsingErrors::ErrorEntry e1{ "Invalid", "msg" };
            ParsingErrors::ErrorEntry e2{ "Invalid", "msg" };
            ParsingErrors::ErrorEntry e3{ "InvalidCode", "msg" };
            ParsingErrors::ErrorEntry e4{ "Invalid", "other" };

            CHECK(e1 == e2);
            CHECK_FALSE(e1 == e3);
            CHECK_FALSE(e1 == e4);
        }

        TEST_CASE("toString - empty returns Success")
        {
            CHECK_EQ(ParsingErrors{}.toString(), "Success");
        }

        TEST_CASE("toString - formats entries")
        {
            auto errors = makeErrors({ { "Invalid", "bad input" }, { "InvalidCode", "bad code" } });
            const auto result = errors.toString();

            CHECK_NE(result.find("Parsing errors:"), std::string::npos);
            CHECK_NE(result.find("Invalid"), std::string::npos);
            CHECK_NE(result.find("bad input"), std::string::npos);
            CHECK_NE(result.find("InvalidCode"), std::string::npos);
            CHECK_NE(result.find("bad code"), std::string::npos);
            CHECK_NE(result.find('\t'), std::string::npos);
        }

        TEST_CASE("iteration")
        {
            auto errors =
                makeErrors({ { "Invalid", "first" }, { "InvalidCode", "second" }, { "InvalidOrder", "third" } });

            size_t count = 0;
            for (const auto& [type, message] : errors)
            {
                CHECK_FALSE(type.empty());
                CHECK_FALSE(message.empty());
                ++count;
            }
            CHECK_EQ(count, 3u);

            auto it = errors.begin();
            CHECK_EQ(it->type, "Invalid");
            CHECK_EQ(it->message, "first");
            ++it;
            CHECK_EQ(it->type, "InvalidCode");
            ++it;
            CHECK_EQ(it->type, "InvalidOrder");
            ++it;
            CHECK_EQ(it, errors.end());
        }

        TEST_CASE("STL algorithms work on range")
        {
            auto errors = makeErrors({ { "Invalid", "a" }, { "InvalidCode", "b" }, { "Invalid", "c" } });

            auto invalidCount =
                std::count_if(errors.begin(), errors.end(), [](const auto& e) { return e.type == "Invalid"; });
            CHECK_EQ(invalidCount, 2);

            auto it = std::find_if(errors.begin(), errors.end(), [](const auto& e) { return e.type == "InvalidCode"; });
            REQUIRE_NE(it, errors.end());
            CHECK_EQ(it->message, "b");
        }
    }
} // namespace dnv::vista::sdk::tests
