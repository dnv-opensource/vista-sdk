/**
 * @file stringbuilder.cpp
 * @brief Nanobench performance benchmark for StringBuilder vs std::string
 */

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <dnv/vista/sdk/core/VIS.h>
#include <dnv/vista/sdk/utils/StringBuilder.h>

#include <charconv>
#include <string>
#include <string_view>

namespace dnv::vista::sdk::benchmarks
{
    int run()
    {
        ankerl::nanobench::Bench bench;
        bench.title("StringBuilder").relative(true).performanceCounters(true).warmup(10000).minEpochIterations(300000);

        constexpr std::string_view kFragment1 = "{\"shipId\":\"IMO";
        constexpr std::string_view kFragment2 = "\",\"value\":";
        constexpr std::string_view kFragment3 = ",\"quality\":\"Good\",\"timestamp\":\"2026-01-01T00:00:00Z\"}";

        bench.run("StringBuilder_jsonish", [&] {
            StringBuilder sb;
            sb.append(kFragment1);
            sb.append(std::uint64_t{ 1234567 });
            sb.append(kFragment2);
            sb.append(45.2);
            sb.append(kFragment3);
            ankerl::nanobench::doNotOptimizeAway(sb.size());
        });

        bench.run("StdString_jsonish", [&] {
            std::string s;
            s.append(kFragment1);
            char buf[32];
            auto [ptr1, ec1] = std::to_chars(buf, buf + sizeof(buf), std::uint64_t{ 1234567 });
            s.append(buf, ptr1);
            s.append(kFragment2);
            auto [ptr2, ec2] = std::to_chars(buf, buf + sizeof(buf), 45.2);
            s.append(buf, ptr2);
            s.append(kFragment3);
            ankerl::nanobench::doNotOptimizeAway(s.size());
        });

        bench.run("StringBuilder_largeAccumulation", [&] {
            StringBuilder sb;
            for (int i = 0; i < 50; ++i)
            {
                sb.append(std::string_view{ "\"field" });
                sb.append(std::int64_t{ i });
                sb.append(std::string_view{ "\":" });
                sb.append(static_cast<double>(i) * 1.5);
                sb.append(std::string_view{ "," });
            }
            ankerl::nanobench::doNotOptimizeAway(sb.size());
        });

        bench.run("StdString_largeAccumulation", [&] {
            std::string s;
            char buf[32];
            for (int i = 0; i < 50; ++i)
            {
                s.append("\"field");
                auto [ptr1, ec1] = std::to_chars(buf, buf + sizeof(buf), std::int64_t{ i });
                s.append(buf, ptr1);
                s.append("\":");
                auto [ptr2, ec2] = std::to_chars(buf, buf + sizeof(buf), static_cast<double>(i) * 1.5);
                s.append(buf, ptr2);
                s.append(",");
            }
            ankerl::nanobench::doNotOptimizeAway(s.size());
        });

        {
            StringBuilder sb;
            bench.run("StringBuilder_indent", [&] {
                sb.clear();
                sb.append(std::string_view{ "{" });
                sb.appendRepeated(' ', 4);
                sb.append(std::string_view{ "\"a\":1" });
                ankerl::nanobench::doNotOptimizeAway(sb.size());
            });
        }

        {
            std::string s;
            bench.run("StdString_indent", [&] {
                s.clear();
                s.append("{");
                s.append(4, ' ');
                s.append("\"a\":1");
                ankerl::nanobench::doNotOptimizeAway(s.size());
            });
        }

        return 0;
    }
} // namespace dnv::vista::sdk::benchmarks

int main()
{
    return dnv::vista::sdk::benchmarks::run();
}
