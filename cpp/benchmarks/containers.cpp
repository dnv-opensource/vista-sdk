/**
 * @file containers.cpp
 * @brief Nanobench performance benchmark for SDK custom containers vs STL
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/VistaSDK.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace dnv::vista::sdk::benchmarks
{
    static std::vector<std::string> makeGmodKeys()
    {
        std::vector<std::string> keys;
        keys.reserve(512);

        keys.push_back("VE");

        for (int a = 4; a <= 9; ++a)
        {
            for (int b = 0; b <= 9; ++b)
            {
                keys.push_back(std::to_string(a * 100 + b * 10));
            }
        }

        for (int a = 4; a <= 6; ++a)
        {
            for (int b = 1; b <= 9; ++b)
            {
                for (int c = 1; c <= 9; ++c)
                {
                    keys.push_back(std::to_string(a * 100 + b * 10) + "." + std::to_string(c));
                }
            }
        }

        for (int a = 4; a <= 5; ++a)
        {
            for (int b = 1; b <= 5; ++b)
            {
                for (int c = 1; c <= 5; ++c)
                {
                    for (int d = 1; d <= 4; ++d)
                    {
                        keys.push_back(
                            std::to_string(a * 100 + b * 10) + "." + std::to_string(c) + "." + std::to_string(d));
                    }
                }
            }
        }

        return keys;
    }

    static std::vector<std::pair<std::string, int>> makeItems(const std::vector<std::string>& keys)
    {
        std::vector<std::pair<std::string, int>> items;
        items.reserve(keys.size());
        for (int i = 0; i < static_cast<int>(keys.size()); ++i)
        {
            items.emplace_back(keys[i], i);
        }
        return items;
    }

    int run()
    {
        const auto keys = makeGmodKeys();

        PerfectHashMap<int> perfectMap{ makeItems(keys) };

        std::unordered_map<std::string, int> stdMap;
        stdMap.reserve(keys.size());
        for (int i = 0; i < static_cast<int>(keys.size()); ++i)
        {
            stdMap.emplace(keys[i], i);
        }

        std::unordered_set<std::string> stdSet;
        stdSet.reserve(keys.size());
        for (const auto& k : keys)
        {
            stdSet.insert(k);
        }

        ankerl::nanobench::Bench bench;
        bench.title("Containers").warmup(10000).minEpochIterations(11000000);

        bench.run("PerfectHashMap_hit", [&] {
            auto a = perfectMap.find("VE");
            auto b = perfectMap.find("411.1");
            auto c = perfectMap.find("400");
            auto d = perfectMap.find("510.2.3");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        bench.run("StdUnorderedMap_hit", [&] {
            auto a = stdMap.find("VE");
            auto b = stdMap.find("411.1");
            auto c = stdMap.find("400");
            auto d = stdMap.find("510.2.3");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        bench.run("PerfectHashMap_miss", [&] {
            auto a = perfectMap.find("VEX");
            auto b = perfectMap.find("411.1X");
            auto c = perfectMap.find("400X");
            auto d = perfectMap.find("H346.11112");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        bench.run("StdUnorderedMap_miss", [&] {
            auto a = stdMap.find("VEX");
            auto b = stdMap.find("411.1X");
            auto c = stdMap.find("400X");
            auto d = stdMap.find("H346.11112");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        std::string_view svA = "VE";
        std::string_view svB = "411.1";
        std::string_view svC = "400";
        std::string_view svD = "H346.11112";

        bench.run("PerfectHashMap_stringview", [&] {
            auto a = perfectMap.find(svA);
            auto b = perfectMap.find(svB);
            auto c = perfectMap.find(svC);
            auto d = perfectMap.find(svD);
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        bench.run("StdUnorderedSet_hit", [&] {
            auto a = stdSet.contains("VE");
            auto b = stdSet.contains("411.1");
            auto c = stdSet.contains("400");
            auto d = stdSet.contains("510.2.3");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        StringSet fastSet;
        fastSet.reserve(keys.size());
        for (const auto& k : keys)
        {
            fastSet.insert(k);
        }

        bench.run("StringSet_hit", [&] {
            auto a = fastSet.contains("VE");
            auto b = fastSet.contains("411.1");
            auto c = fastSet.contains("400");
            auto d = fastSet.contains("510.2.3");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        bench.run("StringSet_miss", [&] {
            auto a = fastSet.contains("VEX");
            auto b = fastSet.contains("411.1X");
            auto c = fastSet.contains("400X");
            auto d = fastSet.contains("H346.11112");
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        std::string_view fsvA = "VE";
        std::string_view fsvB = "411.1";
        std::string_view fsvC = "400";
        std::string_view fsvD = "H346.11112";

        bench.run("StringSet_stringview", [&] {
            auto a = fastSet.contains(fsvA);
            auto b = fastSet.contains(fsvB);
            auto c = fastSet.contains(fsvC);
            auto d = fastSet.contains(fsvD);
            ankerl::nanobench::doNotOptimizeAway(a);
            ankerl::nanobench::doNotOptimizeAway(b);
            ankerl::nanobench::doNotOptimizeAway(c);
            ankerl::nanobench::doNotOptimizeAway(d);
        });

        static constexpr std::string_view SHORT_POSITION = "center-fore-2";
        static constexpr std::string_view LONG_POSITION = "aft-center-fore-lower-port-starboard-upper-2";

        bench.run("StackVector_split_short", [&] {
            StackVector<std::string_view, 8> parts;
            for (std::string_view remaining = SHORT_POSITION; !remaining.empty();)
            {
                auto sep = remaining.find('-');
                parts.push_back(remaining.substr(0, sep));
                remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);
            }
            ankerl::nanobench::doNotOptimizeAway(parts.size());
        });

        bench.run("StdVector_split_short", [&] {
            std::vector<std::string_view> parts;
            for (std::string_view remaining = SHORT_POSITION; !remaining.empty();)
            {
                auto sep = remaining.find('-');
                parts.push_back(remaining.substr(0, sep));
                remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);
            }
            ankerl::nanobench::doNotOptimizeAway(parts.size());
        });

        bench.run("StackVector_split_long", [&] {
            StackVector<std::string_view, 8> parts;
            for (std::string_view remaining = LONG_POSITION; !remaining.empty();)
            {
                auto sep = remaining.find('-');
                parts.push_back(remaining.substr(0, sep));
                remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);
            }
            ankerl::nanobench::doNotOptimizeAway(parts.size());
        });

        bench.run("StdVector_split_long", [&] {
            std::vector<std::string_view> parts;
            for (std::string_view remaining = LONG_POSITION; !remaining.empty();)
            {
                auto sep = remaining.find('-');
                parts.push_back(remaining.substr(0, sep));
                remaining = (sep == std::string_view::npos) ? std::string_view{} : remaining.substr(sep + 1);
            }
            ankerl::nanobench::doNotOptimizeAway(parts.size());
        });

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
